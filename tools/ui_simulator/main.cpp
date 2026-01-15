// MinPilot UI Simulator - Main Entry Point
// Run with: ./ui_simulator

#include <QApplication>
#include <QWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QSlider>
#include <QLabel>
#include <QCheckBox>
#include <QComboBox>
#include <QGroupBox>
#include <QLineEdit>
#include <QTimer>
#include <QDebug>
#include <QFontDatabase>
#include <QScreen>
#include <QPainter>
#include <QPen>
#include <QBrush>
#include <QLinearGradient>
#include <QFontMetrics>
#include <QMouseEvent>
#include <QMediaPlayer>
#include <QVideoWidget>
#include <QAbstractVideoSurface>
#include <QVideoSurfaceFormat>
#include <QVideoFrame>

#include "mock/mock_messaging.h"
#include "mock/mock_ui_state.h"
#include "control_panel.h"

// ============================================
// Video Frame Grabber (captures video frames as QImage)
// ============================================
class VideoFrameGrabber : public QAbstractVideoSurface {
    Q_OBJECT

public:
    VideoFrameGrabber(QObject* parent = nullptr) : QAbstractVideoSurface(parent) {}

    QList<QVideoFrame::PixelFormat> supportedPixelFormats(
        QAbstractVideoBuffer::HandleType type) const override {
        Q_UNUSED(type);
        return QList<QVideoFrame::PixelFormat>()
            << QVideoFrame::Format_RGB32
            << QVideoFrame::Format_ARGB32
            << QVideoFrame::Format_ARGB32_Premultiplied
            << QVideoFrame::Format_RGB565
            << QVideoFrame::Format_RGB555;
    }

    bool present(const QVideoFrame& frame) override {
        if (frame.isValid()) {
            QVideoFrame cloneFrame(frame);
            cloneFrame.map(QAbstractVideoBuffer::ReadOnly);
            
            QImage::Format imageFormat = QVideoFrame::imageFormatFromPixelFormat(cloneFrame.pixelFormat());
            if (imageFormat != QImage::Format_Invalid) {
                currentFrame = QImage(
                    cloneFrame.bits(),
                    cloneFrame.width(),
                    cloneFrame.height(),
                    cloneFrame.bytesPerLine(),
                    imageFormat
                ).copy();
                emit frameAvailable(currentFrame);
            }
            cloneFrame.unmap();
            return true;
        }
        return false;
    }

    QImage getCurrentFrame() const { return currentFrame; }

signals:
    void frameAvailable(const QImage& frame);

private:
    QImage currentFrame;
};

// ============================================
// HUD Display Widget (simplified onroad.cc style)
// ============================================
class HudDisplay : public QWidget {
    Q_OBJECT

public:
    HudDisplay(MockUIState* uiState, MockLaneSimulator* laneSim, QWidget* parent = nullptr)
        : QWidget(parent), state(uiState), simulator(laneSim) {
        
        // Le Pro 3 resolution with scaling
        QScreen* screen = QApplication::primaryScreen();
        float dpiX = screen->physicalDotsPerInchX();
        
        // 122mm width = 4.8 inches
        int width = static_cast<int>(122.0f / 25.4f * dpiX);
        int height = static_cast<int>(69.0f / 25.4f * dpiX);
        
        setFixedSize(width, height);
        setStyleSheet("background: #1a1a2e;");
        
        scale = static_cast<float>(width) / 1920.0f;
        
        // Update timer (20 Hz like openpilot)
        updateTimer = new QTimer(this);
        connect(updateTimer, &QTimer::timeout, this, &HudDisplay::onUpdate);
        updateTimer->start(50);  // 50ms = 20Hz
        
        // Sidebar animation timer
        sidebarTimer = new QTimer(this);
        connect(sidebarTimer, &QTimer::timeout, this, &HudDisplay::animateSidebar);
        
        // Video player setup
        videoPlayer = new QMediaPlayer(this);
        frameGrabber = new VideoFrameGrabber(this);
        videoPlayer->setVideoOutput(frameGrabber);
        
        connect(frameGrabber, &VideoFrameGrabber::frameAvailable, this, [this](const QImage& frame) {
            videoFrame = frame;
            update();
        });
        
        // Loop video
        connect(videoPlayer, &QMediaPlayer::stateChanged, this, [this](QMediaPlayer::State state) {
            if (state == QMediaPlayer::StoppedState) {
                videoPlayer->play();  // Loop
            }
        });
        
        // Try to load video
        QString videoPath = "/mnt/e/c2/MinPilot/tools/ui_simulator/mock/video/001/615b11d4c01d81ec_00000115--524b45920b--15--fcamera.hevc";
        if (QFile::exists(videoPath)) {
            videoPlayer->setMedia(QUrl::fromLocalFile(videoPath));
            videoPlayer->play();
            videoEnabled = true;
        }
    }
    
    void setLaneMode(int mode) {
        laneMode = mode;
        update();
    }
    
    void setSpeed(float speed) {
        currentSpeed = speed;
        update();
    }
    
    void setMaxSpeed(float max) {
        maxSpeed = max;
        update();
    }
    
    void setEngaged(bool engaged) {
        isEngaged = engaged;
        update();
    }
    
    void setRoadName(const QString& name) {
        roadName = name;
        update();
    }
    
    void setShowLanes(bool show) {
        showLanes = show;
        update();
    }
    
protected:
    void paintEvent(QPaintEvent*) override {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        p.scale(scale, scale);
        
        // Background - video frame or solid color
        if (videoEnabled && !videoFrame.isNull()) {
            p.drawImage(QRect(0, 0, 1920, 1080), videoFrame.scaled(1920, 1080, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
        } else {
            p.fillRect(QRect(0, 0, 1920, 1080), QColor(26, 26, 46));
        }
        
        // Draw lane lines and path
        drawLaneGraphics(p);
        
        // Draw lead vehicle
        drawLeadVehicle(p);
        
        // Draw HUD overlay
        drawHud(p);
        
        // Draw Sidebar (on top)
        drawSidebar(p);
    }
    
    void mousePressEvent(QMouseEvent* e) override {
        // Click on left edge opens sidebar
        float clickX = e->pos().x() / scale;
        if (clickX < 100 && !sidebarOpen) {
            sidebarOpen = true;
            sidebarTimer->start(16);  // ~60 FPS animation
        } else if (sidebarOpen && clickX > sidebarWidth * sidebarOffset) {
            // Click outside sidebar closes it
            sidebarOpen = false;
            sidebarTimer->start(16);
        }
        QWidget::mousePressEvent(e);
    }
    
private slots:
    void onUpdate() {
        // Animate lane simulator
        simulator->animate(0.05f);
        update();
    }
    
    void animateSidebar() {
        float target = sidebarOpen ? 1.0f : 0.0f;
        float step = 0.1f;
        
        if (sidebarOpen) {
            sidebarOffset = std::min(sidebarOffset + step, target);
        } else {
            sidebarOffset = std::max(sidebarOffset - step, target);
        }
        
        if (sidebarOffset == target) {
            sidebarTimer->stop();
        }
        update();
    }
    
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
    int laneMode = 1;  // 0=Auto, 1=Lane, 2=Laneless
    float currentSpeed = 60.0f;
    float maxSpeed = 80.0f;
    bool isEngaged = true;
    QString roadName = "강변북로";
    bool showLanes = true;
    
    // 추가 UI 요소
    bool madsEnabled = true;
    bool showSpeedLimit = true;
    int speedLimit = 80;
    bool showVTC = false;
    int vtcSpeed = 60;
    bool isBraking = false;
    int dlpMode = 1;  // 0=Auto, 1=Lane, 2=Laneless
    
    // Sidebar
    bool sidebarOpen = false;
    float sidebarOffset = 0.0f;  // 0=closed, 1=fully open
    const int sidebarWidth = 300;
    
    void drawLaneGraphics(QPainter& p) {
        // Get lane vertices from simulator (already closed polygons)
        line_vertices_data laneVertices[4];
        line_vertices_data roadEdges[2];
        line_vertices_data trackVertices;
        
        simulator->getLaneVertices(laneVertices);
        simulator->getRoadEdgeVertices(roadEdges);
        simulator->getTrackVertices(trackVertices);
        
        p.setPen(Qt::NoPen);
        
        // Draw road edges (red, like openpilot)
        for (int edge = 0; edge < 2; edge++) {
            if (roadEdges[edge].cnt > 2) {
                p.setBrush(QColor(255, 0, 0, 150));  // Red for road edges
                p.drawPolygon(roadEdges[edge].v, roadEdges[edge].cnt);
            }
        }
        
        // Draw lane lines based on mode (Lane mode only)
        if (laneMode != 2 && showLanes) {
            for (int lane = 0; lane < 4; lane++) {
                if (laneVertices[lane].cnt > 2) {
                    // Color: inner lanes are green, outer are white
                    if (lane == 1 || lane == 2) {
                        p.setBrush(QColor(0, 200, 0, 200));  // Green
                    } else {
                        p.setBrush(QColor(255, 255, 255, 120));  // White
                    }
                    p.drawPolygon(laneVertices[lane].v, laneVertices[lane].cnt);
                }
            }
        }
        
        // Draw track (path) with gradient - ALWAYS visible
        if (trackVertices.cnt > 2) {
            // Gradient from bottom to top
            QLinearGradient pathGradient(960, 1080, 960, 540);
            if (laneMode == 2) {
                // Laneless mode: Red path (dynamicLaneProfileStatus = true)
                pathGradient.setColorAt(0, QColor(201, 34, 49, 255));
                pathGradient.setColorAt(1, QColor(201, 34, 49, 0));
            } else {
                // Lane mode: Green path
                pathGradient.setColorAt(0, QColor(23, 134, 68, 255));
                pathGradient.setColorAt(1, QColor(23, 134, 68, 0));
            }
            p.setBrush(pathGradient);
            p.drawPolygon(trackVertices.v, trackVertices.cnt);
        }
    }
    
    void drawLeadVehicle(QPainter& p) {
        QPointF leadVerts[2];
        simulator->getLeadVertices(leadVerts);
        
        if (leadVerts[0] != QPointF(0, 0)) {
            // Lead vehicle box with gradient
            QRectF leadRect(leadVerts[0], leadVerts[1]);
            
            // Glowing effect based on distance
            float distance = simulator->getLeadDistance();
            int alpha = std::min(200, 50 + static_cast<int>(150 * (1.0f - distance / 100.0f)));
            
            // Draw glow
            p.setPen(Qt::NoPen);
            p.setBrush(QColor(255, 100, 100, alpha / 2));
            p.drawRect(leadRect.adjusted(-5, -5, 5, 5));
            
            // Draw main box
            p.setPen(QPen(QColor(255, 100, 100), 3));
            p.setBrush(QColor(255, 100, 100, alpha));
            p.drawRect(leadRect);
            
            // Draw distance text
            QFont distFont("Open Sans", 16, QFont::Bold);
            p.setFont(distFont);
            p.setPen(Qt::white);
            QString distText = QString::number(static_cast<int>(distance)) + "m";
            p.drawText(leadRect.center().x() - 20, leadRect.bottom() + 25, distText);
        }
    }
    
    void drawHud(QPainter& p) {
        // Header gradient
        QLinearGradient bg(0, 0, 0, 250);
        bg.setColorAt(0, QColor(0, 0, 0, 115));
        bg.setColorAt(1, QColor(0, 0, 0, 0));
        p.fillRect(0, 0, 1920, 250, bg);
        
        // Max speed box
        int bdr = 30;
        QRect maxRect(bdr * 2, bdr * 1.5, 184, 202);
        p.setPen(QPen(QColor(255, 255, 255, 100), 10));
        p.setBrush(QColor(0, 0, 0, 100));
        p.drawRoundedRect(maxRect, 20, 20);
        
        // Max speed text
        QFont labelFont("Open Sans", 24);
        p.setFont(labelFont);
        p.setPen(QColor(255, 255, 255, 200));
        p.drawText(maxRect.center().x() - 50, 118, "최고속도");
        
        QFont speedFont("Open Sans", 44, QFont::Bold);
        p.setFont(speedFont);
        p.setPen(Qt::white);
        p.drawText(maxRect.center().x() - 30, 212, QString::number((int)maxSpeed));
        
        // Current speed
        QFont bigFont("Open Sans", 88, QFont::Bold);
        p.setFont(bigFont);
        QString speedStr = QString::number((int)currentSpeed);
        QFontMetrics fm(bigFont);
        int speedWidth = fm.horizontalAdvance(speedStr);
        p.drawText(960 - speedWidth/2, 210, speedStr);
        
        // Speed unit
        QFont unitFont("Open Sans", 33);
        p.setFont(unitFont);
        p.setPen(QColor(255, 255, 255, 200));
        p.drawText(925, 290, "km/h");
        
        // Status icon (Engaged)
        p.setPen(Qt::NoPen);
        p.setBrush(isEngaged ? QColor(23, 162, 73) : QColor(75, 75, 75));
        p.drawEllipse(1920 - 192 - bdr*2, bdr*1.5, 192, 192);
        
        // MADS icon (below status icon)
        if (madsEnabled) {
            p.setBrush(QColor(0, 125, 0, 200));
            p.drawEllipse(1920 - 142 - bdr*2, bdr*1.5 + 210, 92, 92);
            QFont madsFont("Open Sans", 12, QFont::Bold);
            p.setFont(madsFont);
            p.setPen(Qt::white);
            p.drawText(1920 - 130 - bdr*2, bdr*1.5 + 265, "MADS");
        }
        
        // Speed limit sign (left side)
        if (showSpeedLimit && speedLimit > 0) {
            int signX = bdr * 2;
            int signY = bdr * 1.5 + 220;
            int signSize = 130;
            
            // Red circle
            p.setPen(QPen(QColor(200, 0, 0), 12));
            p.setBrush(Qt::white);
            p.drawEllipse(signX, signY, signSize, signSize);
            
            // Speed number
            QFont signFont("Open Sans", 36, QFont::Bold);
            p.setFont(signFont);
            p.setPen(Qt::black);
            QString limitStr = QString::number(speedLimit);
            QFontMetrics signFm(signFont);
            int limitWidth = signFm.horizontalAdvance(limitStr);
            p.drawText(signX + signSize/2 - limitWidth/2, signY + signSize/2 + 15, limitStr);
        }
        
        // VTC indicator (Turn Speed)
        if (showVTC) {
            int vtcX = bdr * 2;
            int vtcY = bdr * 1.5 + 370;
            
            p.setPen(QPen(QColor(218, 111, 37), 3));
            p.setBrush(QColor(218, 111, 37, 150));
            QRect vtcRect(vtcX, vtcY, 100, 60);
            p.drawRoundedRect(vtcRect, 10, 10);
            
            QFont vtcFont("Open Sans", 24, QFont::Bold);
            p.setFont(vtcFont);
            p.setPen(Qt::white);
            p.drawText(vtcX + 20, vtcY + 42, QString::number(vtcSpeed));
        }
        
        // DLP Button (bottom left)
        int dlpX = bdr * 2;
        int dlpY = 1080 - 150;
        int dlpW = 180;
        int dlpH = 80;
        
        QColor dlpColors[3] = {
            QColor(0, 125, 0),    // Auto - green
            QColor(201, 34, 49),  // Lane - red
            QColor(125, 0, 125)   // Laneless - purple
        };
        QString dlpLabels[3] = {"Auto", "Lane", "Laneless"};
        
        p.setPen(Qt::NoPen);
        p.setBrush(dlpColors[laneMode]);
        p.drawRoundedRect(dlpX, dlpY, dlpW, dlpH, 15, 15);
        
        QFont dlpFont("Open Sans", 22, QFont::Bold);
        p.setFont(dlpFont);
        p.setPen(Qt::white);
        QFontMetrics dlpFm(dlpFont);
        int dlpTextWidth = dlpFm.horizontalAdvance(dlpLabels[laneMode]);
        p.drawText(dlpX + dlpW/2 - dlpTextWidth/2, dlpY + dlpH/2 + 8, dlpLabels[laneMode]);
        
        // Road name bar
        if (!roadName.isEmpty()) {
            QRect barRect(0, 1020, 1920, 60);
            p.setBrush(QColor(0, 0, 0, 100));
            p.setPen(Qt::NoPen);
            p.drawRect(barRect);
            
            QFont roadFont("Malgun Gothic", 19, QFont::Bold);
            p.setFont(roadFont);
            p.setPen(QColor(255, 255, 255, 200));
            QFontMetrics roadFm(roadFont);
            int roadWidth = roadFm.horizontalAdvance(roadName);
            p.drawText(960 - roadWidth/2, 1057, roadName);
        }
    }
    
    void drawSidebar(QPainter& p) {
        if (sidebarOffset <= 0.0f) return;
        
        // Sidebar position (slides in from left)
        int sbX = static_cast<int>(-sidebarWidth * (1.0f - sidebarOffset));
        int sbHeight = 1080;
        
        // Dim background when sidebar is open
        if (sidebarOffset > 0.0f) {
            p.setBrush(QColor(0, 0, 0, static_cast<int>(100 * sidebarOffset)));
            p.setPen(Qt::NoPen);
            p.drawRect(0, 0, 1920, 1080);
        }
        
        // ========================================
        // SIDEBAR BACKGROUND (openpilot style: #393939)
        // ========================================
        p.setBrush(QColor(57, 57, 57));
        p.setPen(Qt::NoPen);
        p.drawRect(sbX, 0, sidebarWidth, sbHeight);
        
        // ========================================
        // SETTINGS BUTTON (top)
        // ========================================
        p.setOpacity(0.65);
        p.setBrush(QColor(80, 80, 80));
        p.drawRoundedRect(sbX + 50, 35, 200, 117, 20, 20);
        QFont settingsFont("Open Sans", 28, QFont::Bold);
        p.setFont(settingsFont);
        p.setPen(Qt::white);
        p.setOpacity(1.0);
        p.drawText(sbX + 100, 110, "설정");
        
        // ========================================
        // NETWORK SIGNAL (5 dots)
        // ========================================
        int dotX = sbX + 58;
        const QColor gray(0x54, 0x54, 0x54);
        int netStrength = 4;  // Mock: 4/5 signal
        for (int i = 0; i < 5; ++i) {
            p.setBrush(i < netStrength ? Qt::white : gray);
            p.setPen(Qt::NoPen);
            p.drawEllipse(dotX, 196, 27, 27);
            dotX += 37;
        }
        
        // Network type
        QFont netFont("Open Sans", 35);
        p.setFont(netFont);
        p.setPen(Qt::white);
        p.drawText(sbX + 50, 280, "Wi-Fi");
        
        // IP Address
        QFont ipFont("Open Sans", 26);
        p.setFont(ipFont);
        p.setPen(QColor(0x88, 0x88, 0x88));
        p.drawText(sbX + 30, 320, "192.168.1.100");
        
        // ========================================
        // STATUS METRICS (3개)
        // ========================================
        // Helper lambda for metric drawing
        auto drawMetric = [&](const QString& label, QColor color, int y) {
            const QRect rect(sbX + 30, y, 240, label.contains("\n") ? 124 : 100);
            
            // Color bar on left
            p.setPen(Qt::NoPen);
            p.setBrush(color);
            p.setClipRect(rect.x() + 6, rect.y(), 18, rect.height());
            p.drawRoundedRect(rect.x() + 6, rect.y() + 6, 100, rect.height() - 12, 10, 10);
            p.setClipping(false);
            
            // Border
            QPen pen(QColor(0xff, 0xff, 0xff, 0x55));
            pen.setWidth(2);
            p.setPen(pen);
            p.setBrush(Qt::NoBrush);
            p.drawRoundedRect(rect, 20, 20);
            
            // Text
            p.setPen(Qt::white);
            QFont metricFont("Open Sans", 35, QFont::Bold);
            p.setFont(metricFont);
            p.drawText(rect.adjusted(30, 0, -10, 0), Qt::AlignCenter, label);
        };
        
        // Temperature status
        drawMetric("온도\n좋음", QColor(255, 255, 255), 338);
        
        // Panda status
        drawMetric("차량\n연결됨", QColor(255, 255, 255), 496);
        
        // Connect status
        drawMetric(isEngaged ? "연결됨\n온라인" : "연결\n오프라인", 
                   isEngaged ? QColor(255, 255, 255) : QColor(218, 202, 37), 654);
        
        // ========================================
        // HOME BUTTON (bottom)
        // ========================================
        p.setOpacity(1.0);
        p.setBrush(QColor(80, 80, 80));
        p.setPen(Qt::NoPen);
        int homeY = sbHeight - 180 - 40;
        p.drawEllipse(sbX + 60, homeY, 180, 180);
        
        // Home icon (simple house shape)
        p.setPen(QPen(Qt::white, 8));
        p.setBrush(Qt::NoBrush);
        int hx = sbX + 150;
        int hy = homeY + 90;
        // Roof
        p.drawLine(hx - 50, hy, hx, hy - 45);
        p.drawLine(hx, hy - 45, hx + 50, hy);
        // Walls
        p.drawLine(hx - 40, hy, hx - 40, hy + 50);
        p.drawLine(hx + 40, hy, hx + 40, hy + 50);
        p.drawLine(hx - 40, hy + 50, hx + 40, hy + 50);
    }
};

// ============================================
// Main Window
// ============================================
class MainWindow : public QWidget {
public:
    MainWindow() {
        setWindowTitle("MinPilot UI Simulator (Le Pro 3: 5.5\")");
        
        // Create mock state and simulator
        mockState = new MockUIState();
        laneSimulator = new MockLaneSimulator();
        
        auto layout = new QHBoxLayout(this);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->setSpacing(0);
        
        // HUD display
        hudDisplay = new HudDisplay(mockState, laneSimulator);
        layout->addWidget(hudDisplay);
        
        // Control panel (1080p에 적합한 크기)
        controlPanel = new ControlPanel(hudDisplay, laneSimulator);
        controlPanel->setStyleSheet("background: #2d2d44; color: white; font-size: 12px;");
        controlPanel->setFixedWidth(400);
        layout->addWidget(controlPanel);
        
        // Connect signals from control panel to HUD display
        connect(controlPanel, &ControlPanel::speedChanged, hudDisplay, &HudDisplay::setSpeed);
        connect(controlPanel, &ControlPanel::maxSpeedChanged, hudDisplay, &HudDisplay::setMaxSpeed);
        connect(controlPanel, &ControlPanel::laneModeChanged, hudDisplay, &HudDisplay::setLaneMode);
        connect(controlPanel, &ControlPanel::engagedChanged, hudDisplay, &HudDisplay::setEngaged);
        connect(controlPanel, &ControlPanel::showLanesChanged, hudDisplay, &HudDisplay::setShowLanes);
        connect(controlPanel, &ControlPanel::roadNameChanged, hudDisplay, &HudDisplay::setRoadName);
        
        // Window size: HUD (실제 5.5인치) + 컨트롤 패널
        // 1920x1080 모니터에 적합한 크기로 조절
        QSize hudSize = hudDisplay->size();
        int panelWidth = 400;  // 컨트롤 패널 너비
        int windowHeight = 700; // 1080p 모니터에 적합한 높이
        setFixedSize(hudSize.width() + panelWidth, windowHeight);
    }
    
    ~MainWindow() {
        delete mockState;
        delete laneSimulator;
    }
    
private:
    MockUIState* mockState;
    MockLaneSimulator* laneSimulator;
    HudDisplay* hudDisplay;
    ControlPanel* controlPanel;
};

// ============================================
// Main
// ============================================
int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    
    // Load fonts
    QString fontPath = "/mnt/e/c2/MinPilot/selfdrive/assets/fonts/";
    QFontDatabase::addApplicationFont(fontPath + "opensans_regular.ttf");
    QFontDatabase::addApplicationFont(fontPath + "opensans_bold.ttf");
    QFontDatabase::addApplicationFont(fontPath + "Inter-Regular.ttf");
    QFontDatabase::addApplicationFont(fontPath + "Inter-Bold.ttf");
    
    QFont defaultFont("Open Sans", 12);
    app.setFont(defaultFont);
    
    MainWindow window;
    window.show();
    
    return app.exec();
}

#include "main.moc"
