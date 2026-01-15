# MinPilot

SunnyPilot 기반의 Custom Openpilot Fork (C2 Platform)

## 주요 기능

## 주요 기능 (설정 메뉴 가이드)

### 1. 시스템 설정
- **조용한 주행**: 경고음 최소화로 동승자 배려
- **빠른 부팅**: 시동 직후 즉시 사용할 수 있도록 부팅 속도 최적화
- **저사양 모드**: 로깅/녹화/GPS 등 백그라운드 프로세스 중단 (발열 감소 및 성능 확보)
- **데이터 절약 모드**: 주행 로그 업로드 차단 (지도 기능 사용 시 해제 필요)
- **배터리 방전 방지**: 시동 OFF 시 장치 강제 종료 및 전원 차단

### 2. MADS (상시 조향)
- **독립형 조향 보조**: 크루즈(ACC) 비활성 상태에서도 차선 유지 기능 단독 사용
- **크루즈 연동**: 핸들의 SET/RES 버튼으로 상시 조향 ON/OFF 연동
- **핸들 파지 감지**: 운전자 핸들 미파지 시 경고 알림

### 3. 속도 제어
- **제한 속도 연동**: 지도 데이터 기반 규정 속도 자동 설정
- **커브 감속 (지도/비전)**: 급커브 진입 시 지도/카메라 분석을 통한 사전 감속
- **속도 조절 간격**: 크루즈 속도 변경 단위 사용자 설정

### 4. 차선 변경
- **도로 가장자리 감지 (RED)**: 차선 없음/갓길 등 위험 구역 차선 변경 제한
- **자동 차선 변경 타이머**: 방향지시등 작동 후 차선 변경 시작 대기 시간 조절

### 5. 조향 튜닝
- **동적 차선 모드 (DLP)**: 고속도로/국도 등 주행 환경에 따른 차선 유지 감도 자동 전환
- **조향 토크 설정**: 차종별 핸들 조향 힘(토크) 미세 조정
- **가변 조향비**: 속도별 조향비 변화율 설정을 통한 고속 주행 안정성 확보
- **차선 편향 (Offset)**: 차선 중앙 편향 주행 시 위치 보정

### 6. 화면 설정
- **레이더 정보 표시**: 선행 차량 거리 및 상대 속도 박스 표시
- **디버그 UI**: 개발자용 상세 수치 및 로그 저장 버튼 표시
- **화면 밝기/끄기**: 주행 중 화면 밝기 조절 및 자동 꺼짐 설정

### 7. 안전운전 도우미 (CarrotMan)
- **과속 단속 경고**: 고정식/이동식/구간 단속 카메라 사전 경고 및 감속
- **방지턱 감속**: 전방 과속 방지턱 인식 시 부드러운 감속
- **스쿨존 알림**: 어린이 보호 구역 진입 시 주의 알림

### 8. 기타 편의 (장치 메뉴)
- **설정 초기화**: 모든 커스텀 설정 기본값 원클릭 복원
- **설정 백업/복원**: 서버를 통한 설정 백업 및 타 기기 동기화
- **IP 주소 표시**: 사이드바에 실시간 네트워크 연결 상태 및 IP 표시


## 버전 정보
- **MinPilot Version**: 0.8.12
- **Base Branch**: SunnyPilot / OpenPilot 0.8.12
- **Platform**: Comma Two (C2) / Black Panda (LeEco Le Pro3)

## 호환 차량
MinPilot은 **Toyota Camry Hybrid**를 포함하여 Openpilot/SunnyPilot이 지원하는 대부분의 차량과 호환됩니다.

### ✅ 주요 테스트 차량
- **Toyota Camry Hybrid** (2021-2024 Model, TSS 2.5+)

<details>
<summary>📋 <b>전체 호환 차량 목록 보기 (클릭)</b></summary>

### Toyota / Lexus (TSS2 / TSS 2.5+)
- **Toyota**: Alphard, Avalon, **Camry**, C-HR, Corolla, Highlander, Prius, RAV4, Sienna, Mirai
- **Lexus**: CT, ES, IS, NX, RC, RX, UX

### Hyundai / Kia / Genesis (HKG)
- **Hyundai**: Elantra (Avante), Ioniq (EV/HEV), Kona (EV/HEV), Santa Fe, Sonata, Palisade, Veloster
- **Kia**: Forte (K3), K5 (Optima), Niro (EV/HEV), Seltos, Sorento, Stinger, Ceed
- **Genesis**: G70, G80, G90

*위 목록에 없는 차량이라도 openpilot이 지원하는 경우 호환될 가능성이 높습니다.*
</details>
<br/>



