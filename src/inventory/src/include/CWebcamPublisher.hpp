// MTRX3760 2026 Project 2: Package Deal
// File: CWebcamPublisher.hpp
// Author(s): Christina Miljak, Davin Pranata, Joshua Samuel David, Joshua Kwok, Kaif Masiudddin Mohammed, Kieran Wong
//
// Declares the optional webcam ROS node that publishes camera images.

#ifndef CWEBCAMPUBLISHER_HPP_
#define CWEBCAMPUBLISHER_HPP_

#include <chrono>
#include <functional>
#include <stdexcept>
#include <string>

#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>
#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/image.hpp>
#include <cv_bridge/cv_bridge.hpp>
#include <std_msgs/msg/header.hpp>

// Optional image source for testing inventory detection without a TurtleBot.
class CWebcamPublisher : public rclcpp::Node
{
public:
  // Throws std::runtime_error if the camera cannot be opened.
  CWebcamPublisher();

  ~CWebcamPublisher() override;

private:
  // Reads one frame from the camera and publishes it.
  void publishFrame();

  // Defaults are exposed as ROS parameters in the constructor.
  int kDefaultDevice = 0;

  int kDefaultWidth = 640;

  int kDefaultHeight = 480;

  double kDefaultFps = 30.0;

  int kWarnThrottleMs = 2000;

  const char * kDefaultTopic = "camera/image_raw";

  const char * kDefaultFrameId = "camera_rgb_optical_frame";

  // OpenCV capture device from which individual frames are read.
  cv::VideoCapture mCapture;

  std::string mFrameId;

  // Publishes sensor_msgs/Image frames to the configured topic.
  rclcpp::Publisher<sensor_msgs::msg::Image>::SharedPtr mPublisher;

  rclcpp::TimerBase::SharedPtr mTimer;

};

#endif  // CWEBCAMPUBLISHER_HPP_
