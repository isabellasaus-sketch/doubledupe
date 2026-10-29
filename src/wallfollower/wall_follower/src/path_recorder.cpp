// MTRX3760 2026 Project 2: Package Deal
// File: path_recorder.cpp
// Author(s): Joe Egger, Max Entwistle, An Nguyen, Max Yip, Selina Ngyuen, Sarina Zaman
//
// Implements the PathRecorderNode, which samples odometry poses and publishes
// the robot's accumulated path.

#include <cmath>
#include <functional>
#include <memory>

#include "wall_follower/path_recorder.hpp"
#include "rclcpp/rclcpp.hpp"
#include "nav_msgs/msg/odometry.hpp"
#include "nav_msgs/msg/path.hpp"
#include "geometry_msgs/msg/pose_stamped.hpp"

const double PathRecorderNode::kMinStep = 0.02;

PathRecorderNode::PathRecorderNode() 
  : Node("path_recorder")
{
	// initialise publisher & subscriber data
	rclcpp::QoS qos(rclcpp::KeepLast(10)); // NOTE: PLACEHOLDER, HAVEN'T CHECKED THAT THIS QOS IS APPROPRIATE, ALSO MAY WANT TO MOVE INTO PRIVATE (?)

	odom_sub_ = this->create_subscription<nav_msgs::msg::Odometry>(
	"odom", qos, std::bind(&PathRecorderNode::onOdom, this, std::placeholders::_1));

	path_pub_ = create_publisher<nav_msgs::msg::Path>("path", qos);

}

void PathRecorderNode::onOdom(const nav_msgs::msg::Odometry & odom)
{
	// Only record when the robot has moved, so the path doesn't grow forever
	const geometry_msgs::msg::Point & p = odom.pose.pose.position;
	if (!path_.poses.empty()) {
		const geometry_msgs::msg::Point & last = path_.poses.back().pose.position;
		// Skip near-duplicate poses so the published path only grows after meaningful motion.
		if (std::hypot(p.x - last.x, p.y - last.y) < kMinStep) {
		return;
		}
	}

	geometry_msgs::msg::PoseStamped point;
	// Preserve odometry's frame and timestamp so recorded poses stay in the same reference frame.
	point.header = odom.header;      // same frame ("odom") and timestamp
	point.pose = odom.pose.pose;

	path_.header = odom.header;
	path_.poses.push_back(point);
	path_pub_->publish(path_);
}


int main(int argc, char ** argv)
{
	// Initialize ROS, run the path-recorder node, then shut ROS down cleanly.
	rclcpp::init(argc, argv);
	rclcpp::spin(std::make_shared<PathRecorderNode>());
	rclcpp::shutdown();
	return 0;
}