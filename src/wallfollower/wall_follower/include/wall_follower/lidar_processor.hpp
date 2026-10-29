// MTRX3760 2026 Project 2: Package Deal
// File: lidar_processor.hpp
// Author(s): Joe Egger, Max Entwistle, An Nguyen, Max Yip, Selina Ngyuen, Sarina Zaman
//
// Declares the LidarProcessor class, which extracts minimum distances from
// selected angular sectors of a LaserScan message.

#ifndef WALL_FOLLOWER__LIDAR_PROCESSOR_HPP_
#define WALL_FOLLOWER__LIDAR_PROCESSOR_HPP_

#include <sensor_msgs/msg/laser_scan.hpp>

class LidarProcessor
{
    public:
        // Creates a lidar processor with no per-instance state.
        LidarProcessor();
        // Releases the lidar processor.
        ~LidarProcessor();

        // Returns the nearest valid range within the requested clockwise scan window.
        double GetMinimumDistance(const sensor_msgs::msg::LaserScan::SharedPtr & msg, int angle_deg, int window_deg) const;

    private:
        static const double DEG2RAD;
};

#endif  // WALL_FOLLOWER__LIDAR_PROCESSOR_HPP_