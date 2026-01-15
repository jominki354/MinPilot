#!/usr/bin/env python3
"""
Road Edge Detector for SunnyPilot
Based on dragonpilot's road_edge_detector.py
Detects road edges to prevent lane changes into dangerous areas (e.g., road shoulders)
"""

import numpy as np

# Detection thresholds
NEARSIDE_PROB = 0.2  # Lane line probability threshold
EDGE_PROB = 0.35  # Road edge probability threshold


class RoadEdgeDetector:
    def __init__(self, enabled=False):
        self._is_enabled = enabled
        self.left_edge_detected = False
        self.right_edge_detected = False

    def update(self, road_edge_stds, lane_line_probs):
        """
        Update road edge detection state.

        Args:
            road_edge_stds: List[2] of road edge standard deviations (lower = more certain)
            lane_line_probs: List[4] of lane line probabilities [left_far, left_near, right_near, right_far]
        """
        if not self._is_enabled:
            self.left_edge_detected = False
            self.right_edge_detected = False
            return

        # Convert std to probability (lower std = higher probability of edge)
        left_road_edge_prob = np.clip(1.0 - road_edge_stds[0], 0.0, 1.0)
        left_lane_nearside_prob = lane_line_probs[0]  # Leftmost lane line

        right_road_edge_prob = np.clip(1.0 - road_edge_stds[1], 0.0, 1.0)
        right_lane_nearside_prob = lane_line_probs[3]  # Rightmost lane line

        # Left edge detected when:
        # - High probability of road edge on left
        # - Low probability of lane line on left (no lane to change into)
        # - Right side has higher lane probability (we're at the edge)
        self.left_edge_detected = bool(
            left_road_edge_prob > EDGE_PROB
            and left_lane_nearside_prob < NEARSIDE_PROB
            and right_lane_nearside_prob >= left_lane_nearside_prob
        )

        # Right edge detected when:
        # - High probability of road edge on right
        # - Low probability of lane line on right (no lane to change into)
        # - Left side has higher lane probability (we're at the edge)
        self.right_edge_detected = bool(
            right_road_edge_prob > EDGE_PROB
            and right_lane_nearside_prob < NEARSIDE_PROB
            and left_lane_nearside_prob >= right_lane_nearside_prob
        )

    def set_enabled(self, enabled):
        self._is_enabled = enabled
        if not enabled:
            self.left_edge_detected = False
            self.right_edge_detected = False

    def is_enabled(self):
        return self._is_enabled

    def should_block_lane_change_left(self):
        """Returns True if left lane change should be blocked due to road edge"""
        return self._is_enabled and self.left_edge_detected

    def should_block_lane_change_right(self):
        """Returns True if right lane change should be blocked due to road edge"""
        return self._is_enabled and self.right_edge_detected
