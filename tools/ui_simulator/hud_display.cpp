// HUD Display Widget Implementation for MinPilot UI Simulator

#include "hud_display.h"
#include <QFile>
#include <QUrl>
#include <QTime>
#include <algorithm>

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
// HudDisplay Implementation
// ============================================

HudDisplay::HudDisplay(MockUIState* uiState, MockLaneSimulator* laneSim, QWidget* parent)
    : QWidget(parent), state(uiState), simulator(laneSim), elementManager(new UIElementManager()) {
    
    QScreen* screen = QApplication::primaryScreen();
    float dpiX = screen->physicalDotsPerInchX();
    
    int width = static_cast<int>(122.0f / 25.4f * dpiX);
    int height = static_cast<int>(69.0f / 25.4f * dpiX);
    
    setFixedSize(width, height);
    setStyleSheet("background: #1a1a2e;");
    
    scale = static_cast<float>(width) / 1920.0f;
    
    updateTimer = new QTimer(this);
    connect(updateTimer, &QTimer::timeout, this, &HudDisplay::onUpdate);
    updateTimer->start(50);
    
    sidebarTimer = new QTimer(this);
    connect(sidebarTimer, &QTimer::timeout, this, &HudDisplay::animateSidebar);
    
    videoPlayer = new QMediaPlayer(this);
    frameGrabber = new VideoFrameGrabber(this);
    videoPlayer->setVideoOutput(frameGrabber);
    
    connect(frameGrabber, &VideoFrameGrabber::frameAvailable, this, [this](const QImage& frame) {
        videoFrame = frame;
        update();
    });
    
    connect(videoPlayer, &QMediaPlayer::stateChanged, this, [this](QMediaPlayer::State state) {
        if (state == QMediaPlayer::StoppedState) {
            videoPlayer->play();
        }
    });
    
    QString videoPath = "/mnt/e/c2/MinPilot/tools/ui_simulator/mock/video/001/615b11d4c01d81ec_00000115--524b45920b--15--fcamera.hevc";
    if (QFile::exists(videoPath)) {
        videoPlayer->setMedia(QUrl::fromLocalFile(videoPath));
        videoPlayer->play();
        videoEnabled = true;
    }
}

void HudDisplay::setLaneMode(int mode) {
    laneMode = mode;
    update();
}

void HudDisplay::setSpeed(float speed) {
    currentSpeed = speed;
    update();
}

void HudDisplay::setMaxSpeed(float max) {
    maxSpeed = max;
    update();
}

void HudDisplay::setEngaged(bool engaged) {
    isEngaged = engaged;
    update();
}

void HudDisplay::setRoadName(const QString& name) {
    roadName = name;
    update();
}

void HudDisplay::setShowLanes(bool show) {
    showLanes = show;
    update();
}

void HudDisplay::setEditMode(bool enabled) {
    editMode = enabled;
    update();
}

void HudDisplay::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.scale(scale, scale);
    
    if (videoEnabled && !videoFrame.isNull()) {
        p.drawImage(QRect(0, 0, 1920, 1080), videoFrame.scaled(1920, 1080, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
    } else {
        p.fillRect(QRect(0, 0, 1920, 1080), QColor(26, 26, 46));
    }
    
    drawLaneGraphics(p);
    drawLeadVehicle(p);
    drawHud(p);
    drawSidebar(p);
    
    if (editMode) {
        drawEditOverlay(p);
    }
}

void HudDisplay::mousePressEvent(QMouseEvent* e) {
    float clickX = e->pos().x() / scale;
    float clickY = e->pos().y() / scale;
    
    if (editMode) {
        UIElementPosition* elem = elementManager->hitTest(clickX, clickY, 1920, 1080);
        if (elem) {
            elementManager->selectElement(elem->name);
            dragging = true;
            dragStart = QPoint(clickX, clickY);
            dragElementStart = elem->toRect(1920, 1080).topLeft();
            emit elementSelected(elem->name);
        } else {
            elementManager->clearSelection();
            emit elementSelected("");
        }
        update();
        return;
    }
    
    if (clickX < 100 && !sidebarOpen) {
        sidebarOpen = true;
        sidebarTimer->start(16);
    } else if (sidebarOpen && clickX > sidebarWidth * sidebarOffset) {
        sidebarOpen = false;
        sidebarTimer->start(16);
    }
    QWidget::mousePressEvent(e);
}

void HudDisplay::mouseMoveEvent(QMouseEvent* e) {
    if (editMode && dragging) {
        float x = e->pos().x() / scale;
        float y = e->pos().y() / scale;
        
        UIElementPosition* elem = elementManager->getSelectedElement();
        if (elem) {
            int newX = dragElementStart.x() + (x - dragStart.x());
            int newY = dragElementStart.y() + (y - dragStart.y());
            elem->setFromScreenPosition(newX, newY, 1920, 1080);
            emit elementMoved(elem->name, newX, newY);
            update();
        }
    }
    QWidget::mouseMoveEvent(e);
}

void HudDisplay::mouseReleaseEvent(QMouseEvent* e) {
    dragging = false;
    QWidget::mouseReleaseEvent(e);
}

void HudDisplay::onUpdate() {
    simulator->animate(0.05f);
    update();
}

void HudDisplay::animateSidebar() {
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

void HudDisplay::drawLaneGraphics(QPainter& p) {
    line_vertices_data laneVertices[4];
    line_vertices_data roadEdges[2];
    line_vertices_data trackVertices;
    
    simulator->getLaneVertices(laneVertices);
    simulator->getRoadEdgeVertices(roadEdges);
    simulator->getTrackVertices(trackVertices);
    
    p.setPen(Qt::NoPen);
    
    for (int edge = 0; edge < 2; edge++) {
        if (roadEdges[edge].cnt > 2) {
            p.setBrush(QColor(255, 0, 0, 150));
            p.drawPolygon(roadEdges[edge].v, roadEdges[edge].cnt);
        }
    }
    
    if (laneMode != 2 && showLanes) {
        for (int lane = 0; lane < 4; lane++) {
            if (laneVertices[lane].cnt > 2) {
                if (lane == 1 || lane == 2) {
                    p.setBrush(QColor(0, 200, 0, 200));
                } else {
                    p.setBrush(QColor(255, 255, 255, 120));
                }
                p.drawPolygon(laneVertices[lane].v, laneVertices[lane].cnt);
            }
        }
    }
    
    if (trackVertices.cnt > 2) {
        QLinearGradient pathGradient(960, 1080, 960, 540);
        if (laneMode == 2) {
            pathGradient.setColorAt(0, QColor(201, 34, 49, 255));
            pathGradient.setColorAt(1, QColor(201, 34, 49, 0));
        } else {
            pathGradient.setColorAt(0, QColor(23, 134, 68, 255));
            pathGradient.setColorAt(1, QColor(23, 134, 68, 0));
        }
        p.setBrush(pathGradient);
        p.drawPolygon(trackVertices.v, trackVertices.cnt);
    }
}

void HudDisplay::drawLeadVehicle(QPainter& p) {
    QPointF leadVerts[2];
    simulator->getLeadVertices(leadVerts);
    
    if (leadVerts[0] != QPointF(0, 0)) {
        QRectF leadRect(leadVerts[0], leadVerts[1]);
        
        float distance = simulator->getLeadDistance();
        int alpha = std::min(200, 50 + static_cast<int>(150 * (1.0f - distance / 100.0f)));
        
        p.setPen(Qt::NoPen);
        p.setBrush(QColor(255, 100, 100, alpha / 2));
        p.drawRect(leadRect.adjusted(-5, -5, 5, 5));
        
        p.setPen(QPen(QColor(255, 100, 100), 3));
        p.setBrush(QColor(255, 100, 100, alpha));
        p.drawRect(leadRect);
        
        QFont distFont("Open Sans", 16, QFont::Bold);
        p.setFont(distFont);
        p.setPen(Qt::white);
        QString distText = QString::number(static_cast<int>(distance)) + "m";
        p.drawText(leadRect.center().x() - 20, leadRect.bottom() + 25, distText);
    }
}

void HudDisplay::drawHud(QPainter& p) {
    // Header gradient
    QLinearGradient bg(0, 0, 0, 250);
    bg.setColorAt(0, QColor(0, 0, 0, 115));
    bg.setColorAt(1, QColor(0, 0, 0, 0));
    p.fillRect(0, 0, 1920, 250, bg);
    
    int bdr = 30;
    
    // MinPilot: Central speed pair (current speed | cruise speed)
    drawSpeedPair(p);
    
    // MinPilot: Top left info (clock | temperature)
    drawTopLeftInfo(p);
    
    // MinPilot: CarrotPilot speed camera info (dummy for simulator)
    drawCarrotCameraInfo(p);
    
    // Note: Engage icon removed for MinPilot UI
    // Note: MADS icon removed for MinPilot UI
    
    // DLP Button (kept)
    int dlpX = bdr * 2;
    int dlpY = 1080 - 150;
    int dlpW = 180;
    int dlpH = 80;
    
    QColor dlpColors[3] = {
        QColor(0, 125, 0),
        QColor(201, 34, 49),
        QColor(125, 0, 125)
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
    
    // MinPilot: ACC/LKAS status indicators (bottom right)
    drawStatusIndicators(p);
    
    // MinPilot: Road name bar with gradient (bottom center)
    drawRoadNameBar(p);
}

void HudDisplay::drawSidebar(QPainter& p) {
    if (sidebarOffset <= 0.0f) return;
    
    int sbX = static_cast<int>(-sidebarWidth * (1.0f - sidebarOffset));
    int sbHeight = 1080;
    
    if (sidebarOffset > 0.0f) {
        p.setBrush(QColor(0, 0, 0, static_cast<int>(100 * sidebarOffset)));
        p.setPen(Qt::NoPen);
        p.drawRect(0, 0, 1920, 1080);
    }
    
    p.setBrush(QColor(57, 57, 57));
    p.setPen(Qt::NoPen);
    p.drawRect(sbX, 0, sidebarWidth, sbHeight);
    
    p.setOpacity(0.65);
    p.setBrush(QColor(80, 80, 80));
    p.drawRoundedRect(sbX + 50, 35, 200, 117, 20, 20);
    QFont settingsFont("Open Sans", 28, QFont::Bold);
    p.setFont(settingsFont);
    p.setPen(Qt::white);
    p.setOpacity(1.0);
    p.drawText(sbX + 100, 110, "설정");
    
    int dotX = sbX + 58;
    const QColor gray(0x54, 0x54, 0x54);
    int netStrength = 4;
    for (int i = 0; i < 5; ++i) {
        p.setBrush(i < netStrength ? Qt::white : gray);
        p.setPen(Qt::NoPen);
        p.drawEllipse(dotX, 196, 27, 27);
        dotX += 37;
    }
    
    QFont netFont("Open Sans", 35);
    p.setFont(netFont);
    p.setPen(Qt::white);
    p.drawText(sbX + 50, 280, "Wi-Fi");
    
    QFont ipFont("Open Sans", 26);
    p.setFont(ipFont);
    p.setPen(QColor(0x88, 0x88, 0x88));
    p.drawText(sbX + 30, 320, "192.168.1.100");
    
    auto drawMetric = [&](const QString& label, QColor color, int y) {
        const QRect rect(sbX + 30, y, 240, label.contains("\n") ? 124 : 100);
        
        p.setPen(Qt::NoPen);
        p.setBrush(color);
        p.setClipRect(rect.x() + 6, rect.y(), 18, rect.height());
        p.drawRoundedRect(rect.x() + 6, rect.y() + 6, 100, rect.height() - 12, 10, 10);
        p.setClipping(false);
        
        QPen pen(QColor(0xff, 0xff, 0xff, 0x55));
        pen.setWidth(2);
        p.setPen(pen);
        p.setBrush(Qt::NoBrush);
        p.drawRoundedRect(rect, 20, 20);
        
        p.setPen(Qt::white);
        QFont metricFont("Open Sans", 35, QFont::Bold);
        p.setFont(metricFont);
        p.drawText(rect.adjusted(30, 0, -10, 0), Qt::AlignCenter, label);
    };
    
    drawMetric("온도\n좋음", QColor(255, 255, 255), 338);
    drawMetric("차량\n연결됨", QColor(255, 255, 255), 496);
    drawMetric(isEngaged ? "연결됨\n온라인" : "연결\n오프라인", 
               isEngaged ? QColor(255, 255, 255) : QColor(218, 202, 37), 654);
    
    p.setOpacity(1.0);
    p.setBrush(QColor(80, 80, 80));
    p.setPen(Qt::NoPen);
    int homeY = sbHeight - 180 - 40;
    p.drawEllipse(sbX + 60, homeY, 180, 180);
    
    p.setPen(QPen(Qt::white, 8));
    p.setBrush(Qt::NoBrush);
    int hx = sbX + 150;
    int hy = homeY + 90;
    p.drawLine(hx - 50, hy, hx, hy - 45);
    p.drawLine(hx, hy - 45, hx + 50, hy);
    p.drawLine(hx - 40, hy, hx - 40, hy + 50);
    p.drawLine(hx + 40, hy, hx + 40, hy + 50);
    p.drawLine(hx - 40, hy + 50, hx + 40, hy + 50);
}

void HudDisplay::drawEditOverlay(QPainter& p) {
    for (const auto& elem : elementManager->getAllElements()) {
        if (!elem.visible) continue;
        
        QRect rect = elem.toRect(1920, 1080);
        
        if (elem.selected) {
            p.setPen(QPen(QColor(0, 255, 255), 3, Qt::SolidLine));
            p.setBrush(QColor(0, 255, 255, 30));
            p.drawRect(rect);
            
            int hs = 10;
            p.setBrush(QColor(0, 255, 255));
            p.drawRect(rect.left() - hs/2, rect.top() - hs/2, hs, hs);
            p.drawRect(rect.right() - hs/2, rect.top() - hs/2, hs, hs);
            p.drawRect(rect.left() - hs/2, rect.bottom() - hs/2, hs, hs);
            p.drawRect(rect.right() - hs/2, rect.bottom() - hs/2, hs, hs);
            
            QFont labelFont("Open Sans", 14, QFont::Bold);
            p.setFont(labelFont);
            QString posLabel = QString("%1 | X:%2 Y:%3 W:%4 H:%5")
                .arg(elem.displayName)
                .arg(rect.x()).arg(rect.y())
                .arg(rect.width()).arg(rect.height());
            
            QRect labelRect(rect.left(), rect.top() - 30, 350, 25);
            p.setBrush(QColor(0, 0, 0, 180));
            p.setPen(Qt::NoPen);
            p.drawRect(labelRect);
            p.setPen(Qt::white);
            p.drawText(labelRect.adjusted(5, 0, 0, 0), Qt::AlignVCenter, posLabel);
        } else {
            p.setPen(QPen(QColor(255, 255, 255, 100), 2, Qt::DashLine));
            p.setBrush(Qt::NoBrush);
            p.drawRect(rect);
        }
    }
    
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(0, 200, 100, 200));
    p.drawRect(1920 - 200, 0, 200, 40);
    QFont bannerFont("Open Sans", 16, QFont::Bold);
    p.setFont(bannerFont);
    p.setPen(Qt::white);
    p.drawText(1920 - 190, 28, "✏️ EDIT MODE");
}

// ============================================
// MinPilot UI Functions
// ============================================

void HudDisplay::drawSpeedPair(QPainter& p) {
    // Central speed pair: [current speed] | [cruise speed]
    int centerX = 960;  // 1920 / 2
    int y = 30;
    int fontSize = 90;
    int gap = 25;  // Gap between speeds and divider

    // Current speed (left side)
    QFont speedFont("Inter", fontSize, QFont::Bold);
    p.setFont(speedFont);
    p.setPen(isBraking ? MP_ALERT : Qt::white);
    QString speedStr = QString::number((int)currentSpeed);
    QFontMetrics fm1(speedFont);
    int speedWidth = fm1.horizontalAdvance(speedStr);
    int speedHeight = fm1.height();
    int speedY = y + speedHeight;  // Speed number position (removed +25)
    p.drawText(centerX - speedWidth - gap, speedY, speedStr);

    // "현재속도" label just above current speed number
    QFont labelFont("Open Sans", 20, QFont::Bold);  // Smaller font
    p.setFont(labelFont);
    p.setPen(MP_GREY);
    QFontMetrics fmLabel(labelFont);
    int labelWidth = fmLabel.horizontalAdvance("현재속도");
    int labelHeight = fmLabel.height();
    int labelY = speedY - speedHeight + labelHeight - 5;  // Closer to number
    p.drawText(centerX - speedWidth/2 - gap - labelWidth/2, labelY, "현재속도");

    // Divider line
    p.setPen(QPen(MP_GREY, 4));
    p.drawLine(centerX, labelY - labelHeight + 5, centerX, speedY);

    // Cruise speed (right side)
    p.setFont(speedFont);
    p.setPen(isEngaged ? Qt::white : MP_GREY);
    QString cruiseStr = QString::number((int)maxSpeed);
    p.drawText(centerX + gap, speedY, cruiseStr);

    // "크루즈" label just above cruise speed number
    p.setFont(labelFont);
    p.setPen(MP_GREY);
    QFontMetrics fmCruise(speedFont);
    int cruiseWidth = fmCruise.horizontalAdvance(cruiseStr);
    int cruiseLabelWidth = fmLabel.horizontalAdvance("크루즈");
    p.drawText(centerX + gap + cruiseWidth/2 - cruiseLabelWidth/2, labelY, "크루즈");

    // Speed unit label (centered below everything)
    QFont unitFont("Open Sans", 20);
    p.setFont(unitFont);
    p.setPen(MP_GREY);
    QFontMetrics fmUnit(unitFont);
    int unitWidth = fmUnit.horizontalAdvance("km/h");
    p.drawText(centerX - unitWidth/2, speedY + 25, "km/h");
}

void HudDisplay::drawStatusIndicators(QPainter& p) {
    // ACC/LKAS status indicators at bottom right
    int x = 1920 - 220;
    int y = 1080 - 160;
    int dotSize = 20;
    int spacing = 50;

    auto drawIndicator = [&](int dy, const QString& label, bool active) {
        // Status dot
        p.setPen(Qt::NoPen);
        p.setBrush(active ? MP_SUCCESS : MP_GREY);
        p.drawEllipse(x, y + dy, dotSize, dotSize);

        // Label
        QFont font("Open Sans", 28, QFont::Bold);
        p.setFont(font);
        p.setPen(Qt::white);
        p.drawText(x + dotSize + 12, y + dy + 16, label);
    };

    // ACC: Active when engaged
    drawIndicator(0, "ACC", isEngaged);

    // LKAS: Active when MADS enabled
    drawIndicator(spacing, "LKAS", madsEnabled);
}

void HudDisplay::drawRoadNameBar(QPainter& p) {
    if (roadName.isEmpty()) return;

    const int h = 140;
    QRect bar(0, 1080 - h, 1920, h);

    // Gradient background
    QLinearGradient grad(0, bar.top(), 0, bar.bottom());
    grad.setColorAt(0, QColor(0, 0, 0, 0));
    grad.setColorAt(0.4, QColor(0, 0, 0, 100));
    grad.setColorAt(1, QColor(0, 0, 0, 160));
    p.fillRect(bar, grad);

    // Road name text
    QFont font("Malgun Gothic", 40, QFont::Bold);
    p.setFont(font);
    p.setPen(Qt::white);
    QFontMetrics fm(font);
    int textWidth = fm.horizontalAdvance(roadName);
    p.drawText(bar.center().x() - textWidth / 2, bar.center().y() + 15, roadName);
}

void HudDisplay::drawTopLeftInfo(QPainter& p) {
    // Top left: Clock | Temperature
    int x = 60;
    int y = 50;
    
    // Get current time
    QTime currentTime = QTime::currentTime();
    QString timeStr = currentTime.toString("HH:mm");
    
    // Clock
    QFont timeFont("Inter", 36, QFont::Bold);
    p.setFont(timeFont);
    p.setPen(Qt::white);
    p.drawText(x, y + 40, timeStr);
    
    // Divider
    QFontMetrics fmTime(timeFont);
    int timeWidth = fmTime.horizontalAdvance(timeStr);
    p.setPen(QPen(MP_GREY, 2));
    p.drawLine(x + timeWidth + 20, y + 10, x + timeWidth + 20, y + 45);
    
    // Temperature
    QString tempStr = QString::number(deviceTemp) + "°C";
    QFont tempFont("Inter", 32, QFont::Bold);
    p.setFont(tempFont);
    
    // Color based on temperature
    QColor tempColor = Qt::white;
    if (deviceTemp >= 80) {
        tempColor = MP_ALERT;
    } else if (deviceTemp >= 60) {
        tempColor = QColor(255, 188, 0);  // Orange
    }
    p.setPen(tempColor);
    p.drawText(x + timeWidth + 40, y + 38, tempStr);
}

void HudDisplay::drawCarrotCameraInfo(QPainter& p) {
    // CarrotPilot speed camera info
    // Dummy data for simulator - real MinPilot reads from /dev/shm/params
    
    // Position: Left center of screen
    int x = 50;
    int y = 540 - 60;  // Vertical center (1080/2 - iconSize/2)
    
    // Dummy data
    int limitSpeed = 80;
    int distanceM = 450;  // meters to camera
    QString cameraType = "고정식";  // Fixed camera
    
    // --- Camera Icon (speed limit circle) ---
    int iconSize = 120;
    p.setPen(QPen(MP_ALERT, 8));
    p.setBrush(Qt::white);
    p.drawEllipse(x, y, iconSize, iconSize);
    
    // Speed limit inside circle
    QString limitStr = QString::number(limitSpeed);
    QFont font("Inter", 48, QFont::Bold);
    p.setFont(font);
    p.setPen(Qt::black);
    QFontMetrics fm(font);
    int textWidth = fm.horizontalAdvance(limitStr);
    p.drawText(x + iconSize/2 - textWidth/2, y + iconSize/2 + 18, limitStr);
    
    // --- Info right of icon (Distance + Camera Type) ---
    int infoX = x + iconSize + 20;
    int infoY = y + iconSize/2;
    
    // Distance
    QString distStr = QString::number(distanceM) + "m";
    QFont distFont("Inter", 28, QFont::Bold);
    p.setFont(distFont);
    p.setPen(Qt::white);
    p.drawText(infoX, infoY - 5, distStr);
    
    // Camera type in parentheses
    QString typeStr = "(" + cameraType + ")";
    QFont typeFont("Open Sans", 24, QFont::Bold);
    p.setFont(typeFont);
    p.setPen(MP_GREY);
    p.drawText(infoX, infoY + 30, typeStr);
}

#include "hud_display.moc"
