#pragma once

// UI Element Position System for MinPilot UI Simulator
// Enables drag-and-drop positioning with multiple coordinate systems

#include <QString>
#include <QRect>
#include <QPoint>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <vector>

// Anchor point for positioning
enum class Anchor {
    TOP_LEFT,
    TOP_CENTER,
    TOP_RIGHT,
    CENTER_LEFT,
    CENTER,
    CENTER_RIGHT,
    BOTTOM_LEFT,
    BOTTOM_CENTER,
    BOTTOM_RIGHT
};

// Position mode
enum class PositionMode {
    PIXEL,      // Absolute pixel offset from anchor
    PERCENT     // Percentage of screen size from anchor
};

// Single UI element position data
struct UIElementPosition {
    QString name;           // Element identifier
    QString displayName;    // Korean display name
    
    Anchor anchor = Anchor::TOP_LEFT;
    PositionMode mode = PositionMode::PIXEL;
    
    // Pixel-based offset from anchor
    int offsetX = 0;
    int offsetY = 0;
    
    // Percent-based offset (0.0 ~ 100.0)
    float percentX = 0.0f;
    float percentY = 0.0f;
    
    // Size
    int width = 100;
    int height = 100;
    bool resizable = true;
    
    // UI state
    bool selected = false;
    bool visible = true;
    
    // Calculate actual screen position
    QPoint calculatePosition(int screenWidth, int screenHeight) const;
    QRect toRect(int screenWidth, int screenHeight) const;
    
    // Update position from screen coordinates (for drag)
    void setFromScreenPosition(int x, int y, int screenWidth, int screenHeight);
    
    // Serialization
    QJsonObject toJson() const;
    static UIElementPosition fromJson(const QJsonObject& obj);
    
    // Generate C++ code snippet
    QString toCppCode() const;
};

// Collection of all UI elements
class UIElementManager {
public:
    UIElementManager();
    
    // Element access
    void addElement(const UIElementPosition& element);
    UIElementPosition* getElement(const QString& name);
    const std::vector<UIElementPosition>& getAllElements() const { return elements; }
    
    // Selection
    UIElementPosition* getSelectedElement();
    void selectElement(const QString& name);
    void clearSelection();
    UIElementPosition* hitTest(int x, int y, int screenWidth, int screenHeight);
    
    // Serialization
    bool saveToFile(const QString& path);
    bool loadFromFile(const QString& path);
    QString exportToCpp() const;
    
    // Initialize with default MinPilot layout
    void initializeDefaults();
    
private:
    std::vector<UIElementPosition> elements;
};

// Anchor helper functions
QString anchorToString(Anchor anchor);
Anchor stringToAnchor(const QString& str);
QPoint anchorPoint(Anchor anchor, int screenWidth, int screenHeight);
