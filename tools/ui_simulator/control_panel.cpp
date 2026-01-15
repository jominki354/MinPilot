#include "control_panel.h"

// Forward declare HudDisplay methods (defined in main.cpp)
class HudDisplay;

ControlPanel::ControlPanel(HudDisplay* hud, MockLaneSimulator* sim, QWidget* parent)
    : QWidget(parent), hudDisplay(hud), simulator(sim) {
    setupUI();
}

void ControlPanel::setupUI() {
    auto layout = new QVBoxLayout(this);
    layout->setSpacing(10);
    layout->setContentsMargins(15, 15, 15, 15);
    
    // Title
    auto titleLabel = new QLabel("🎮 MinPilot UI Simulator");
    titleLabel->setStyleSheet("font-size: 16px; font-weight: bold; padding: 5px;");
    layout->addWidget(titleLabel);
    
    // === Lane Mode ===
    auto laneModeGroup = new QGroupBox("🛣️ Lane Mode");
    auto laneModeLayout = new QVBoxLayout(laneModeGroup);
    laneModeCombo = new QComboBox();
    laneModeCombo->addItem("Auto");
    laneModeCombo->addItem("Lane (차선 표시)");
    laneModeCombo->addItem("Laneless (경로만)");
    laneModeCombo->setCurrentIndex(1);
    connect(laneModeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ControlPanel::onLaneModeChanged);
    laneModeLayout->addWidget(laneModeCombo);
    layout->addWidget(laneModeGroup);
    
    // === Road Type ===
    auto roadTypeGroup = new QGroupBox("🛤️ 도로 타입");
    auto roadTypeLayout = new QVBoxLayout(roadTypeGroup);
    roadTypeCombo = new QComboBox();
    roadTypeCombo->addItem("직선");
    roadTypeCombo->addItem("좌회전");
    roadTypeCombo->addItem("우회전");
    roadTypeCombo->addItem("S-커브");
    roadTypeCombo->addItem("왼쪽 차선변경");
    roadTypeCombo->addItem("오른쪽 차선변경");
    connect(roadTypeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ControlPanel::onRoadTypeChanged);
    roadTypeLayout->addWidget(roadTypeCombo);
    
    // Curve radius
    auto curveLayout = new QHBoxLayout();
    curveLayout->addWidget(new QLabel("곡률:"));
    curveRadiusSlider = new QSlider(Qt::Horizontal);
    curveRadiusSlider->setRange(50, 1000);
    curveRadiusSlider->setValue(300);
    curveRadiusLabel = new QLabel("300m");
    connect(curveRadiusSlider, &QSlider::valueChanged,
            this, &ControlPanel::onCurveRadiusChanged);
    curveLayout->addWidget(curveRadiusSlider);
    curveLayout->addWidget(curveRadiusLabel);
    roadTypeLayout->addLayout(curveLayout);
    
    layout->addWidget(roadTypeGroup);
    
    // === Lead Vehicle ===
    auto leadGroup = new QGroupBox("🚗 선두 차량");
    auto leadLayout = new QVBoxLayout(leadGroup);
    
    auto leadDistLayout = new QHBoxLayout();
    leadDistLayout->addWidget(new QLabel("거리:"));
    leadDistanceSlider = new QSlider(Qt::Horizontal);
    leadDistanceSlider->setRange(10, 150);
    leadDistanceSlider->setValue(30);
    leadDistanceLabel = new QLabel("30m");
    connect(leadDistanceSlider, &QSlider::valueChanged,
            this, &ControlPanel::onLeadDistanceChanged);
    leadDistLayout->addWidget(leadDistanceSlider);
    leadDistLayout->addWidget(leadDistanceLabel);
    leadLayout->addLayout(leadDistLayout);
    
    layout->addWidget(leadGroup);
    
    // === Speed ===
    auto speedGroup = new QGroupBox("⚡ 속도");
    auto speedLayout = new QVBoxLayout(speedGroup);
    
    auto currentSpeedLayout = new QHBoxLayout();
    currentSpeedLayout->addWidget(new QLabel("현재:"));
    speedSlider = new QSlider(Qt::Horizontal);
    speedSlider->setRange(0, 200);
    speedSlider->setValue(60);
    speedLabel = new QLabel("60 km/h");
    connect(speedSlider, &QSlider::valueChanged,
            this, &ControlPanel::onSpeedChanged);
    currentSpeedLayout->addWidget(speedSlider);
    currentSpeedLayout->addWidget(speedLabel);
    speedLayout->addLayout(currentSpeedLayout);
    
    auto maxSpeedLayout = new QHBoxLayout();
    maxSpeedLayout->addWidget(new QLabel("최고:"));
    maxSpeedSlider = new QSlider(Qt::Horizontal);
    maxSpeedSlider->setRange(30, 150);
    maxSpeedSlider->setValue(80);
    maxSpeedLabel = new QLabel("80 km/h");
    connect(maxSpeedSlider, &QSlider::valueChanged,
            this, &ControlPanel::onMaxSpeedChanged);
    maxSpeedLayout->addWidget(maxSpeedSlider);
    maxSpeedLayout->addWidget(maxSpeedLabel);
    speedLayout->addLayout(maxSpeedLayout);
    
    layout->addWidget(speedGroup);
    
    // === Status ===
    auto statusGroup = new QGroupBox("🔘 상태");
    auto statusLayout = new QVBoxLayout(statusGroup);
    
    engagedCheck = new QCheckBox("Engaged (활성화)");
    engagedCheck->setChecked(true);
    connect(engagedCheck, &QCheckBox::toggled,
            this, &ControlPanel::onEngagedChanged);
    statusLayout->addWidget(engagedCheck);
    
    showLanesCheck = new QCheckBox("차선 표시");
    showLanesCheck->setChecked(true);
    connect(showLanesCheck, &QCheckBox::toggled,
            this, &ControlPanel::onShowLanesChanged);
    statusLayout->addWidget(showLanesCheck);
    
    layout->addWidget(statusGroup);
    
    // === Road Name ===
    auto roadNameGroup = new QGroupBox("📍 도로명");
    auto roadNameLayout = new QVBoxLayout(roadNameGroup);
    roadNameEdit = new QLineEdit("강변북로");
    connect(roadNameEdit, &QLineEdit::textChanged,
            this, &ControlPanel::onRoadNameChanged);
    roadNameLayout->addWidget(roadNameEdit);
    layout->addWidget(roadNameGroup);
    
    layout->addStretch();
    
    // Help text
    auto helpLabel = new QLabel("💡 실시간으로 조절하며 UI 변화 확인");
    helpLabel->setStyleSheet("color: gray; font-size: 11px;");
    layout->addWidget(helpLabel);
}

void ControlPanel::onSpeedChanged(int value) {
    speedLabel->setText(QString::number(value) + " km/h");
    emit speedChanged((float)value);
}

void ControlPanel::onMaxSpeedChanged(int value) {
    maxSpeedLabel->setText(QString::number(value) + " km/h");
    emit maxSpeedChanged((float)value);
}

void ControlPanel::onLaneModeChanged(int index) {
    emit laneModeChanged(index);
}

void ControlPanel::onRoadTypeChanged(int index) {
    simulator->setRoadType(static_cast<MockLaneSimulator::RoadType>(index));
}

void ControlPanel::onCurveRadiusChanged(int value) {
    curveRadiusLabel->setText(QString::number(value) + "m");
    simulator->setCurveRadius(value);
}

void ControlPanel::onLeadDistanceChanged(int value) {
    leadDistanceLabel->setText(QString::number(value) + "m");
    simulator->setLeadDistance(value);
}

void ControlPanel::onEngagedChanged(bool checked) {
    emit engagedChanged(checked);
}

void ControlPanel::onShowLanesChanged(bool checked) {
    emit showLanesChanged(checked);
}

void ControlPanel::onRoadNameChanged(const QString& text) {
    emit roadNameChanged(text);
}
