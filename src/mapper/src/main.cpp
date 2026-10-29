#include "mapper/MapperNode.hpp"

#include <memory>

int main(int argc, char * argv[])
{
  // Start ROS and create the mapper node.
  rclcpp::init(argc, argv);

  auto node = std::make_shared<MapperNode>();

  // Keep the node alive until shutdown is requested.
  rclcpp::spin(node);

  rclcpp::shutdown();
  return 0;
}

