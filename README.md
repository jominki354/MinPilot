# MinPilot

SunnyPilot 기반의 Custom Openpilot Fork (C2 Platform)

## 주요 기능

### 1. 자율 주행 및 제어 (Driving Control)
- **M.A.D.S. (Modified Assistive Driving Safety)**: openpilot이 활성화되지 않은 상태에서도 차로 유지 보조 기능을 독립적으로 사용 가능
- **Dynamic Lane Profile (DLP)**: 주행 환경에 따라 자동으로 최적화된 차선 유지 모드 전환 (Auto/Laneless)
- **Vision-based Turn Control**: 카메라 비전 기반 코너 감속 제어
- **Map-Data-based Turn Control**: 지도 데이터 기반 곡선 구간 자동 감속
- **Longitudinal Control**: 앞차와의 거리 유지 및 속도 제어 (Stop & Go 지원)
- **Gap Adjust**: 앞차와의 차간 거리를 스티어링 휠 버튼으로 실시간 조절

### 2. 안전 운전 보조 (CarrotMan 연동)
- **School Zone Alert**: 어린이 보호 구역 진입 시 알림 및 감속
- **Speed Camera Alert**: 고정식/이동식 과속 단속 카메라 감지 및 경고
- **Speed Bump Alert**: 과속 방지턱 감지 및 자동 감속
- **Section Control**: 구간 단속 구간 평균 속도 모니터링 및 안내

### 3. 운전자 모니터링 및 편의 (Driver Monitoring & UX)
- **Driver Monitoring (DM)**: 운전자 주의 태만 및 졸음 감지
- **Hands on Wheel Monitoring**: 스티어링 휠 파지 여부 모니터링 (EU 규격 준수)
- **Screen Recorder**: 주행 화면 녹화 및 저장 기능
- **Quiet Drive**: 음성 안내 최소화 모드 (동승자 배려)
- **Fast Boot**: 시동 시 빠른 부팅 (Prebuilt 기능)

### 4. 로컬라이제이션 및 커스터마이징 (Localization)
- **Korean UI**: 전체 인터페이스 한글화 (날짜, 경고 메시지, 설정 메뉴)
- **Custom Tuning**: 조향 토크(Torque), 조향비(Steer Ratio), 오프셋(Path Offset) 사용자 설정
- **Road Edge Detection**: 노면 가장자리 인식 시각화 및 차선 이탈 방지 보조
- **Network Stats**: 사이드바 실시간 IP 주소 및 네트워크 상태 표시
- **Settings Reset**: 원클릭 설정 초기화 및 복구 기능


## 버전 정보
- **MinPilot Version**: 0.8.12
- **Base Branch**: SunnyPilot / OpenPilot 0.8.12
- **Platform**: Comma Two (C2) / Black Panda

## 호환 차량
**Toyota**
- Camry Hybrid (2021-2024 Model Year)
  - TSS 2.5+ 시스템 완벽 호환
  - 롱 컨트롤 (Longitudinal Control) 지원 (Stock ACC 대체 가능)
  - MADS (상시 조향) 지원


