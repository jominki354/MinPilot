#include "control_panel.h"
#include "element_position.h"
#include "hud_display.h"

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
    
    // === EDIT MODE (새 섹션) ===
    setupEditModeUI(layout);
    
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

void ControlPanel::setupEditModeUI(QVBoxLayout* layout) {
    // Edit mode toggle
    editModeCheck = new QCheckBox("✏️ Edit Mode (요소 위치 편집)");
    editModeCheck->setStyleSheet("font-weight: bold; color: #00ff88;");
    connect(editModeCheck, &QCheckBox::toggled, this, &ControlPanel::onEditModeToggled);
    layout->addWidget(editModeCheck);
    
    // Element editor group (initially hidden)
    elementEditGroup = new QGroupBox("📐 요소 편집");
    elementEditGroup->setVisible(false);
    auto editLayout = new QVBoxLayout(elementEditGroup);
    
    // Selected element label
    selectedElementLabel = new QLabel("선택된 요소: 없음");
    selectedElementLabel->setStyleSheet("color: cyan;");
    editLayout->addWidget(selectedElementLabel);
    
    // Position X/Y
    auto posLayout = new QHBoxLayout();
    posLayout->addWidget(new QLabel("X:"));
    posXSpin = new QSpinBox();
    posXSpin->setRange(-500, 2000);
    connect(posXSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, &ControlPanel::onPositionXChanged);
    posLayout->addWidget(posXSpin);
    
    posLayout->addWidget(new QLabel("Y:"));
    posYSpin = new QSpinBox();
    posYSpin->setRange(-500, 1500);
    connect(posYSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, &ControlPanel::onPositionYChanged);
    posLayout->addWidget(posYSpin);
    editLayout->addLayout(posLayout);
    
    // Size W/H
    auto sizeLayout = new QHBoxLayout();
    sizeLayout->addWidget(new QLabel("W:"));
    sizeWSpin = new QSpinBox();
    sizeWSpin->setRange(10, 1920);
    connect(sizeWSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, &ControlPanel::onSizeWChanged);
    sizeLayout->addWidget(sizeWSpin);
    
    sizeLayout->addWidget(new QLabel("H:"));
    sizeHSpin = new QSpinBox();
    sizeHSpin->setRange(10, 1080);
    connect(sizeHSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, &ControlPanel::onSizeHChanged);
    sizeLayout->addWidget(sizeHSpin);
    editLayout->addLayout(sizeLayout);
    
    // Anchor combo
    auto anchorLayout = new QHBoxLayout();
    anchorLayout->addWidget(new QLabel("앵커:"));
    anchorCombo = new QComboBox();
    anchorCombo->addItem("좌상단");
    anchorCombo->addItem("상단 중앙");
    anchorCombo->addItem("우상단");
    anchorCombo->addItem("좌측 중앙");
    anchorCombo->addItem("중앙");
    anchorCombo->addItem("우측 중앙");
    anchorCombo->addItem("좌하단");
    anchorCombo->addItem("하단 중앙");
    anchorCombo->addItem("우하단");
    connect(anchorCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ControlPanel::onAnchorChanged);
    anchorLayout->addWidget(anchorCombo);
    editLayout->addLayout(anchorLayout);
    
    // Export/Import buttons
    auto btnLayout = new QHBoxLayout();
    exportBtn = new QPushButton("💾 Export");
    connect(exportBtn, &QPushButton::clicked, this, &ControlPanel::onExportClicked);
    btnLayout->addWidget(exportBtn);
    
    importBtn = new QPushButton("📂 Import");
    connect(importBtn, &QPushButton::clicked, this, &ControlPanel::onImportClicked);
    btnLayout->addWidget(importBtn);
    editLayout->addLayout(btnLayout);
    
    exportCppBtn = new QPushButton("📝 Export C++ Code");
    connect(exportCppBtn, &QPushButton::clicked, this, &ControlPanel::onExportCppClicked);
    editLayout->addWidget(exportCppBtn);
    
    layout->addWidget(elementEditGroup);
}

void ControlPanel::updateElementEditor(UIElementPosition* elem) {
    if (!elem) {
        selectedElementLabel->setText("선택된 요소: 없음");
        return;
    }
    
    updatingFromCode = true;
    selectedElementLabel->setText("선택됨: " + elem->displayName);
    posXSpin->setValue(elem->offsetX);
    posYSpin->setValue(elem->offsetY);
    sizeWSpin->setValue(elem->width);
    sizeHSpin->setValue(elem->height);
    anchorCombo->setCurrentIndex(static_cast<int>(elem->anchor));
    updatingFromCode = false;
}

// Slots
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

void ControlPanel::onEditModeToggled(bool checked) {
    hudDisplay->setEditMode(checked);
    elementEditGroup->setVisible(checked);
}

void ControlPanel::onElementSelected(const QString& name) {
    currentSelectedElement = name;
    if (name.isEmpty()) {
        updateElementEditor(nullptr);
    } else {
        UIElementPosition* elem = hudDisplay->getElementManager()->getElement(name);
        updateElementEditor(elem);
    }
}

void ControlPanel::onElementMoved(const QString& name, int x, int y) {
    if (name == currentSelectedElement) {
        UIElementPosition* elem = hudDisplay->getElementManager()->getElement(name);
        if (elem) {
            updateElementEditor(elem);
        }
    }
}

void ControlPanel::onPositionXChanged(int value) {
    if (updatingFromCode) return;
    UIElementPosition* elem = hudDisplay->getElementManager()->getElement(currentSelectedElement);
    if (elem) {
        elem->offsetX = value;
    }
}

void ControlPanel::onPositionYChanged(int value) {
    if (updatingFromCode) return;
    UIElementPosition* elem = hudDisplay->getElementManager()->getElement(currentSelectedElement);
    if (elem) {
        elem->offsetY = value;
    }
}

void ControlPanel::onSizeWChanged(int value) {
    if (updatingFromCode) return;
    UIElementPosition* elem = hudDisplay->getElementManager()->getElement(currentSelectedElement);
    if (elem && elem->resizable) {
        elem->width = value;
    }
}

void ControlPanel::onSizeHChanged(int value) {
    if (updatingFromCode) return;
    UIElementPosition* elem = hudDisplay->getElementManager()->getElement(currentSelectedElement);
    if (elem && elem->resizable) {
        elem->height = value;
    }
}

void ControlPanel::onAnchorChanged(int index) {
    if (updatingFromCode) return;
    UIElementPosition* elem = hudDisplay->getElementManager()->getElement(currentSelectedElement);
    if (elem) {
        elem->anchor = static_cast<Anchor>(index);
    }
}

void ControlPanel::onExportClicked() {
    QString path = QFileDialog::getSaveFileName(this, "레이아웃 저장", 
        "layout.json", "JSON Files (*.json)");
    if (!path.isEmpty()) {
        if (hudDisplay->saveLayout(path)) {
            QMessageBox::information(this, "성공", "레이아웃이 저장되었습니다.");
        } else {
            QMessageBox::warning(this, "실패", "저장에 실패했습니다.");
        }
    }
}

void ControlPanel::onImportClicked() {
    QString path = QFileDialog::getOpenFileName(this, "레이아웃 불러오기", 
        "", "JSON Files (*.json)");
    if (!path.isEmpty()) {
        if (hudDisplay->loadLayout(path)) {
            QMessageBox::information(this, "성공", "레이아웃을 불러왔습니다.");
        } else {
            QMessageBox::warning(this, "실패", "불러오기에 실패했습니다.");
        }
    }
}

void ControlPanel::onExportCppClicked() {
    QString code = hudDisplay->exportCppCode();
    
    QDialog dialog(this);
    dialog.setWindowTitle("C++ 코드");
    dialog.resize(500, 400);
    
    auto layout = new QVBoxLayout(&dialog);
    auto textEdit = new QTextEdit();
    textEdit->setPlainText(code);
    textEdit->setReadOnly(true);
    textEdit->setFont(QFont("Consolas", 10));
    layout->addWidget(textEdit);
    
    auto closeBtn = new QPushButton("닫기");
    connect(closeBtn, &QPushButton::clicked, &dialog, &QDialog::accept);
    layout->addWidget(closeBtn);
    
    dialog.exec();
}
