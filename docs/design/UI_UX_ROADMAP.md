# MinPilot UI/UX 개편 로드맵 (UI/UX Revamp Roadmap)

## 🎨 1. 디자인 철학 (Design Philosophy)
- **Modern & Minimal**: 불필요한 장식 요소(원형 아이콘 등)를 제거하고 정보 전달에 집중
- **Dark Theme**: 야간 운전 시 눈의 피로를 최소화하는 Deep Black 배경
- **Blue Accent**: `#0088FF`를 메인 컬러로 사용하여 신뢰감과 하이테크 느낌 전달
- **Gradient**: 부드러운 그라데이션을 활용하여 고급스러운 마감 처리

---

## 📐 2. 레이아웃 명세 (Layout Specification)

### 2.1 색상 팔레트 (Color Palette)
| 역할 | 색상 코드 | 용도 |
|:---:|:---:|:---|
| **Background** | `#000000` | 전체 배경 |
| **Secondary** | `#808080` | 비활성 요소, 보조 텍스트 |
| **Primary** | `#0088FF` | 활성 상태, 경로, 강조 포인트 |
| **Gradient** | `Transparent` ↔ `#0088FF` | 경로, 도로명 바, 구분선 |

### 2.2 구역별 구성 (Zone Layout)

#### ↖️ 좌측 상단 (Top Left)
- **정보**: 디지털 시계 | 기기 온도
- **경고**: 과속카메라 정보 (CarrotPilot 연동)
  - 아이콘 + 남은 거리 + 제한 속도 표기
- **스타일**: 심플한 텍스트 배치

#### ⬆️ 중앙 상단 (Top Center)
- **속도계 통합**: 현재 속도와 크루즈 설정 속도를 나란히 배치
  - `[현재 속도] │ [크루즈 속도]`
  - 크고 명확한 폰트 사용
  - 중앙 구분선 또는 미묘한 배경 차이로 구분

#### ↙️ 좌측 하단 (Bottom Left)
- **Lane Mode 토글**: 차선 인식 모드 표시
- **상태**: `Auto` / `Lane` / `Laneless`
- **그래픽**: 
  - **Lane Mode**: 차선 외곽선(Outline) 강조
  - **Laneless**: 심플한 라인 처리

#### ↘️ 우측 하단 (Bottom Right)
- **주행 상태 인디케이터**:
  - `[ACC] ● ON/OFF` (종방향 제어)
  - `[LKAS] ● ON/OFF` (횡방향 제어)
- **스타일**: 텍스트 + 상태 점(Dot) - ON(Green/Blue), OFF(Grey)

#### ⬇️ 중앙 하단 (Bottom Center)
- **도로명 표시**:
  - 크기 확대 (가독성 강화)
  - 배경에 그라데이션 페이드 효과 적용 (고급화)

#### ❌ 제거 항목 (Removed)
- 우상단 원형 아이콘 파티 (`Engage Icon`, `MADS Icon`) 제거
- 기존 좌상단 최고속도 박스 제거 (중앙 통합)

---

## � 3. 디자인 가이드라인 (Design Guidelines)

### 3.1 그리드 및 여백 (Grid & Spacing)
- **Safe Area**: 
  - 화면 가장자리에서 `30px ~ 50px` 여백 준수 (베젤 고려)
  - 상단바/하단바 영역은 터치 조작 용이성을 위해 `120px` 이상 높이 확보

- **UI Element Spacing**:
  - 요소 간 간격(Gap): 최소 `20px`
  - 박스 내부 패딩(Padding): 최소 `15px`

### 3.2 타이포그래피 (Typography)
- **Font Family**: `Open Sans` (기본), `Inter` (숫자 강조용)
- **Size System**:
  - **Hero Text** (속도): `90px ~ 120px` (Bold)
  - **Title** (도로명): `40px ~ 48px` (Bold)
  - **Body** (정보): `28px ~ 32px` (Regular)
  - **Label** (단위/설명): `20px ~ 24px` (Light)

### 3.3 컬러 시스템 상세 (Color System)
| Name | Hex Code | Opacity | Usage |
|:---|:---:|:---:|:---|
| **Deep Black** | `#000000` | 100% | 메인 배경 |
| **Overlay Black**| `#000000` | 40-60% | 패널 배경, 팝업 배경 |
| **MinPilot Blue**| `#0088FF` | 100% | 활성 상태, 강조, 경로 |
| **Alert Red** | `#FF4D4D` | 100% | 경고, 오작동 |
| **Success Green**| `#00D166` | 100% | 정상 작동, 인디케이터 ON |
| **Inactive Grey**| `#808080` | 100% | 비활성 아이콘, 보조 텍스트 |

### 3.4 좌표 기준 (Resolution: 1920x1080)
- **Center Speed Zone**:
  - Y: `150px` (상단 여백)
  - Width: `800px` (중앙 정렬)
- **Bottom Bar (Road Name)**:
  - Height: `160px`
  - Y: `920px` (Bottom aligned)
- **Side Info Panels**:
  - Width: `400px`
  - Margin: `40px` from edges

---

## �🛣️ 4. 구현 로드맵 (Implementation Roadmap)

### Phase 1: 시뮬레이터 프로토타이핑 (Simulator Prototyping)
- [ ] `hud_display.cpp` 리팩토링 및 레이아웃 재구성
- [ ] 불필요한 요소(아이콘 등) 제거
- [ ] 그라데이션 및 색상 팔레트 적용
- [ ] 신규 요소(속도계 쌍, 상태 인디케이터) 배치 및 렌더링 구현
- [ ] **목표**: 정지 이미지 및 더미 데이터로 디자인룩 확정

### Phase 2: 실제 데이터 연동 (Data Integration)
- [ ] `ui_state` 구조체 확장 (ACC/LKAS 상태, 카메라 정보 등)
- [ ] 시뮬레이터 목업 데이터(control_panel)와 새 UI 요소 연동 테스트
- [ ] Lane/Laneless 그래픽 렌더링 로직 분기 구현 (Outline 효과)

### Phase 3: MinPilot 이식 (Porting to MinPilot)
- [ ] `selfdrive/ui/qt/onroad.cc` 및 관련 파일 수정
- [ ] 실제 차량(Camry/Toyota)의 ACC/LKAS 신호 매핑 검토 및 적용
- [ ] CarrotPilot 코드 참조하여 과속카메라 로직 통합
- [ ] 실차 테스트 및 폰트/크기 미세 조정

---

## ⚠️ 5. 기술적 검토사항 (Technical Review)
- **ACC/LKAS Signal**: 도요타/렉서스 차량의 경우 `PCM Cruise`와 `Openpilot Steer` 상태를 정확히 구분하여 매핑해야 함.
- **CarrotPilot Integration**: 과속카메라 정보는 **CarrotPilot 코드**를 직접 이식하여 구현 (별도의 MapD/Network 분석 불필요).
- **Performance**: 그라데이션 렌더링 시 프레임 드랍 주의 (Qt QLinearGradient 최적화).
