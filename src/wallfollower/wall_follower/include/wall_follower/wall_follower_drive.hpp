// MTRX3760 2026 Project 2: Package Deal
// File: wall_follower_drive.hpp
// Author(s): Joe Egger, Max Entwistle, An Nguyen, Max Yip, Selina Ngyuen, Sarina Zaman
//
// Declares the ROS node that reads lidar and odometry, delegates wall-following
// decisions, and publishes stamped velocity commands.

#ifndef WALL_FOLLOWER__WALL_FOLLOWER_DRIVE_HPP_
#define WALL_FOLLOWER__WALL_FOLLOWER_DRIVE_HPP_

#include "wall_follower/lidar_processor.hpp"
#include "wall_follower/wall_follower_controller.hpp"
#include "wall_follower/msg/wall_follower_state.hpp"

#include <geometry_msgs/msg/twist_stamped.hpp>
#include <nav_msgs/msg/odometry.hpp>
#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/laser_scan.hpp>
#include <std_msgs/msg/header.hpp>
#include <std_srvs/srv/set_bool.hpp>
#include <std_srvs/srv/trigger.hpp>
#include <tf2/LinearMath/Matrix3x3.hpp>
#include <tf2/LinearMath/Quaternion.hpp>

#include <memory>
#include <string>

// Minimum lidar distances for the directions consumed by the controller.
struct ScanDistances
{
  double front;
  double front_right;
  double right;
};

class WallFollowerDrive : public rclcpp::Node
{
public:
  // Creates the node, ROS interfaces, and periodic controller update timer.
  WallFollowerDrive();
  // Logs node shutdown.
  ~WallFollowerDrive();

private:
  // ROS topic publishers
  rclcpp::Publisher<geometry_msgs::msg::TwistStamped>::SharedPtr cmd_vel_pub_;
  rclcpp::Publisher<wall_follower::msg::WallFollowerState>::SharedPtr state_pub_;

  // ROS topic subscribers
  rclcpp::Subscription<sensor_msgs::msg::LaserScan>::SharedPtr scan_sub_;
  rclcpp::Subscription<nav_msgs::msg::Odometry>::SharedPtr odom_sub_;

  // ROS service servers
  rclcpp::Service<std_srvs::srv::Trigger>::SharedPtr start_srv_;
  rclcpp::Service<std_srvs::srv::Trigger>::SharedPtr stop_srv_;
  rclcpp::Service<std_srvs::srv::SetBool>::SharedPtr set_running_srv_;
  rclcpp::Service<std_srvs::srv::SetBool>::SharedPtr follow_left_srv_;

  // Variables
  double robot_pose_;
  double prev_robot_pose_;
  bool has_scan_;
  rclcpp::Time last_scan_time_;
  std_msgs::msg::Header last_scan_header_;
  bool is_running_;
  ScanDistances scan_distances_{};
  std::string follow_side_;

  std::shared_ptr<wall_follower_params::ParamListener> param_listener_;
  wall_follower_params::Params params_;

  // Composed modular classes
  LidarProcessor mLidarProcessor;
  WallFollowerController mController;

  // ROS timer
  rclcpp::TimerBase::SharedPtr update_timer_;

  // Function prototypes
  // Runs the controller when scan data is available and publishes its command.
  void UpdateCallback();
  // Publishes a stamped forward and angular velocity command.
  void UpdateCommandVel(double linear, double angular);
  // Converts a LaserScan into the three distances used by the controller.
  void ScanCallback(const sensor_msgs::msg::LaserScan::SharedPtr msg);
  // Extracts the robot's yaw angle from its odometry orientation.
  void OdomCallback(const nav_msgs::msg::Odometry::SharedPtr msg);

  // Service callbacks
  void HandleStart(
    const std::shared_ptr<std_srvs::srv::Trigger::Request> request,
    std::shared_ptr<std_srvs::srv::Trigger::Response> response);
  void HandleStop(
    const std::shared_ptr<std_srvs::srv::Trigger::Request> request,
    std::shared_ptr<std_srvs::srv::Trigger::Response> response);
  void HandleSetRunning(
    const std::shared_ptr<std_srvs::srv::SetBool::Request> request,
    std::shared_ptr<std_srvs::srv::SetBool::Response> response);
  void HandleFollowLeft(
    const std::shared_ptr<std_srvs::srv::SetBool::Request> request,
    std::shared_ptr<std_srvs::srv::SetBool::Response> response);
};

#endif  // WALL_FOLLOWER__WALL_FOLLOWER_DRIVE_HPP_
