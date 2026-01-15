#!/usr/bin/env python3
"""
설정 초기화 스크립트
manager.py의 default_params를 참조하여 모든 커스텀 설정을 기본값으로 재설정합니다.
새 기능 추가 시 manager.py에만 추가하면 자동으로 초기화 대상에 포함됩니다.
"""

import sys
import os

# openpilot 경로 추가
sys.path.insert(0, "/data/openpilot")
os.chdir("/data/openpilot")

from common.params import Params

# manager.py에서 default_params 가져오기
# 순환 import 방지를 위해 직접 정의 (manager.py와 동기화 필요)
# 하지만 manager.py를 직접 참조하면 더 좋음
try:
    from selfdrive.manager.manager import default_params
except ImportError:
    # fallback: 직접 정의
    default_params = [
        ("CompletedTrainingVersion", "0"),
        ("DisableRadar", "0"),
        ("DisableUpdates", "0"),
        ("DynamicLaneProfile", "2"),
        ("EndToEndToggle", "1"),
        ("GpxDeleteAfterUpload", "1"),
        ("GpxDeleteIfUploaded", "1"),
        ("HasAcceptedTerms", "0"),
        ("HandsOnWheelMonitoring", "0"),
        ("MaxTimeOffroad", "9"),
        ("NoOffroadFix", "0"),
        ("OnroadScreenOff", "0"),
        ("OnroadScreenOffBrightness", "50"),
        ("OpenpilotEnabledToggle", "1"),
        ("PrebuiltOn", "0"),
        ("ShowDebugUI", "1"),
        ("SpeedLimitControl", "1"),
        ("SpeedLimitPercOffset", "1"),
        ("TurnSpeedControl", "1"),
        ("ShowRadarInfo", "1"),
        ("TurnVisionControl", "1"),
        ("LateralTorqueCustom", "0"),
        ("LateralTorqueAccelFactor", "2500"),
        ("LateralTorqueFriction", "100"),
        ("CustomSteerRatio", "0"),
        ("CustomSteerRatioEnable", "0"),
        ("SteerRatioRate", "100"),
        ("SteerRatioRateEnable", "0"),
        ("PathOffset", "0"),
        ("PathOffsetEnable", "0"),
        ("RoadEdgeDetection", "0"),
    ]


def reset_to_defaults():
    """모든 커스텀 설정을 기본값으로 재설정"""
    params = Params()

    # default_params의 모든 키를 기본값으로 재설정
    for item in default_params:
        if isinstance(item, tuple) and len(item) >= 2:
            key, value = item[0], item[1]
            try:
                if isinstance(value, bytes):
                    params.put(key, value.decode("utf-8"))
                else:
                    params.put(key, str(value))
                print(f"Reset: {key} = {value}")
            except Exception as e:
                print(f"Error resetting {key}: {e}")

    # 캘리브레이션도 초기화 (선택적)
    # params.remove("CalibrationParams")

    # 재부팅 트리거
    params.put_bool("DoReboot", True)
    print("설정 초기화 완료. 재부팅합니다...")


if __name__ == "__main__":
    reset_to_defaults()
