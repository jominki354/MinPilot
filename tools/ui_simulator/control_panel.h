#pragma once

#include <QWidget>
#include <QSlider>
#include <QLabel>
#include <QCheckBox>
#include <QComboBox>
#include <QPushButton>
#include <QLineEdit>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSpinBox>
#include <QFileDialog>
#include <QMessageBox>
#include <QTextEdit>

#include "mock/mock_ui_state.h"

// Forward declaration
class HudDisplay;
class UIElementManager;
struct UIElementPosition;

class ControlPanel : public QWidget {
    Q_OBJECT

public:
    ControlPanel(HudDisplay* hud, MockLaneSimulator* sim, QWidget* parent = nullptr);

public slots:
    void onSpeedChanged(int value);
    void onMaxSpeedChanged(int value);
    void onLaneModeChanged(int index);
    void onRoadTypeChanged(int index);
    void onCurveRadiusChanged(int value);
    void onLeadDistanceChanged(int value);
    void onEngagedChanged(bool checked);
    void onShowLanesChanged(bool checked);
    void onRoadNameChanged(const QString& text);
    
    // Edit mode slots - public for signal connection
    void onEditModeToggled(bool checked);
    void onElementSelected(const QString& name);
    void onElementMoved(const QString& name, int x, int y);
    
private slots:
    void onPositionXChanged(int value);
    void onPositionYChanged(int value);
    void onSizeWChanged(int value);
    void onSizeHChanged(int value);
    void onAnchorChanged(int index);
    void onExportClicked();
    void onImportClicked();
    void onExportCppClicked();

signals:
    void speedChanged(float value);
    void maxSpeedChanged(float value);
    void laneModeChanged(int mode);
    void engagedChanged(bool engaged);
    void showLanesChanged(bool show);
    void roadNameChanged(const QString& name);

private:
    HudDisplay* hudDisplay;
    MockLaneSimulator* simulator;
    
    // Simulation controls
    QSlider* speedSlider;
    QLabel* speedLabel;
    QSlider* maxSpeedSlider;
    QLabel* maxSpeedLabel;
    QComboBox* laneModeCombo;
    QComboBox* roadTypeCombo;
    QSlider* curveRadiusSlider;
    QLabel* curveRadiusLabel;
    QSlider* leadDistanceSlider;
    QLabel* leadDistanceLabel;
    QCheckBox* engagedCheck;
    QCheckBox* showLanesCheck;
    QLineEdit* roadNameEdit;
    
    // Edit mode controls
    QCheckBox* editModeCheck;
    QGroupBox* elementEditGroup;
    QLabel* selectedElementLabel;
    QSpinBox* posXSpin;
    QSpinBox* posYSpin;
    QSpinBox* sizeWSpin;
    QSpinBox* sizeHSpin;
    QComboBox* anchorCombo;
    QPushButton* exportBtn;
    QPushButton* importBtn;
    QPushButton* exportCppBtn;
    
    QString currentSelectedElement;
    bool updatingFromCode = false;
    
    void setupUI();
    void setupEditModeUI(QVBoxLayout* layout);
    void updateElementEditor(UIElementPosition* elem);
};
