#pragma once

// Mock UIState for MinPilot UI Simulator
// Replaces UIState without cereal runtime dependencies

#include <QString>
#include <QPointF>
#include <QTransform>
#include <array>
#include <memory>
#include "mock_messaging.h"

// From modeldata.h
#define TRAJECTORY_SIZE 33

// From ui.h - line_vertices_data
typedef struct {
    QPointF v[TRAJECTORY_SIZE * 2];
    int cnt;
} line_vertices_data;

// UI Status
enum UIStatus {
    STATUS_DISENGAGED = 0,
    STATUS_ENGAGED = 1,
    STATUS_WARNING = 2,
    STATUS_ALERT = 3,
};

// Mock UIScene (simplified version of ui.h UIScene)
struct MockUIScene {
    // Lane Mode
    int dynamic_lane_profile = 0;  // 0=Auto, 1=Lane, 2=Laneless
    bool end_to_end = false;
    
    // Debug UI
    bool show_debug_ui = true;
    bool debug_snapshot_enabled = false;
    int dev_ui_enabled = 1;
    
    // Speed limit
    bool speed_limit_control_enabled = false;
    
    // Radar info
    bool show_radar_info = false;
    
    // Lane lines (4 lines)
    float lane_line_probs[4] = {0.3f, 1.0f, 1.0f, 0.3f};
    line_vertices_data lane_line_vertices[4];
    
    // Road edges
    float road_edge_stds[2] = {1.0f, 1.0f};
    line_vertices_data road_edge_vertices[2];
    
    // Track (path)
    line_vertices_data track_vertices;
    
    // Lead vehicles
    QPointF lead_vertices[2];
    
    // State
    bool started = true;
    bool ignition = true;
    bool is_metric = true;
    bool longitudinal_control = true;
    uint64_t started_frame = 0;
    
    // Sensors
    float light_sensor = 100.0f;
    float accel_sensor = 0.0f;
    float gyro_sensor = 0.0f;
    
    // Lateral plan
    struct {
        bool dynamicLaneProfileStatus = true;
    } lateralPlan;
};

// Mock UIState
struct MockUIState {
    int fb_w = 1920;
    int fb_h = 1080;
    
    std::unique_ptr<MockSubMaster> sm;
    
    UIStatus status = STATUS_DISENGAGED;
    MockUIScene scene;
    
    bool awake = true;
    bool has_prime = false;
    
    QTransform car_space_transform;
    bool wide_camera = false;
    
    MockUIState() {
        sm = std::make_unique<MockSubMaster>();
    }
};

// Lane Simulator for dynamic graphics
class MockLaneSimulator {
public:
    enum RoadType {
        STRAIGHT,
        LEFT_CURVE,
        RIGHT_CURVE,
        S_CURVE,
        LANE_CHANGE_LEFT,
        LANE_CHANGE_RIGHT
    };
    
    MockLaneSimulator();
    
    // Road configuration
    void setRoadType(RoadType type);
    void setCurveRadius(float radius);  // meters
    void setLaneCount(int count);       // 2, 3, or 4
    void setLaneWidth(float width);     // meters
    
    // Animation
    void animate(float dt);  // dt in seconds
    void reset();
    
    // Get computed vertices (call after animate)
    void getLaneVertices(line_vertices_data vertices[4]) const;
    void getRoadEdgeVertices(line_vertices_data vertices[2]) const;
    void getTrackVertices(line_vertices_data& vertices) const;
    void getLeadVertices(QPointF vertices[2]) const;
    float getLeadDistance() const { return leadDistance; }
    
    // Lead vehicle
    void setLeadDistance(float distance);
    void setLeadSpeed(float relSpeed);
    void setLeadVisible(bool visible);
    
private:
    RoadType roadType = STRAIGHT;
    float curveRadius = 500.0f;
    int laneCount = 3;
    float laneWidth = 3.7f;
    
    float animationTime = 0.0f;
    
    // Lead vehicle
    float leadDistance = 30.0f;
    float leadRelSpeed = 0.0f;
    bool leadVisible = true;
    
    // Computed data
    line_vertices_data laneVertices[4];
    line_vertices_data roadEdgeVertices[2];
    line_vertices_data trackVertices;
    QPointF leadVertices[2];
    
    void computeVertices();
    QPointF projectPoint(float x, float y, float z) const;
};
