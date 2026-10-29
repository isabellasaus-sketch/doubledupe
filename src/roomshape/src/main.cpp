// MTRX3760 2026 Project 2: Package Deal
// File: main.cpp
// Author(s): 520174067, 510146584
//
// Entry point for the room-shape ROS 2 node.

#include "roomshape/CRoomShapeNode.h"

#include <memory>

int main(int argc, char* argv[]) {
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<roomshape::CRoomShapeNode>());
  rclcpp::shutdown();
  return 0;
}
