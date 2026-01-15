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

#include "mock/mock_ui_state.h"

// Forward declaration
class HudDisplay;

class ControlPanel : public QWidget {
    Q_OBJECT

public:
    ControlPanel(HudDisplay* hud, MockLaneSimulator* sim, QWidget* parent = nullptr);

private slots:
    void onSpeedChanged(int value);
    void onMaxSpeedChanged(int value);
    void onLaneModeChanged(int index);
    void onRoadTypeChanged(int index);
    void onCurveRadiusChanged(int value);
    void onLeadDistanceChanged(int value);
    void onEngagedChanged(bool checked);
    void onShowLanesChanged(bool checked);
    void onRoadNameChanged(const QString& text);

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
    
    void setupUI();
};
