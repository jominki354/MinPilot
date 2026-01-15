# MinPilot

SunnyPilot 기반의 Custom Openpilot Fork (C2 Platform)

## 주요 기능

### 1. 자율 주행 및 제어 (Driving Control)
- **상시 조향 보조 (M.A.D.S.)**: 크루즈 컨트롤(SCC)을 켜지 않아도 차선 유지 기능을 독립적으로 사용 가능
- **동적 차선 프로필 (DLP)**: 주행 환경(고속도로/국도)에 따라 차선 유지 모드 자동 최적화 (Auto/Laneless)
- **비전/맵 기반 코너링 제어 (Turn Control)**: 커브길 진입 시 카메라 및 지도 데이터를 분석하여 부드럽게 감속
- **차간 거리 자동 조절 (Longitudinal Control)**: 앞차와의 거리 유지 및 정차 후 재출발(Stop & Go) 완벽 지원
- **차간 거리 상세 설정 (Gap Adjust)**: 스티어링 휠 버튼으로 앞차와의 거리를 상황에 맞춰 실시간 조절

### 2. 안전 운전 도우미 (CarrotMan 연동)
- **어린이 보호 구역 알림**: 스쿨존 진입 시 경고 및 규정 속도 이하로 자동 감속
- **과속 단속 카메라 경고**: 고정식/이동식/구간 단속 카메라를 감지하여 사전 경고 및 감속 유도
- **방지턱 알림**: 전방 과속 방지턱 인식 시 부드러운 감속 및 알림 제공
- **구간 단속 모니터링**: 구간 단속 시작/종료 지점 및 평균 속도를 실시간으로 안내

### 3. 운전자 모니터링 및 편의 (편의성 강화)
- **운전자 주의 감지 (DM)**: 운전자의 시선을 분석하여 주의 태만 및 졸음 운전 방지
- **핸들 파지 감지 (HOD)**: 정전식/토크식 핸들 파지 여부를 정밀하게 모니터링 (EU 규제 대응)
- **주행 화면 녹화**: 주행 중인 화면을 녹화하여 블랙박스처럼 활용 가능
- **정숙 주행 모드 (Quiet Drive)**: 불필요한 음성 안내를 최소화하여 동승자의 휴식을 배려
- **빠른 부팅 (Fast Boot)**: 시동 직후 즉시 시스템을 사용할 수 있도록 부팅 속도 최적화

### 4. 한글화 및 커스터마이징 (User Customization)
- **완벽한 한글 지원**: 날짜, 시간, 경고 메시지, 설정 메뉴 등 모든 UI를 자연스러운 한글로 제공
- **정밀 튜닝 옵션**: 조향감(Torque), 조향비(Steer Ratio), 차선 편향(Path Offset) 등을 운전자 성향에 맞게 세밀하게 설정
- **도로 경계 감지 (Road Edge Detection)**: 차선이 없는 도로에서도 도로 가장자리를 인식하여 이탈 방지
- **네트워크 상태 표시**: 사이드바에 실시간 IP 주소를 표시하여 테더링/와이파이 연결 상태 확인 용이
- **원클릭 초기화**: 설정이 꼬였을 때 기본값으로 즉시 복원하는 '설정 초기화' 기능 탑재 (Rescue Tool 불필요)

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



