#pragma once

// HUD Display Widget Header for MinPilot UI Simulator
// Separated for cross-file access

#include <QWidget>
#include <QTimer>
#include <QMediaPlayer>
#include <QAbstractVideoSurface>
#include <QVideoSurfaceFormat>
#include <QVideoFrame>
#include <QPainter>
#include <QMouseEvent>
#include <QScreen>
#include <QApplication>
#include <QFile>
#include <QUrl>
#include <QLinearGradient>
#include <QFontMetrics>

#include "element_position.h"
#include "mock/mock_ui_state.h"

// MinPilot Color Palette
const QColor MP_BLACK   = QColor(0x00, 0x00, 0x00);
const QColor MP_BLUE    = QColor(0x00, 0x88, 0xFF);
const QColor MP_GREY    = QColor(0x80, 0x80, 0x80);
const QColor MP_SUCCESS = QColor(0x00, 0xD1, 0x66);
const QColor MP_ALERT   = QColor(0xFF, 0x4D, 0x4D);

// Forward declaration
class VideoFrameGrabber;

class HudDisplay : public QWidget {
    Q_OBJECT

public:
    HudDisplay(MockUIState* uiState, MockLaneSimulator* laneSim, QWidget* parent = nullptr);
    
    void setLaneMode(int mode);
    void setSpeed(float speed);
    void setMaxSpeed(float max);
    void setEngaged(bool engaged);
    void setRoadName(const QString& name);
    void setShowLanes(bool show);
    
    // Edit mode
    void setEditMode(bool enabled);
    bool isEditMode() const { return editMode; }
    UIElementManager* getElementManager() { return elementManager; }
    
    // Export functions
    bool saveLayout(const QString& path) { return elementManager->saveToFile(path); }
    bool loadLayout(const QString& path) { return elementManager->loadFromFile(path); }
    QString exportCppCode() const { return elementManager->exportToCpp(); }
    
signals:
    void elementSelected(const QString& name);
    void elementMoved(const QString& name, int x, int y);
    
protected:
    void paintEvent(QPaintEvent*) override;
    void mousePressEvent(QMouseEvent* e) override;
    void mouseMoveEvent(QMouseEvent* e) override;
    void mouseReleaseEvent(QMouseEvent* e) override;
    
private slots:
    void onUpdate();
    void animateSidebar();
    
private:
    MockUIState* state;
    MockLaneSimulator* simulator;
    QTimer* updateTimer;
    QTimer* sidebarTimer;
    
    // Video player
    QMediaPlayer* videoPlayer = nullptr;
    VideoFrameGrabber* frameGrabber = nullptr;
    QImage videoFrame;
    bool videoEnabled = false;
    
    float scale = 0.5f;
    int laneMode = 1;
    float currentSpeed = 60.0f;
    float maxSpeed = 80.0f;
    bool isEngaged = true;
    QString roadName = "강변북로";
    bool showLanes = true;
    
    bool madsEnabled = true;
    bool showSpeedLimit = true;
    int speedLimit = 80;
    bool showVTC = false;
    int vtcSpeed = 60;
    bool isBraking = false;
    int dlpMode = 1;
    int deviceTemp = 42;  // Device temperature (C)
    
    bool sidebarOpen = false;
    float sidebarOffset = 0.0f;
    const int sidebarWidth = 300;
    
    // Edit mode
    bool editMode = false;
    UIElementManager* elementManager;
    bool dragging = false;
    QPoint dragStart;
    QPoint dragElementStart;
    
    void drawLaneGraphics(QPainter& p);
    void drawLeadVehicle(QPainter& p);
    void drawHud(QPainter& p);
    void drawSidebar(QPainter& p);
    void drawEditOverlay(QPainter& p);
    // MinPilot UI functions
    void drawSpeedPair(QPainter& p);
    void drawStatusIndicators(QPainter& p);
    void drawRoadNameBar(QPainter& p);
    void drawTopLeftInfo(QPainter& p);
    void drawCarrotCameraInfo(QPainter& p);
};
