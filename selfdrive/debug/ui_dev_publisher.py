#!/usr/bin/env python3
"""
UI 개발 모드용 더미 메시지 퍼블리셔
판다/차량 연결 없이 온로드 UI를 테스트할 때 사용합니다.

⚠️ SAFETY: 실제 차량이 연결되면 자동 종료됩니다.
"""

import cereal.messaging as messaging
import time
import random


def is_car_connected():
    """판다/차량 연결 여부 체크"""
    try:
        # pandaStates 메시지 체크
        sm = messaging.SubMaster(["pandaStates"], poll=["pandaStates"])
        sm.update(100)  # 100ms timeout
        if sm.valid["pandaStates"] and len(sm["pandaStates"].pandaStates) > 0:
            panda_type = sm["pandaStates"].pandaStates[0].pandaType
            # UNKNOWN이 아니면 판다 연결됨
            return panda_type != 0  # 0 = UNKNOWN
    except Exception:
        pass
    return False


def main():
    # SAFETY CHECK: 판다가 연결되어 있으면 실행 안함
    if is_car_connected():
        print("⚠️ 판다/차량이 연결되어 있습니다. UI Dev Publisher를 실행할 수 없습니다.")
        print("실제 주행 데이터와 충돌을 방지하기 위해 종료합니다.")
        return

    pm = messaging.PubMaster(
        [
            "deviceState",
            "carState",
            "controlsState",
            "radarState",
            "lateralPlan",
            "longitudinalPlan",
            "liveCalibration",
        ]
    )

    print("UI Dev Publisher 시작...")
    print("⚠️ 판다 연결 시 자동 종료됩니다.")
    print("Ctrl+C로 종료")

    frame = 0
    while True:
        # 주기적으로 판다 연결 체크 (10초마다)
        if frame % 100 == 0 and is_car_connected():
            print("⚠️ 판다/차량이 연결되었습니다. 종료합니다.")
            break

        # deviceState
        msg = messaging.new_message("deviceState")
        msg.deviceState.started = True
        msg.deviceState.ambientTempC = 45 + random.randint(-5, 10)  # 40~55도
        msg.deviceState.networkType = 4  # WiFi
        msg.deviceState.networkStrength = 3
        msg.deviceState.freeSpacePercent = 50
        pm.send("deviceState", msg)

        # carState
        msg = messaging.new_message("carState")
        msg.carState.vEgo = 27.8  # 100km/h
        msg.carState.cruiseState.speed = 30.6  # 110km/h
        msg.carState.cruiseState.enabled = True
        msg.carState.brakeLights = False
        msg.carState.steeringAngleDeg = random.uniform(-5, 5)
        pm.send("carState", msg)

        # controlsState
        msg = messaging.new_message("controlsState")
        cs = msg.controlsState
        cs.engageable = True
        cs.enabled = True
        cs.active = True
        cs.madsEnabled = True
        cs.suspended = False
        cs.cruiseEnabled = True
        cs.lateralControlState.pidState.steeringAngleDesiredDeg = random.uniform(-3, 3)
        pm.send("controlsState", msg)

        # radarState (선행차 시뮬레이션)
        msg = messaging.new_message("radarState")
        lead = msg.radarState.leadOne
        lead.status = True
        lead.dRel = 25.0 + random.uniform(-5, 5)  # 20~30m
        lead.vRel = random.uniform(-2, 2)
        pm.send("radarState", msg)

        # lateralPlan
        msg = messaging.new_message("lateralPlan")
        msg.lateralPlan.dynamicLaneProfile = 0
        pm.send("lateralPlan", msg)

        # longitudinalPlan (속도 제한 시뮬레이션)
        msg = messaging.new_message("longitudinalPlan")
        lp = msg.longitudinalPlan
        lp.speedLimit = 27.8  # 100km/h
        lp.speedLimitOffset = 2.78  # +10km/h
        pm.send("longitudinalPlan", msg)

        # liveCalibration (캘리브레이션 완료 상태)
        msg = messaging.new_message("liveCalibration")
        msg.liveCalibration.calStatus = 1  # Calibrated
        msg.liveCalibration.rpyCalib = [0.0, 0.0, 0.0]
        pm.send("liveCalibration", msg)

        frame += 1
        if frame % 100 == 0:
            print(f"Frame {frame}: 메시지 송신 중...")

        time.sleep(0.1)  # 10Hz


if __name__ == "__main__":
    try:
        main()
    except KeyboardInterrupt:
        print("\nUI Dev Publisher 종료")
