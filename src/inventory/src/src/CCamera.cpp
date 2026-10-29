// MTRX3760 2026 Project 2: Package Deal
// File: CCamera.cpp
// Author(s): Christina Miljak, Davin Pranata, Joshua Samuel David, Joshua Kwok, Kaif Masiudddin Mohammed, Kieran Wong
//
// Receives ROS camera images and converts them to grayscale OpenCV frames.

#include "CCamera.hpp"

// Subscribe through the owning processor node without creating a second node.
CCamera::CCamera(rclcpp::Node & node)
    :mImage(nullptr), //images are both of types cv_bridge::CvImageConstPtr, so we need to make sure they point to nothing
    mGreyscaleImage()
{
    image_sub_ = node.create_subscription<sensor_msgs::msg::Image>(
    "camera/image_raw", rclcpp::SensorDataQoS(),  // Use the sensor-data delivery settings for camera images.
    std::bind(&CCamera::image_callback, this, std::placeholders::_1));
    RCLCPP_INFO(node.get_logger(), "CCamera subscribed to camera/image_raw");

}

// Convert each received camera message and retain the image capture time.
void CCamera::image_callback(const sensor_msgs::msg::Image::SharedPtr msg)
{
  try {
    //convert the message 'camera/image_raw' to a bgr8
    mImage = cv_bridge::toCvShare(msg, "bgr8");

  } catch (const cv_bridge::Exception & e) {
    std::cerr << "cv_bridge failed: " << e.what() << std::endl;
    return;
  }

  //convert to greyscale to save some time 
  cv::cvtColor(mImage->image, mGreyscaleImage, cv::COLOR_BGR2GRAY);

  //get timestamp of current image in seconds
  mStampS = rclcpp::Time(msg->header.stamp).seconds();
}


// Return the most recently converted grayscale frame for detection.
cv::Mat CCamera::get_image()
{

    return mGreyscaleImage;

}

//returns the timestamp of current image in seconds.
double CCamera::get_stamp_s() const
{
    return mStampS;
}
