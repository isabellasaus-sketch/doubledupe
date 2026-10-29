// MTRX3760 2026 Project 2: Package Deal
// File: main.cpp
// Author(s): Christina Miljak, Davin Pranata, Joshua Samuel David, Joshua Kwok, Kaif Masiudddin Mohammed, Kieran Wong
//
// Starts the inventory processor node and dispatches ROS 2 callbacks.

#include "CArucoProcessor.hpp"

// Initialise ROS, spin the inventory processor, then shut down cleanly.
int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);  // Apply ROS command-line arguments before creating nodes.

  //spin the node until shutdown
  rclcpp::spin(std::make_shared<CArucoProcessor>());

  //use CTRL + C to shutdown
  rclcpp::shutdown();

  return 0;
}
