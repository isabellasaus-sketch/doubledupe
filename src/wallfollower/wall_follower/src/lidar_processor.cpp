// MTRX3760 2026 Project 2: Package Deal
// File: lidar_processor.cpp
// Author(s): Joe Egger, Max Entwistle, An Nguyen, Max Yip, Selina Ngyuen, Sarina Zaman
//
// Implements the LidarProcessor class, which finds minimum valid ranges in
// angular sectors of a LaserScan message.

#include "wall_follower/lidar_processor.hpp"

#include <cmath>
#include <cstddef>

const double LidarProcessor::DEG2RAD = M_PI / 180.0;

LidarProcessor::LidarProcessor()
{
}

LidarProcessor::~LidarProcessor()
{
}

double LidarProcessor::GetMinimumDistance(
    const sensor_msgs::msg::LaserScan::SharedPtr & msg,
    int angle_deg,
    int window_deg) const
{
    double min_range = msg->range_max;
    if (msg->ranges.empty()) {
        return msg->range_max;
    }

    // Convert the requested clockwise direction to ROS scan's counterclockwise radians.
    const double target_angle = -static_cast<double>(angle_deg) * DEG2RAD;
    const double window_angle = static_cast<double>(window_deg) * DEG2RAD;

    for (std::size_t i = 0; i < msg->ranges.size(); ++i) {
        // LaserScan metadata defines each ray's angle; array length alone is not enough.
        const double ray_angle = msg->angle_min + static_cast<double>(i) * msg->angle_increment;
        
        // atan2(sin, cos) normalizes the difference across the -pi/pi boundary.
        const double angle_difference = std::atan2(
            std::sin(ray_angle - target_angle), std::cos(ray_angle - target_angle));
    
    
        if (std::abs(angle_difference) > window_angle) {
            continue;
        }

        // Ignore NaN, infinity, and values outside the sensor's declared range.
        const float r = msg->ranges[i];
        if (std::isfinite(r) && r >= msg->range_min && r <= msg->range_max) {
            if (r < min_range) {
            min_range = r;
            }
        }
    }
    return min_range;
}
