#pragma once

// Mock Messaging for MinPilot UI Simulator
// Replaces cereal SubMaster without runtime dependencies

#include <string>
#include <map>
#include <vector>
#include <array>
#include <cstdint>

// Forward declarations
namespace cereal {
    namespace ControlsState {
        enum class AlertSize { NONE, SMALL, MID, FULL };
    }
}

// Mock CarState
struct MockCarState {
    float vEgo = 0.0f;           // 속도 (m/s)
    float steeringAngleDeg = 0.0f;
    bool brakePressed = false;
    bool gasPressed = false;
    bool leftBlinker = false;
    bool rightBlinker = false;
    float aEgo = 0.0f;           // 가속도
};

// Mock ControlsState
struct MockControlsState {
    bool enabled = false;
    bool active = false;
    std::string alertText1;
    std::string alertText2;
    std::string alertType;
    cereal::ControlsState::AlertSize alertSize = cereal::ControlsState::AlertSize::NONE;
    float vCruise = 0.0f;
    bool madsEnabled = false;
};

// Mock Lead Vehicle
struct MockLeadData {
    float dRel = 100.0f;         // 상대 거리 (m)
    float vRel = 0.0f;           // 상대 속도 (m/s)
    float yRel = 0.0f;           // 횡방향 위치
    bool status = false;         // 감지 여부
};

// Mock Lane Line
struct MockLaneLine {
    float probability = 0.0f;
    std::vector<float> y;        // y좌표 (33개 포인트)
    std::vector<float> z;        // z좌표
};

// Mock ModelV2 Data
struct MockModelData {
    std::array<MockLaneLine, 4> laneLines;
    std::array<float, 2> roadEdgeStds;
    std::vector<float> pathX;
    std::vector<float> pathY;
    MockLeadData leadOne;
    MockLeadData leadTwo;
};

// Mock Lateral Plan
struct MockLateralPlan {
    bool dynamicLaneProfileStatus = true;
    float curvature = 0.0f;
    float curvatureRate = 0.0f;
};

// Mock Longitudinal Plan
struct MockLongitudinalPlan {
    int visionTurnControllerState = 0;  // DISABLED=0, ENTERING=1, TURNING=2, LEAVING=3
    float visionTurnSpeed = 0.0f;
};

// Mock Live Map Data
struct MockLiveMapData {
    std::string currentRoadName;
    std::string nextRoadName;
    float speedLimit = 0.0f;
    bool speedLimitValid = false;
};

// Mock SubMaster
class MockSubMaster {
public:
    MockSubMaster();
    
    // Compatibility with real SubMaster
    void update(const std::string& service = "");
    bool updated(const std::string& service) const;
    bool alive(const std::string& service) const;
    uint64_t rcv_frame(const std::string& service) const;
    uint64_t rcv_time(const std::string& service) const;
    
    uint64_t frame = 0;
    
    // Mock data setters
    void setCarState(const MockCarState& state);
    void setControlsState(const MockControlsState& state);
    void setModelData(const MockModelData& model);
    void setLateralPlan(const MockLateralPlan& plan);
    void setLongitudinalPlan(const MockLongitudinalPlan& plan);
    void setLiveMapData(const MockLiveMapData& mapData);
    
    // Mock data getters (for internal use)
    const MockCarState& getCarState() const { return carState; }
    const MockControlsState& getControlsState() const { return controlsState; }
    const MockModelData& getModelData() const { return modelData; }
    const MockLateralPlan& getLateralPlan() const { return lateralPlan; }
    const MockLongitudinalPlan& getLongitudinalPlan() const { return longitudinalPlan; }
    const MockLiveMapData& getLiveMapData() const { return liveMapData; }
    
private:
    MockCarState carState;
    MockControlsState controlsState;
    MockModelData modelData;
    MockLateralPlan lateralPlan;
    MockLongitudinalPlan longitudinalPlan;
    MockLiveMapData liveMapData;
    
    std::map<std::string, bool> updatedFlags;
    std::map<std::string, uint64_t> rcvFrames;
};
