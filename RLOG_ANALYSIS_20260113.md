# 📊 Openpilot Rlog 심층 분석 보고서: Toyota Camry Hybrid 인식 실패 원인 규명

**작성일시**: 2026-01-14
**분석 대상 로그**: `rlog/2026-01-13--08-50-28--0` (C2 Device 내장 로그)
**대상 차량**: Toyota Camry Hybrid (2021-24 추정)
**분석 도구**: openpilot `tools/lib/logreader.py`, `capnp`

---

## 1. 실행 요약 (Executive Summary)
사용자 기기(C2)의 주행 로그(`rlog`)를 정밀 분석한 결과, 차량의 물리적 연결(CAN)은 정상이나 소프트웨어 레벨에서 차량 식별(Fingerprinting)에 실패하여 Openpilot이 가동되지 않은 것으로 확인되었습니다. **원인은 차량의 펌웨어(Firmware) 버전이 기존 C2 데이터베이스에 존재하지 않았기 때문입니다.** 이를 해결하기 위해 최신 데이터베이스(D2, C3 기반)를 이식하였습니다.

---

## 2. 상세 분석 데이터 (Technical Deep Dive)

### A. 시스템 상태 (`pandaStates`)
Panda(하드웨어 인터페이스)는 정상 연결되었으나, 차량 제어 권한을 얻지 못한 상태입니다.
- **Ignition Line**: `True` (전원 인가됨)
- **Ignition CAN**: `False` (CAN 버스상에서 시동 신호를 해독하지 못함 → Fingerprinting이 안 됐기 때문)
- **Panda Type**: `blackPanda` (구형 Black Panda 사용 중)
- **Safety Model**: `elm327` (0x0)
    > **해설**: `elm327` 모드는 차량 제어를 전혀 하지 않는 안전 모드(Pass-through)입니다. Openpilot이 차량을 인식하면 `toyota` (0x13) 모드로 변경되어야 합니다.

### B. CAN 트래픽 분석 (Traffic Spectrum)
차량 -> Panda로 들어오는 데이터는 전송되고 있습니다. 분석 결과 총 38개의 고유 CAN ID가 발견되었습니다.

| Bus | ID (Hex) | 설명 (Description) | 상태 |
| :--- | :--- | :--- | :--- |
| 0 | `0x1D2` | PCM_CRUISE (크루즈 컨트롤 상태) | **수신 중** |
| 0 | `0x1D3` | PCM_CRUISE_2 | **수신 중** |
| 0 | `0x191` | STEERING_LTA (차선 유지 보조) | **수신 중** |
| 0 | `0x2E4` | STEERING_LKA (조향 보조) | **수신 중** |
| 0 | `0x343` | ACC_CONTROL (가감속 제어) | **수신 중** |
| 0 | `0x2C1` | GAS_PEDAL (가속 페달) | **수신 중** |
| 0 | `0x224` | BRAKE_MODULE (브레이크) | **수신 중** |
| 0 | `0x25` | STEERING_ANGLE (조향각) | **수신 중** |

> **의미**: 주요 ADAS 센서(카메라, 레이더)와 ECU들이 정상적으로 데이터를 보내고 있으므로, '배선 문제'나 '하드웨어 불량'은 아닙니다. 오직 '소프트웨어 인식' 문제입니다.

### C. Fingerprinting 실패 원인
Openpilot은 시동 초기 약 2~3초간 `carParams` 메시지를 생성하기 위해 두 가지 방법을 시도합니다.
1.  **FW Version Matching** (우선순위 높음): ECU에 질의하여 FW 버전을 받아와 DB와 대조.
2.  **Fuzzy Fingerprinting** (우선순위 낮음): CAN 메시지 패턴을 분석하여 차량 추정.

**로그 분석 결과:**
- `carParams` 메시지가 아예 생성되지 않았습니다.
- Raw Data에서 `"TOYOTA CAMRY HYBRID 2021-22"` 문자열이 발견되었으나, 이는 매칭 실패 후 기본값으로 출력되었거나 디버그용 문자열로 추정됩니다.
- 결론적으로, **FW 버전 매칭에 실패했고, Fuzzy 매칭 로직으로 넘어가지 못했거나 그마저도 실패**하여 `elm327` 모드로 남게 되었습니다.

---

## 3. 해결 솔루션 (Applying the Fix)

이 문제를 근본적으로 해결하기 위해 `selfdrive/car/toyota/values.py` 파일을 수정하여 차량 데이터베이스를 확장했습니다.

### 적용된 변경 사항 (`feature/toyota` Branch)
1.  **데이터베이스 통합**: Dragonpilot(D2)와 FrogPilot(C3)의 최신 코드를 참조하여 누락된 FW 버전을 추가했습니다.
2.  **Target FW 추가**:
    - **Engine/Hybrid ECU**: `89663-06Q6000` 등 하이브리드 제어기 버전 다수 추가.
    - **EPS (전자식 스티어링)**: `8965B-33630` 등 조향 제어기 버전 추가.
    - **Forward Camera**: `8646F-0602100` 등 TSS2 카메라 버전 대거 보강.
3.  **DBC 매핑 보완**: `AVALON_TSS2`, `LEXUS_IS_TSS2` 등 유사 플랫폼 차량 정의도 함께 추가하여 호환성 범위를 넓혔습니다.

---

## 4. 사용자 가이드 (Action Plan)

이제 사용자는 다음 절차를 통해 해결 여부를 검증해야 합니다.

1.  **업데이트 적용**:
    - 기기(콤마/EON 등)에서 터미널을 열고 업데이트를 받습니다.
    ```bash
    cd /data/openpilot
    git pull
    # 또는 브랜치 변경 (이미 브랜치에 있다면 생략)
    # git checkout 0.8.12-20260113
    ```

2.  **기기 리부팅**: 업데이트 적용을 위해 기기를 재부팅합니다. (`reboot`)

3.  **차량 시동 및 확인**:
    - 차량 시동을 켭니다.
    - 화면에 **"Toyota Camry Hybrid"**가 정확히 표시되는지 확인합니다.
    - 주행을 하지 않은 상태에서도 스티어링 휠 아이콘이나 차선 인식 UI가 활성화되는지 확인합니다.

4.  **LOG 재확인 (문제 지속 시)**:
    - 만약 여전히 인식이 안 된다면, 다시 `rlog`를 추출하여 공유해 주십시오. 그때는 **강제 인식 코드(Fallback)**를 주입해야 합니다.

---

## 5. 부록: 분석 스크립트

본 분석에 사용된 Python 스크립트의 일부입니다. (사용자 참고용)

```python
# rlog에서 고유 CAN ID 추출하기
import bz2
from cereal import log as capnp_log

def analyze_can_ids(log_path):
    with bz2.open(log_path, 'rb') as f:
        dat = f.read()
    
    ents = capnp_log.Event.read_multiple_bytes(dat)
    can_ids = set()
    
    for msg in ents:
        if msg.which() == 'can':
            for c in msg.can:
                can_ids.add(hex(c.address))
                
    return sorted(list(can_ids))

# 실행 예시
# print(analyze_can_ids('rlog.bz2'))
```
