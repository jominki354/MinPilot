#!/usr/bin/env python3
"""
CarrotMan 연동 데몬 (carrotd)
- UDP 7706 포트로 TMap/카카오맵 SDI 데이터 수신
- 감속 속도 계산 후 Params에 저장
- controlsd에서 읽어서 v_cruise에 반영
"""

import socket
import json
import math
import time
from common.params import Params

PORT = 7706
DECEL_RATE = 1.5  # m/s²
SAFE_TIME = 3.0  # 초 (카메라 도달 전 여유)
BUMP_SAFE_TIME = 1.0  # 초 (방지턱용)
BUMP_SPEED = 35  # km/h

# SDI 타입 분류
FIXED_CAMERA = [0, 1, 8]  # 고정식 카메라
MOBILE_CAMERA = [7]  # 이동식 카메라
SECTION_CONTROL = [2, 3, 4]  # 구간단속
SPEED_BUMP = [22]  # 과속방지턱


def calc_speed(dist, target_kph, safe_sec, decel):
    """
    등가속도 공식으로 현재 허용 속도 계산
    v² = v₀² + 2ad

    Args:
        dist: 카메라까지 거리 (m)
        target_kph: 목표 속도 (km/h)
        safe_sec: 카메라 도달 전 여유 시간 (초)
        decel: 감속률 (m/s²)

    Returns:
        현재 허용 속도 (km/h)
    """
    if dist <= 0 or target_kph <= 0:
        return 0
    target = target_kph / 3.6  # m/s
    safe_dist = target * safe_sec
    decel_dist = dist - safe_dist
    if decel_dist <= 0:
        return target_kph
    temp = target**2 + 2 * decel * decel_dist
    return max(target_kph, min(250, math.sqrt(max(0, temp)) * 3.6))


def get_ip_address():
    """로컬 IP 주소 가져오기"""
    try:
        s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        s.connect(("8.8.8.8", 80))
        ip = s.getsockname()[0]
        s.close()
        return ip
    except:
        return "0.0.0.0"


def get_broadcast_address():
    """브로드캐스트 주소 계산"""
    ip = get_ip_address()
    parts = ip.split(".")
    if len(parts) == 4:
        parts[3] = "255"
        return ".".join(parts)
    return "255.255.255.255"


def main():
    params = Params()
    params_mem = Params("/dev/shm/params")

    # UDP 수신 소켓 (CarrotMan → C2)
    recv_sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    recv_sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    recv_sock.settimeout(1)  # 1초로 줄여서 브로드캐스트 주기 확보
    recv_sock.bind(("0.0.0.0", PORT))

    # UDP 송신 소켓 (C2 → CarrotMan, 브로드캐스트)
    send_sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    send_sock.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)

    BROADCAST_PORT = 7705  # CarrotMan이 리스닝하는 포트

    print(f"[carrotd] Started - recv:{PORT}, broadcast:{BROADCAST_PORT}")
    last_recv = 0
    last_broadcast = 0

    while True:
        try:
            # 설정 읽기
            enabled = params.get_bool("CarrotSpeedControl")

            # 브로드캐스트 (1초마다)
            now = time.monotonic()
            if now - last_broadcast > 1.0:
                last_broadcast = now
                ip = get_ip_address()
                broadcast_ip = get_broadcast_address()

                # CarrotMan 앱에 보내는 메시지
                msg = {
                    "Carrot2": "SunnyPilot-C2",
                    "ip": ip,
                    "port": PORT,
                    "IsOnroad": params.get_bool("IsOnroad")
                    if params.get("IsOnroad")
                    else False,
                    "active": enabled,
                }
                try:
                    send_sock.sendto(
                        json.dumps(msg).encode(), (broadcast_ip, BROADCAST_PORT)
                    )
                except:
                    pass

            if not enabled:
                params_mem.put("CarrotSpdLimit", "0")
                params_mem.put("CarrotActive", "0")
                time.sleep(0.5)
                continue

            use_mobile = params.get_bool("CarrotMobileCamera")
            use_bump = params.get_bool("CarrotSpeedBump")
            use_section = params.get_bool("CarrotSectionControl")

            try:
                data, addr = recv_sock.recvfrom(4096)
                last_recv = time.monotonic()

                d = json.loads(data.decode("utf-8"))
                if "nRoadLimitSpeed" not in d:
                    continue

                # 데이터 파싱
                road_limit = int(d.get("nRoadLimitSpeed", 30))
                if road_limit > 200:
                    road_limit = int((road_limit - 20) / 10)
                road_limit = max(30, road_limit)

                sdi_type = int(d.get("nSdiType", -1))
                sdi_limit = int(d.get("nSdiSpeedLimit", 0))
                sdi_dist = int(d.get("nSdiDist", 0))
                sdi_block_type = int(d.get("nSdiBlockType", -1))
                sdi_plus_type = int(d.get("nSdiPlusType", -1))
                sdi_plus_dist = int(d.get("nSdiPlusDist", 0))

                target_speed = 0

                # 1. 고정식 카메라 (항상 활성)
                if sdi_type in FIXED_CAMERA and sdi_limit > 0 and sdi_dist > 0:
                    target_speed = calc_speed(
                        sdi_dist, sdi_limit, SAFE_TIME, DECEL_RATE
                    )

                # 2. 이동식 카메라 (옵션)
                elif (
                    sdi_type in MOBILE_CAMERA
                    and sdi_limit > 0
                    and sdi_dist > 0
                    and use_mobile
                ):
                    target_speed = calc_speed(
                        sdi_dist, sdi_limit, SAFE_TIME, DECEL_RATE
                    )

                # 3. 구간단속 (옵션)
                elif sdi_type in SECTION_CONTROL and sdi_limit > 0 and use_section:
                    if sdi_block_type in [2, 3]:  # 구간단속 중
                        target_speed = sdi_limit  # 구간 내 속도 유지
                    elif sdi_dist > 0:
                        target_speed = calc_speed(
                            sdi_dist, sdi_limit, SAFE_TIME, DECEL_RATE
                        )

                # 4. 과속방지턱 (옵션)
                elif (
                    sdi_type in SPEED_BUMP or sdi_plus_type in SPEED_BUMP
                ) and use_bump:
                    bump_dist = sdi_plus_dist if sdi_plus_type == 22 else sdi_dist
                    if bump_dist > 0:
                        target_speed = calc_speed(
                            bump_dist, BUMP_SPEED, BUMP_SAFE_TIME, DECEL_RATE
                        )

                # Params에 저장 (controlsd에서 읽음)
                params_mem.put("CarrotSpdLimit", str(int(target_speed)))
                params_mem.put("CarrotRoadLimit", str(road_limit))
                params_mem.put("CarrotActive", "1")

            except socket.timeout:
                if time.monotonic() - last_recv > 10:
                    params_mem.put("CarrotSpdLimit", "0")
                    params_mem.put("CarrotActive", "0")
            except json.JSONDecodeError:
                pass

        except Exception as e:
            print(f"[carrotd] Error: {e}")
            time.sleep(0.5)


if __name__ == "__main__":
    main()
