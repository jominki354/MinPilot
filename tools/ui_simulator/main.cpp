// MinPilot UI Simulator - Main Entry Point
// Run with: ./ui_simulator

#include <QApplication>
#include <QWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFontDatabase>

#include "mock/mock_messaging.h"
#include "mock/mock_ui_state.h"
#include "control_panel.h"
#include "hud_display.h"

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
        
        // Control panel
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
        
        // Connect HudDisplay edit mode signals to ControlPanel
        connect(hudDisplay, &HudDisplay::elementSelected, controlPanel, &ControlPanel::onElementSelected);
        connect(hudDisplay, &HudDisplay::elementMoved, controlPanel, &ControlPanel::onElementMoved);
        
        // Window size
        QSize hudSize = hudDisplay->size();
        int panelWidth = 400;
        int windowHeight = 700;
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
