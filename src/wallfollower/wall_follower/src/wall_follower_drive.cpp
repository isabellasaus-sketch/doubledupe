// MTRX3760 2026 Project 2: Package Deal
// File: wall_follower_drive.cpp
// Author(s): Joseph Egger, Max Yip, Max Entwistle, Sarina Zaman, Selina Nguyen, An Nguyen
//
// Implements the WallFollowerDrive ROS node, which reads sensor data, requests
// a controller command, publishes velocity commands and node state, and provides
// services for runtime start, stop, and left/right wall following control.

#include "wall_follower/wall_follower_drive.hpp"

#include <cmath>
#include <memory>

using namespace std::chrono_literals;

WallFollowerDrive::WallFollowerDrive()
: Node("wall_follower_drive_node"),
  robot_pose_(0.0),
  prev_robot_pose_(0.0),
  has_scan_(false),
  last_scan_time_(0, 0, RCL_ROS_TIME),
  is_running_(true),
  follow_side_("right"),
  mController(wall_follower_params::Params())
{
  param_listener_ =
    std::make_shared<wall_follower_params::ParamListener>(get_node_parameters_interface());
  params_ = param_listener_->get_params();
  mController = WallFollowerController(params_);

  follow_side_ = params_.follow_left ? "left" : "right";
  is_running_ = params_.start_running;

  rclcpp::QoS qos = rclcpp::QoS(rclcpp::KeepLast(10));

  // Initialise publishers
  cmd_vel_pub_ = this->create_publisher<geometry_msgs::msg::TwistStamped>("cmd_vel", qos);
  state_pub_ = this->create_publisher<wall_follower::msg::WallFollowerState>("wall_follower/state",
    qos);

  // Initialise subscribers
  scan_sub_ = this->create_subscription<sensor_msgs::msg::LaserScan>(
    "scan",
    rclcpp::SensorDataQoS(),
    std::bind(
      &WallFollowerDrive::ScanCallback,
      this,
      std::placeholders::_1));

  odom_sub_ = this->create_subscription<nav_msgs::msg::Odometry>(
    "odom", qos,
    std::bind(&WallFollowerDrive::OdomCallback, this, std::placeholders::_1));

  // Initialise services
  start_srv_ = this->create_service<std_srvs::srv::Trigger>(
    "wall_follower/start",
    std::bind(&WallFollowerDrive::HandleStart, this, std::placeholders::_1,
    std::placeholders::_2));

  stop_srv_ = this->create_service<std_srvs::srv::Trigger>(
    "wall_follower/stop",
    std::bind(&WallFollowerDrive::HandleStop, this, std::placeholders::_1,
    std::placeholders::_2));

  set_running_srv_ = this->create_service<std_srvs::srv::SetBool>(
    "wall_follower/set_running",
    std::bind(&WallFollowerDrive::HandleSetRunning, this, std::placeholders::_1,
    std::placeholders::_2));

  follow_left_srv_ = this->create_service<std_srvs::srv::SetBool>(
    "wall_follower/follow_left",
    std::bind(&WallFollowerDrive::HandleFollowLeft, this, std::placeholders::_1,
    std::placeholders::_2));

  /************************************************************
   ** Initialise ROS timers
  ************************************************************/
  update_timer_ = this->create_wall_timer(10ms,
    std::bind(&WallFollowerDrive::UpdateCallback, this));

  RCLCPP_INFO(this->get_logger(), "Wall Follower node initialised (follow_side: %s, running: %s)",
    follow_side_.c_str(), is_running_ ? "true" : "false");
}

WallFollowerDrive::~WallFollowerDrive()
{
  RCLCPP_INFO(this->get_logger(), "Wall Follower node terminated");
}

/********************************************************************************
** Callback functions for ROS subscribers
********************************************************************************/
void WallFollowerDrive::OdomCallback(const nav_msgs::msg::Odometry::SharedPtr msg)
{
  // Convert the odometry quaternion to yaw for the robot's planar heading.
  tf2::Quaternion q(
    msg->pose.pose.orientation.x,
    msg->pose.pose.orientation.y,
    msg->pose.pose.orientation.z,
    msg->pose.pose.orientation.w);
  tf2::Matrix3x3 m(q);
  double roll, pitch, yaw;
  m.getRPY(roll, pitch, yaw);

  robot_pose_ = yaw;
}

void WallFollowerDrive::ScanCallback(const sensor_msgs::msg::LaserScan::SharedPtr msg)
{
  last_scan_header_ = msg->header;
  last_scan_time_ = this->get_clock()->now();

  scan_distances_.front = mLidarProcessor.GetMinimumDistance(msg, 0, 5);
  const bool follow_left = follow_side_ == "left";
  scan_distances_.front_right = mLidarProcessor.GetMinimumDistance(
    msg, follow_left ? -45 : 45, 5);
  scan_distances_.right = mLidarProcessor.GetMinimumDistance(
    msg, follow_left ? -90 : 90, 5);

  has_scan_ = true;
}

void WallFollowerDrive::UpdateCommandVel(double linear, double angular)
{
  geometry_msgs::msg::TwistStamped cmd_vel;
  cmd_vel.header.stamp = this->get_clock()->now();
  cmd_vel.twist.linear.x = linear;
  cmd_vel.twist.angular.z = angular;

  cmd_vel_pub_->publish(cmd_vel);
}

void WallFollowerDrive::UpdateCallback()
{
  // Check if parameters have been dynamically updated
  if (param_listener_->is_old(params_)) {
    const auto new_params = param_listener_->get_params();
    if (new_params.follow_left != params_.follow_left) {
      follow_side_ = new_params.follow_left ? "left" : "right";
    }
    if (new_params.start_running != params_.start_running) {
      is_running_ = new_params.start_running;
    }
    params_ = new_params;
    mController = WallFollowerController(params_);
  }

  // Assess scan validity and timeout
  bool scan_valid = false;
  if (has_scan_) {
    const double elapsed = (this->get_clock()->now() - last_scan_time_).seconds();
    if (elapsed >= 0.0 && elapsed <= params_.scan_timeout) {
      scan_valid = true;
    }
  }

  // Determine high-level state
  uint8_t state = wall_follower::msg::WallFollowerState::PAUSED;
  if (!is_running_) {
    state = wall_follower::msg::WallFollowerState::PAUSED;
  } else if (!scan_valid) {
    state = wall_follower::msg::WallFollowerState::WAITING_FOR_SCAN;
  } else {
    state = wall_follower::msg::WallFollowerState::FOLLOWING;
  }

  // Compute movement commands according to state
  VelocityCommand cmd{0.0, 0.0};
  if (state == wall_follower::msg::WallFollowerState::FOLLOWING) {
    cmd = mController.CalculateCommand(
      scan_distances_.front, scan_distances_.front_right, scan_distances_.right);
    if (follow_side_ == "left") {
      cmd.angular = -cmd.angular;
    }
  }

  // Publish velocity command
  UpdateCommandVel(cmd.linear, cmd.angular);

  // Construct and publish status interface
  wall_follower::msg::WallFollowerState state_msg;
  if (!has_scan_) {
    state_msg.header.stamp.sec = 0;
    state_msg.header.stamp.nanosec = 0;
    state_msg.header.frame_id = "";
  } else {
    state_msg.header = last_scan_header_;
  }

  state_msg.state = state;
  state_msg.follow_left = follow_side_ == "left";
  state_msg.scan_valid = scan_valid;

  state_pub_->publish(state_msg);
}

void WallFollowerDrive::HandleStart(
  const std::shared_ptr<std_srvs::srv::Trigger::Request> request,
  std::shared_ptr<std_srvs::srv::Trigger::Response> response)
{
  (void)request;
  is_running_ = true;
  response->success = true;
  response->message = "Wall follower started";
  RCLCPP_INFO(this->get_logger(), "%s", response->message.c_str());
}

void WallFollowerDrive::HandleStop(
  const std::shared_ptr<std_srvs::srv::Trigger::Request> request,
  std::shared_ptr<std_srvs::srv::Trigger::Response> response)
{
  (void)request;
  is_running_ = false;
  UpdateCommandVel(0.0, 0.0);
  response->success = true;
  response->message = "Wall follower stopped";
  RCLCPP_INFO(this->get_logger(), "%s", response->message.c_str());
}

void WallFollowerDrive::HandleSetRunning(
  const std::shared_ptr<std_srvs::srv::SetBool::Request> request,
  std::shared_ptr<std_srvs::srv::SetBool::Response> response)
{
  is_running_ = request->data;
  if (!is_running_) {
    UpdateCommandVel(0.0, 0.0);
  }
  response->success = true;
  response->message = is_running_ ? "Wall follower started (running)" : "Wall follower stopped";
  RCLCPP_INFO(this->get_logger(), "%s", response->message.c_str());
}

void WallFollowerDrive::HandleFollowLeft(
  const std::shared_ptr<std_srvs::srv::SetBool::Request> request,
  std::shared_ptr<std_srvs::srv::SetBool::Response> response)
{
  follow_side_ = request->data ? "left" : "right";
  response->success = true;
  response->message = (follow_side_ == "left") ? "Following left wall" : "Following right wall";
  RCLCPP_INFO(this->get_logger(), "%s", response->message.c_str());
}

int main(int argc, char ** argv)
{
  // Initialize ROS, run the wall-follower node, then shut ROS down cleanly.
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<WallFollowerDrive>());
  rclcpp::shutdown();

  return 0;
}
