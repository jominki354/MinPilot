#include "mock_messaging.h"

MockSubMaster::MockSubMaster() {
    // Initialize with default values
    frame = 0;
    
    // Set all services as updated initially
    updatedFlags["carState"] = true;
    updatedFlags["controlsState"] = true;
    updatedFlags["modelV2"] = true;
    updatedFlags["lateralPlan"] = true;
    updatedFlags["longitudinalPlan"] = true;
    updatedFlags["liveMapData"] = true;
    
    // Initialize lane lines with straight road
    for (int i = 0; i < 4; i++) {
        modelData.laneLines[i].probability = (i == 1 || i == 2) ? 1.0f : 0.3f;
        modelData.laneLines[i].y.resize(33, 0.0f);
        modelData.laneLines[i].z.resize(33, 0.0f);
        
        // Default lane positions (left to right)
        float baseY = -3.7f + i * 1.85f;
        for (int j = 0; j < 33; j++) {
            modelData.laneLines[i].y[j] = baseY;
            modelData.laneLines[i].z[j] = 0.0f;
        }
    }
    
    // Initialize path
    modelData.pathX.resize(33);
    modelData.pathY.resize(33);
    for (int i = 0; i < 33; i++) {
        modelData.pathX[i] = i * 5.0f;  // 0 to 160m
        modelData.pathY[i] = 0.0f;      // Straight ahead
    }
}

void MockSubMaster::update(const std::string& service) {
    frame++;
    
    // Update all services
    for (auto& pair : updatedFlags) {
        pair.second = true;
        rcvFrames[pair.first] = frame;
    }
}

bool MockSubMaster::updated(const std::string& service) const {
    auto it = updatedFlags.find(service);
    return it != updatedFlags.end() ? it->second : false;
}

bool MockSubMaster::alive(const std::string& service) const {
    return true;  // Always alive in mock
}

uint64_t MockSubMaster::rcv_frame(const std::string& service) const {
    auto it = rcvFrames.find(service);
    return it != rcvFrames.end() ? it->second : frame;
}

uint64_t MockSubMaster::rcv_time(const std::string& service) const {
    return 0;  // Not used in mock
}

void MockSubMaster::setCarState(const MockCarState& state) {
    carState = state;
    updatedFlags["carState"] = true;
}

void MockSubMaster::setControlsState(const MockControlsState& state) {
    controlsState = state;
    updatedFlags["controlsState"] = true;
}

void MockSubMaster::setModelData(const MockModelData& model) {
    modelData = model;
    updatedFlags["modelV2"] = true;
}

void MockSubMaster::setLateralPlan(const MockLateralPlan& plan) {
    lateralPlan = plan;
    updatedFlags["lateralPlan"] = true;
}

void MockSubMaster::setLongitudinalPlan(const MockLongitudinalPlan& plan) {
    longitudinalPlan = plan;
    updatedFlags["longitudinalPlan"] = true;
}

void MockSubMaster::setLiveMapData(const MockLiveMapData& mapData) {
    liveMapData = mapData;
    updatedFlags["liveMapData"] = true;
}
