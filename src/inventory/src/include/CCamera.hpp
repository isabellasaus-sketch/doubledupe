// MTRX3760 2026 Project 2: Package Deal
// File: CCamera.hpp
// Author(s): Christina Miljak, Davin Pranata, Joshua Samuel David, Joshua Kwok, Kaif Masiudddin Mohammed, Kieran Wong
//
// Declares the image subscriber, grayscale frame storage and timestamp access.

#ifndef CCAMERA_HPP_
#define CCAMERA_HPP_
#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/image.hpp>
#include <opencv2/aruco.hpp>
#include <opencv2/core.hpp>
#include <cv_bridge/cv_bridge.hpp>
#include <iostream>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>


// Converts subscribed ROS image messages into frames suitable for detection.
class CCamera {
    public:
        // gets passed the node because CCamera needs to subscribe to camera/image_raw
        CCamera(rclcpp::Node & node);

        //converts the msg from the subscription to a cv::Mat and stores it as a member of CCamera (compressed)
        void image_callback(const sensor_msgs::msg::Image::SharedPtr msg);

        //returns the compressed image as a cv::Mat so that CArucoReader can detect the codes.
        cv::Mat get_image();

        //returns the timestamp of the current image in seconds as a double
        double get_stamp_s() const;


    private:

    // Camera frames arrive through the sensor-data QoS subscription.
    rclcpp::Subscription<sensor_msgs::msg::Image>::SharedPtr image_sub_;

    cv_bridge::CvImageConstPtr mImage; //current frame

    cv::Mat mGreyscaleImage; //current frame

    double mStampS = -1.0; //header stamp of mGreyscaleImage, in seconds

};

#endif // CCAMERA_HPP_
