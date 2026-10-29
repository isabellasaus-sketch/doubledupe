// MTRX3760 2026 Project 2: Package Deal
// File: path_recorder.hpp
// Author(s): Joe Egger, Max Entwistle, An Nguyen, Max Yip, Selina Ngyuen, Sarina Zaman
//
// Declares the PathRecorderNode, which samples robot odometry and publishes the
// accumulated positions as a nav_msgs/Path.

#ifndef WALL_FOLLOWER__PATH_RECORDER_HPP
#define WALL_FOLLOWER__PATH_RECORDER_HPP

#include <cmath>
#include <functional>
#include <memory>

#include "rclcpp/rclcpp.hpp"
#include "nav_msgs/msg/odometry.hpp"
#include "nav_msgs/msg/path.hpp"
#include "geometry_msgs/msg/pose_stamped.hpp"

class PathRecorderNode : public rclcpp::Node
{
    public:
        // Creates the node and connects its odometry subscription and path publisher.
        PathRecorderNode();
    
    private:
        static const double kMinStep;
        // Adds a pose to the path when the robot has moved at least kMinStep.
        void onOdom(const nav_msgs::msg::Odometry & odom); // Callback when odometry publishes

        nav_msgs::msg::Path path_;
        rclcpp::Subscription<nav_msgs::msg::Odometry>::SharedPtr odom_sub_;
        rclcpp::Publisher<nav_msgs::msg::Path>::SharedPtr path_pub_;

};

#endif 
