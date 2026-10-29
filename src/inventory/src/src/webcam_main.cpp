// MTRX3760 2026 Project 2: Package Deal
// File: webcam_main.cpp
// Author(s): Christina Miljak, Davin Pranata, Joshua Samuel David, Joshua Kwok, Kaif Masiudddin Mohammed, Kieran Wong
//
// Starts the optional webcam publisher and reports camera startup errors.

#include <memory>
#include <stdexcept>

#include <rclcpp/rclcpp.hpp>

#include "CWebcamPublisher.hpp"

// Spin the webcam source separately from the inventory processing node.
int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);

  int exitCode = 0;
  // Report webcam setup failures and return a nonzero process exit code.
  try {
    rclcpp::spin(std::make_shared<CWebcamPublisher>());
  } catch (const std::runtime_error & error) {
    RCLCPP_FATAL(rclcpp::get_logger("webcam_publisher"), "%s", error.what());
    exitCode = 1;
  }

  rclcpp::shutdown();
  return exitCode;
}
