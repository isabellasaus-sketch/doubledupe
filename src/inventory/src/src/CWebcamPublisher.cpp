// MTRX3760 2026 Project 2: Package Deal
// File: CWebcamPublisher.cpp
// Author(s): Christina Miljak, Davin Pranata, Joshua Samuel David, Joshua Kwok, Kaif Masiudddin Mohammed, Kieran Wong
//
// Configures a webcam and publishes frames on a ROS 2 image topic.

#include "CWebcamPublisher.hpp"


// Declare the parameters, open and configure the camera, then start publishing.
CWebcamPublisher::CWebcamPublisher()
: Node("webcam_publisher")
{
  int deviceIndex = declare_parameter<int>("device_index", kDefaultDevice);

  std::string topic = declare_parameter<std::string>("topic", kDefaultTopic);

  int width = declare_parameter<int>("width", kDefaultWidth);

  int height = declare_parameter<int>("height", kDefaultHeight);

  double fps = declare_parameter<double>("fps", kDefaultFps);

  mFrameId = declare_parameter<std::string>("frame_id", kDefaultFrameId);

  // Open the Linux video device and fail early when it is unavailable.
  if (!mCapture.open(deviceIndex, cv::CAP_V4L2)) {
    throw std::runtime_error(
            "Could not open /dev/video" + std::to_string(deviceIndex));
  }

  // MJPG lets most webcams reach 30 fps at 640x480 and above; set it first.
  mCapture.set(cv::CAP_PROP_FOURCC, cv::VideoWriter::fourcc('M', 'J', 'P', 'G'));
  mCapture.set(cv::CAP_PROP_FRAME_WIDTH, width);
  mCapture.set(cv::CAP_PROP_FRAME_HEIGHT, height);
  mCapture.set(cv::CAP_PROP_FPS, fps);

  // Best effort, matching the Gazebo bridge and real camera drivers.
  mPublisher = create_publisher<sensor_msgs::msg::Image>(
    topic, rclcpp::SensorDataQoS());

  // Schedule camera capture at the configured nominal frame rate.
  const auto period = std::chrono::duration_cast<std::chrono::nanoseconds>(
    std::chrono::duration<double>(1.0 / fps));
  mTimer = create_wall_timer(period, std::bind(&CWebcamPublisher::publishFrame, this));

  RCLCPP_INFO(
    get_logger(), "Publishing /dev/video%d on %s (%dx%d requested, %.0f fps)",
    deviceIndex, topic.c_str(), width, height, fps);
}

CWebcamPublisher::~CWebcamPublisher()
{
  mCapture.release();
}

// Skip the tick with a throttled warning if the camera gives no frame.
void CWebcamPublisher::publishFrame()
{
  cv::Mat frame;
  if (!mCapture.read(frame) || frame.empty()) {
    RCLCPP_WARN_THROTTLE(
      get_logger(), *get_clock(), kWarnThrottleMs,
      "Failed to read a frame from the camera");
    return;
  }

  // Stamp each image with the ROS clock and its camera reference frame.
  std_msgs::msg::Header header;
  header.stamp = now();
  header.frame_id = mFrameId;

  mPublisher->publish(*cv_bridge::CvImage(header, "bgr8", frame).toImageMsg());
}
