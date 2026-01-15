#include "element_position.h"
#include <QFile>
#include <QTextStream>

// ============================================
// Anchor Helper Functions
// ============================================

QString anchorToString(Anchor anchor) {
    switch (anchor) {
        case Anchor::TOP_LEFT: return "TOP_LEFT";
        case Anchor::TOP_CENTER: return "TOP_CENTER";
        case Anchor::TOP_RIGHT: return "TOP_RIGHT";
        case Anchor::CENTER_LEFT: return "CENTER_LEFT";
        case Anchor::CENTER: return "CENTER";
        case Anchor::CENTER_RIGHT: return "CENTER_RIGHT";
        case Anchor::BOTTOM_LEFT: return "BOTTOM_LEFT";
        case Anchor::BOTTOM_CENTER: return "BOTTOM_CENTER";
        case Anchor::BOTTOM_RIGHT: return "BOTTOM_RIGHT";
        default: return "TOP_LEFT";
    }
}

Anchor stringToAnchor(const QString& str) {
    if (str == "TOP_LEFT") return Anchor::TOP_LEFT;
    if (str == "TOP_CENTER") return Anchor::TOP_CENTER;
    if (str == "TOP_RIGHT") return Anchor::TOP_RIGHT;
    if (str == "CENTER_LEFT") return Anchor::CENTER_LEFT;
    if (str == "CENTER") return Anchor::CENTER;
    if (str == "CENTER_RIGHT") return Anchor::CENTER_RIGHT;
    if (str == "BOTTOM_LEFT") return Anchor::BOTTOM_LEFT;
    if (str == "BOTTOM_CENTER") return Anchor::BOTTOM_CENTER;
    if (str == "BOTTOM_RIGHT") return Anchor::BOTTOM_RIGHT;
    return Anchor::TOP_LEFT;
}

QPoint anchorPoint(Anchor anchor, int screenWidth, int screenHeight) {
    switch (anchor) {
        case Anchor::TOP_LEFT: return QPoint(0, 0);
        case Anchor::TOP_CENTER: return QPoint(screenWidth / 2, 0);
        case Anchor::TOP_RIGHT: return QPoint(screenWidth, 0);
        case Anchor::CENTER_LEFT: return QPoint(0, screenHeight / 2);
        case Anchor::CENTER: return QPoint(screenWidth / 2, screenHeight / 2);
        case Anchor::CENTER_RIGHT: return QPoint(screenWidth, screenHeight / 2);
        case Anchor::BOTTOM_LEFT: return QPoint(0, screenHeight);
        case Anchor::BOTTOM_CENTER: return QPoint(screenWidth / 2, screenHeight);
        case Anchor::BOTTOM_RIGHT: return QPoint(screenWidth, screenHeight);
        default: return QPoint(0, 0);
    }
}

// ============================================
// UIElementPosition Implementation
// ============================================

QPoint UIElementPosition::calculatePosition(int screenWidth, int screenHeight) const {
    QPoint anchor_pt = anchorPoint(anchor, screenWidth, screenHeight);
    
    int x, y;
    if (mode == PositionMode::PIXEL) {
        x = anchor_pt.x() + offsetX;
        y = anchor_pt.y() + offsetY;
    } else {
        x = anchor_pt.x() + static_cast<int>(screenWidth * percentX / 100.0f);
        y = anchor_pt.y() + static_cast<int>(screenHeight * percentY / 100.0f);
    }
    
    // Adjust for anchor alignment
    switch (anchor) {
        case Anchor::TOP_CENTER:
        case Anchor::CENTER:
        case Anchor::BOTTOM_CENTER:
            x -= width / 2;
            break;
        case Anchor::TOP_RIGHT:
        case Anchor::CENTER_RIGHT:
        case Anchor::BOTTOM_RIGHT:
            x -= width;
            break;
        default:
            break;
    }
    
    switch (anchor) {
        case Anchor::CENTER_LEFT:
        case Anchor::CENTER:
        case Anchor::CENTER_RIGHT:
            y -= height / 2;
            break;
        case Anchor::BOTTOM_LEFT:
        case Anchor::BOTTOM_CENTER:
        case Anchor::BOTTOM_RIGHT:
            y -= height;
            break;
        default:
            break;
    }
    
    return QPoint(x, y);
}

QRect UIElementPosition::toRect(int screenWidth, int screenHeight) const {
    QPoint pos = calculatePosition(screenWidth, screenHeight);
    return QRect(pos.x(), pos.y(), width, height);
}

void UIElementPosition::setFromScreenPosition(int x, int y, int screenWidth, int screenHeight) {
    QPoint anchor_pt = anchorPoint(anchor, screenWidth, screenHeight);
    
    // Adjust for element center based on anchor type
    int offsetFromAnchorX = x - anchor_pt.x();
    int offsetFromAnchorY = y - anchor_pt.y();
    
    if (mode == PositionMode::PIXEL) {
        offsetX = offsetFromAnchorX;
        offsetY = offsetFromAnchorY;
    } else {
        percentX = (offsetFromAnchorX * 100.0f) / screenWidth;
        percentY = (offsetFromAnchorY * 100.0f) / screenHeight;
    }
}

QJsonObject UIElementPosition::toJson() const {
    QJsonObject obj;
    obj["name"] = name;
    obj["displayName"] = displayName;
    obj["anchor"] = anchorToString(anchor);
    obj["mode"] = (mode == PositionMode::PIXEL) ? "PIXEL" : "PERCENT";
    obj["offsetX"] = offsetX;
    obj["offsetY"] = offsetY;
    obj["percentX"] = static_cast<double>(percentX);
    obj["percentY"] = static_cast<double>(percentY);
    obj["width"] = width;
    obj["height"] = height;
    obj["resizable"] = resizable;
    obj["visible"] = visible;
    return obj;
}

UIElementPosition UIElementPosition::fromJson(const QJsonObject& obj) {
    UIElementPosition elem;
    elem.name = obj["name"].toString();
    elem.displayName = obj["displayName"].toString();
    elem.anchor = stringToAnchor(obj["anchor"].toString());
    elem.mode = (obj["mode"].toString() == "PIXEL") ? PositionMode::PIXEL : PositionMode::PERCENT;
    elem.offsetX = obj["offsetX"].toInt();
    elem.offsetY = obj["offsetY"].toInt();
    elem.percentX = static_cast<float>(obj["percentX"].toDouble());
    elem.percentY = static_cast<float>(obj["percentY"].toDouble());
    elem.width = obj["width"].toInt();
    elem.height = obj["height"].toInt();
    elem.resizable = obj["resizable"].toBool(true);
    elem.visible = obj["visible"].toBool(true);
    return elem;
}

QString UIElementPosition::toCppCode() const {
    QString code;
    
    if (mode == PositionMode::PIXEL) {
        switch (anchor) {
            case Anchor::TOP_LEFT:
                code = QString("QRect %1_rect(%2, %3, %4, %5);")
                    .arg(name).arg(offsetX).arg(offsetY).arg(width).arg(height);
                break;
            case Anchor::TOP_RIGHT:
                code = QString("QRect %1_rect(rect().right() - %2 - %4, %3, %4, %5);")
                    .arg(name).arg(-offsetX).arg(offsetY).arg(width).arg(height);
                break;
            case Anchor::TOP_CENTER:
                code = QString("QRect %1_rect(rect().center().x() + %2 - %4/2, %3, %4, %5);")
                    .arg(name).arg(offsetX).arg(offsetY).arg(width).arg(height);
                break;
            case Anchor::BOTTOM_LEFT:
                code = QString("QRect %1_rect(%2, rect().bottom() - %3 - %5, %4, %5);")
                    .arg(name).arg(offsetX).arg(-offsetY).arg(width).arg(height);
                break;
            case Anchor::BOTTOM_RIGHT:
                code = QString("QRect %1_rect(rect().right() - %2 - %4, rect().bottom() - %3 - %5, %4, %5);")
                    .arg(name).arg(-offsetX).arg(-offsetY).arg(width).arg(height);
                break;
            case Anchor::CENTER:
                code = QString("QRect %1_rect(rect().center().x() + %2 - %4/2, rect().center().y() + %3 - %5/2, %4, %5);")
                    .arg(name).arg(offsetX).arg(offsetY).arg(width).arg(height);
                break;
            default:
                code = QString("// %1: complex anchor not supported").arg(name);
        }
    } else {
        code = QString("// %1: percentage mode - convert to pixels for production").arg(name);
    }
    
    return code;
}

// ============================================
// UIElementManager Implementation
// ============================================

UIElementManager::UIElementManager() {
    initializeDefaults();
}

void UIElementManager::addElement(const UIElementPosition& element) {
    elements.push_back(element);
}

UIElementPosition* UIElementManager::getElement(const QString& name) {
    for (auto& elem : elements) {
        if (elem.name == name) return &elem;
    }
    return nullptr;
}

UIElementPosition* UIElementManager::getSelectedElement() {
    for (auto& elem : elements) {
        if (elem.selected) return &elem;
    }
    return nullptr;
}

void UIElementManager::selectElement(const QString& name) {
    for (auto& elem : elements) {
        elem.selected = (elem.name == name);
    }
}

void UIElementManager::clearSelection() {
    for (auto& elem : elements) {
        elem.selected = false;
    }
}

UIElementPosition* UIElementManager::hitTest(int x, int y, int screenWidth, int screenHeight) {
    // Reverse order to hit top-most elements first
    for (int i = elements.size() - 1; i >= 0; --i) {
        if (elements[i].visible && elements[i].toRect(screenWidth, screenHeight).contains(x, y)) {
            return &elements[i];
        }
    }
    return nullptr;
}

bool UIElementManager::saveToFile(const QString& path) {
    QJsonArray arr;
    for (const auto& elem : elements) {
        arr.append(elem.toJson());
    }
    
    QJsonObject root;
    root["version"] = "1.0";
    root["resolution"] = QJsonArray{1920, 1080};
    root["elements"] = arr;
    
    QFile file(path);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
        file.close();
        return true;
    }
    return false;
}

bool UIElementManager::loadFromFile(const QString& path) {
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) return false;
    
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();
    
    if (!doc.isObject()) return false;
    
    QJsonObject root = doc.object();
    QJsonArray arr = root["elements"].toArray();
    
    elements.clear();
    for (const auto& val : arr) {
        elements.push_back(UIElementPosition::fromJson(val.toObject()));
    }
    
    return true;
}

QString UIElementManager::exportToCpp() const {
    QString code;
    code += "// Generated by MinPilot UI Simulator\n";
    code += "// Copy this code to onroad.cc\n\n";
    
    for (const auto& elem : elements) {
        code += elem.toCppCode() + "\n";
    }
    
    return code;
}

void UIElementManager::initializeDefaults() {
    elements.clear();
    
    // Max Speed Box
    UIElementPosition maxSpeed;
    maxSpeed.name = "max_speed_box";
    maxSpeed.displayName = "최고속도 박스";
    maxSpeed.anchor = Anchor::TOP_LEFT;
    maxSpeed.mode = PositionMode::PIXEL;
    maxSpeed.offsetX = 60;
    maxSpeed.offsetY = 45;
    maxSpeed.width = 184;
    maxSpeed.height = 202;
    elements.push_back(maxSpeed);
    
    // Current Speed
    UIElementPosition currSpeed;
    currSpeed.name = "current_speed";
    currSpeed.displayName = "현재 속도";
    currSpeed.anchor = Anchor::TOP_CENTER;
    currSpeed.mode = PositionMode::PIXEL;
    currSpeed.offsetX = 0;
    currSpeed.offsetY = 120;
    currSpeed.width = 200;
    currSpeed.height = 120;
    currSpeed.resizable = false;
    elements.push_back(currSpeed);
    
    // Engage Icon
    UIElementPosition engage;
    engage.name = "engage_icon";
    engage.displayName = "Engage 아이콘";
    engage.anchor = Anchor::TOP_RIGHT;
    engage.mode = PositionMode::PIXEL;
    engage.offsetX = -60;
    engage.offsetY = 45;
    engage.width = 192;
    engage.height = 192;
    elements.push_back(engage);
    
    // MADS Icon
    UIElementPosition mads;
    mads.name = "mads_icon";
    mads.displayName = "MADS 아이콘";
    mads.anchor = Anchor::TOP_RIGHT;
    mads.mode = PositionMode::PIXEL;
    mads.offsetX = -244;
    mads.offsetY = 45;
    mads.width = 92;
    mads.height = 92;
    elements.push_back(mads);
    
    // Speed Limit Sign
    UIElementPosition speedLimit;
    speedLimit.name = "speed_limit";
    speedLimit.displayName = "속도제한 표지판";
    speedLimit.anchor = Anchor::TOP_LEFT;
    speedLimit.mode = PositionMode::PIXEL;
    speedLimit.offsetX = 60;
    speedLimit.offsetY = 265;
    speedLimit.width = 130;
    speedLimit.height = 130;
    elements.push_back(speedLimit);
    
    // DLP Button
    UIElementPosition dlp;
    dlp.name = "dlp_button";
    dlp.displayName = "DLP 버튼";
    dlp.anchor = Anchor::BOTTOM_LEFT;
    dlp.mode = PositionMode::PIXEL;
    dlp.offsetX = 60;
    dlp.offsetY = -150;
    dlp.width = 180;
    dlp.height = 80;
    elements.push_back(dlp);
    
    // DM Icon
    UIElementPosition dm;
    dm.name = "dm_icon";
    dm.displayName = "DM 아이콘";
    dm.anchor = Anchor::BOTTOM_LEFT;
    dm.mode = PositionMode::PIXEL;
    dm.offsetX = 60;
    dm.offsetY = -300;
    dm.width = 96;
    dm.height = 96;
    elements.push_back(dm);
    
    // Road Name Bar
    UIElementPosition roadBar;
    roadBar.name = "road_name_bar";
    roadBar.displayName = "도로명 바";
    roadBar.anchor = Anchor::BOTTOM_CENTER;
    roadBar.mode = PositionMode::PIXEL;
    roadBar.offsetX = 0;
    roadBar.offsetY = 0;
    roadBar.width = 1920;
    roadBar.height = 60;
    roadBar.resizable = false;
    elements.push_back(roadBar);
}
