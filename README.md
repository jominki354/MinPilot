# MinPilot

SunnyPilot 기반의 Custom Openpilot Fork (C2 Platform)

## 주요 기능

### 1. 로컬라이제이션
- 전체 UI/UX 한글화 적용
- 국내 도로 환경에 최적화된 표시 형식 (날짜, 단위 등)

### 2. 안전 운전 보조 (CarrotMan 연동)
- 고정식 과속 단속 카메라 경고
- 과속 방지턱 알림 및 감속 제어
- 구간 단속 구간 평균 속도 모니터링
- 모바일 카메라 출몰 지역 경고

### 3. 주행 제어 및 튜닝
- 커스텀 조향 토크 제어 (Lateral Torque Custom)
- 가변 조향비 설정 (Steer Ratio Rate)
- 차로 편향 주행 설정 (Path Offset)
- 저속 주행 및 턴 속도 최적화 (MADS, Turn Control)

### 4. 사용자 편의 기능
- 메인 화면 사이드바 IP 주소 표시 (실시간 네트워크 상태)
- 설정 초기화 기능 (커스텀 설정 원클릭 복원)
- 노면 가장자리 감지 시각화 (Road Edge Detection)
- 상세 주행 로그 분석 도구 (Rlogs Analysis)

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


