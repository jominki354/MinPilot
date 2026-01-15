#include "mock_ui_state.h"
#include <cmath>

MockLaneSimulator::MockLaneSimulator() {
    reset();
}

void MockLaneSimulator::setRoadType(RoadType type) {
    roadType = type;
    computeVertices();
}

void MockLaneSimulator::setCurveRadius(float radius) {
    curveRadius = std::max(50.0f, radius);
    computeVertices();
}

void MockLaneSimulator::setLaneCount(int count) {
    laneCount = std::clamp(count, 2, 4);
    computeVertices();
}

void MockLaneSimulator::setLaneWidth(float width) {
    laneWidth = std::clamp(width, 2.5f, 4.5f);
    computeVertices();
}

void MockLaneSimulator::animate(float dt) {
    animationTime += dt;
    computeVertices();
}

void MockLaneSimulator::reset() {
    animationTime = 0.0f;
    roadType = STRAIGHT;
    curveRadius = 500.0f;
    computeVertices();
}

void MockLaneSimulator::setLeadDistance(float distance) {
    leadDistance = std::max(5.0f, distance);
    computeVertices();
}

void MockLaneSimulator::setLeadSpeed(float relSpeed) {
    leadRelSpeed = relSpeed;
}

void MockLaneSimulator::setLeadVisible(bool visible) {
    leadVisible = visible;
    computeVertices();
}

void MockLaneSimulator::getLaneVertices(line_vertices_data vertices[4]) const {
    for (int i = 0; i < 4; i++) {
        vertices[i] = laneVertices[i];
    }
}

void MockLaneSimulator::getRoadEdgeVertices(line_vertices_data vertices[2]) const {
    vertices[0] = roadEdgeVertices[0];
    vertices[1] = roadEdgeVertices[1];
}

void MockLaneSimulator::getTrackVertices(line_vertices_data& vertices) const {
    vertices = trackVertices;
}

void MockLaneSimulator::getLeadVertices(QPointF vertices[2]) const {
    vertices[0] = leadVertices[0];
    vertices[1] = leadVertices[1];
}

QPointF MockLaneSimulator::projectPoint(float x, float y, float z) const {
    // Simple perspective projection
    // Based on openpilot's car_space_transform
    const float ZOOM = 2912.8f;
    const float y_offset = 150.0f;
    
    if (x < 1.0f) x = 1.0f;  // Prevent division by zero
    
    float px = ZOOM * y / x + 1920.0f / 2.0f;
    float py = ZOOM * z / x + 1080.0f / 2.0f + y_offset;
    
    return QPointF(px, py);
}

void MockLaneSimulator::computeVertices() {
    // Compute lane line positions based on road type
    float curvature = 0.0f;
    
    switch (roadType) {
        case STRAIGHT:
            curvature = 0.0f;
            break;
        case LEFT_CURVE:
            curvature = 1.0f / curveRadius;
            break;
        case RIGHT_CURVE:
            curvature = -1.0f / curveRadius;
            break;
        case S_CURVE:
            curvature = sin(animationTime * 0.5f) / curveRadius;
            break;
        case LANE_CHANGE_LEFT:
        case LANE_CHANGE_RIGHT:
            // Animated lane offset
            curvature = 0.0f;
            break;
    }
    
    // Lane positions (from left to right)
    float lanePositions[4];
    float totalWidth = (laneCount - 1) * laneWidth;
    float startY = -totalWidth / 2.0f - laneWidth / 2.0f;
    
    for (int i = 0; i < 4; i++) {
        if (i < laneCount + 1) {
            lanePositions[i] = startY + i * laneWidth;
        } else {
            lanePositions[i] = lanePositions[laneCount];  // Repeat last
        }
    }
    
    // Lane change animation
    float laneOffset = 0.0f;
    if (roadType == LANE_CHANGE_LEFT) {
        laneOffset = -laneWidth * (1.0f - cos(animationTime * 2.0f)) / 2.0f;
    } else if (roadType == LANE_CHANGE_RIGHT) {
        laneOffset = laneWidth * (1.0f - cos(animationTime * 2.0f)) / 2.0f;
    }
    
    // Compute vertices for each lane line (like MinPilot's update_line_data)
    // Create polygon by going forward with -y_off, then backward with +y_off
    for (int lane = 0; lane < 4; lane++) {
        laneVertices[lane].cnt = 0;
        
        float y_off = 0.15f;  // Lane line width (like openpilot's 0.025 * prob)
        
        // Forward pass (left edge of lane line)
        QPointF forwardPts[TRAJECTORY_SIZE];
        int forwardCnt = 0;
        
        for (int i = 0; i < TRAJECTORY_SIZE; i++) {
            float x = i * 5.0f;  // 0 to 160m
            float y = lanePositions[lane] + curvature * x * x / 2.0f + laneOffset;
            float z = 0.0f;
            
            QPointF p = projectPoint(x, y - y_off, z);
            
            if (p.x() > 0 && p.x() < 1920 && p.y() > 0 && p.y() < 1080) {
                forwardPts[forwardCnt++] = p;
            }
        }
        
        // Backward pass (right edge of lane line)
        QPointF backwardPts[TRAJECTORY_SIZE];
        int backwardCnt = 0;
        
        for (int i = TRAJECTORY_SIZE - 1; i >= 0; i--) {
            float x = i * 5.0f;
            float y = lanePositions[lane] + curvature * x * x / 2.0f + laneOffset;
            float z = 0.0f;
            
            QPointF p = projectPoint(x, y + y_off, z);
            
            if (p.x() > 0 && p.x() < 1920 && p.y() > 0 && p.y() < 1080) {
                backwardPts[backwardCnt++] = p;
            }
        }
        
        // Combine into closed polygon
        int idx = 0;
        for (int i = 0; i < forwardCnt && idx < TRAJECTORY_SIZE * 2; i++) {
            laneVertices[lane].v[idx++] = forwardPts[i];
        }
        for (int i = 0; i < backwardCnt && idx < TRAJECTORY_SIZE * 2; i++) {
            laneVertices[lane].v[idx++] = backwardPts[i];
        }
        laneVertices[lane].cnt = idx;
    }
    
    // Track (driving path) - same technique with larger width
    trackVertices.cnt = 0;
    float path_width = 0.9f;  // Path width (like openpilot's 0.5)
    
    QPointF trackForward[TRAJECTORY_SIZE];
    QPointF trackBackward[TRAJECTORY_SIZE];
    int trackFwdCnt = 0, trackBwdCnt = 0;
    
    for (int i = 0; i < TRAJECTORY_SIZE; i++) {
        float x = i * 5.0f;
        float y = curvature * x * x / 2.0f + laneOffset;
        float z = 1.22f;  // Height offset like openpilot
        
        QPointF pLeft = projectPoint(x, y - path_width, z);
        if (pLeft.x() > 0 && pLeft.x() < 1920 && pLeft.y() > 0 && pLeft.y() < 1080) {
            trackForward[trackFwdCnt++] = pLeft;
        }
    }
    
    for (int i = TRAJECTORY_SIZE - 1; i >= 0; i--) {
        float x = i * 5.0f;
        float y = curvature * x * x / 2.0f + laneOffset;
        float z = 1.22f;
        
        QPointF pRight = projectPoint(x, y + path_width, z);
        if (pRight.x() > 0 && pRight.x() < 1920 && pRight.y() > 0 && pRight.y() < 1080) {
            trackBackward[trackBwdCnt++] = pRight;
        }
    }
    
    int trackIdx = 0;
    for (int i = 0; i < trackFwdCnt && trackIdx < TRAJECTORY_SIZE * 2; i++) {
        trackVertices.v[trackIdx++] = trackForward[i];
    }
    for (int i = 0; i < trackBwdCnt && trackIdx < TRAJECTORY_SIZE * 2; i++) {
        trackVertices.v[trackIdx++] = trackBackward[i];
    }
    trackVertices.cnt = trackIdx;
    
    // Road edges - same technique
    float edge_width = 0.1f;
    
    for (int edgeIdx = 0; edgeIdx < 2; edgeIdx++) {
        roadEdgeVertices[edgeIdx].cnt = 0;
        float edgeY = (edgeIdx == 0) ? lanePositions[0] - 1.0f : lanePositions[laneCount] + 1.0f;
        
        QPointF edgeFwd[TRAJECTORY_SIZE];
        QPointF edgeBwd[TRAJECTORY_SIZE];
        int fwdCnt = 0, bwdCnt = 0;
        
        for (int i = 0; i < TRAJECTORY_SIZE; i++) {
            float x = i * 5.0f;
            float y = edgeY + curvature * x * x / 2.0f;
            
            QPointF p = projectPoint(x, y - edge_width, 0);
            if (p.x() > 0 && p.x() < 1920 && p.y() > 0 && p.y() < 1080) {
                edgeFwd[fwdCnt++] = p;
            }
        }
        
        for (int i = TRAJECTORY_SIZE - 1; i >= 0; i--) {
            float x = i * 5.0f;
            float y = edgeY + curvature * x * x / 2.0f;
            
            QPointF p = projectPoint(x, y + edge_width, 0);
            if (p.x() > 0 && p.x() < 1920 && p.y() > 0 && p.y() < 1080) {
                edgeBwd[bwdCnt++] = p;
            }
        }
        
        int idx = 0;
        for (int i = 0; i < fwdCnt && idx < TRAJECTORY_SIZE * 2; i++) {
            roadEdgeVertices[edgeIdx].v[idx++] = edgeFwd[i];
        }
        for (int i = 0; i < bwdCnt && idx < TRAJECTORY_SIZE * 2; i++) {
            roadEdgeVertices[edgeIdx].v[idx++] = edgeBwd[i];
        }
        roadEdgeVertices[edgeIdx].cnt = idx;
    }
    
    // Lead vehicle
    if (leadVisible) {
        float leadY = curvature * leadDistance * leadDistance / 2.0f + laneOffset;
        QPointF leadCenter = projectPoint(leadDistance, leadY, -0.5f);
        
        // Simple box representation
        float boxWidth = 100.0f / leadDistance;  // Smaller when far
        float boxHeight = 80.0f / leadDistance;
        
        leadVertices[0] = QPointF(leadCenter.x() - boxWidth, leadCenter.y());
        leadVertices[1] = QPointF(leadCenter.x() + boxWidth, leadCenter.y() - boxHeight);
    } else {
        leadVertices[0] = QPointF(0, 0);
        leadVertices[1] = QPointF(0, 0);
    }
}
