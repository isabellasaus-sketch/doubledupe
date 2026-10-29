// MTRX3760 2026 Project 2: Package Deal
// File: CArucoProcessor.hpp
// Author(s): Christina Miljak, Davin Pranata, Joshua Samuel David, Joshua Kwok, Kaif Masiudddin Mohammed, Kieran Wong
//
// Declares the inventory ROS node that links camera frames, ArUco detection,
// encounter tracking and publishing.

#ifndef CARUCO_PROCESSOR_HPP_
#define CARUCO_PROCESSOR_HPP_


#include <iostream>
#include <vector>

#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/image.hpp>
#include <opencv2/opencv.hpp>

#include "CEncounter.hpp"
#include "CArucoReader.hpp"
#include "CCamera.hpp"
#include "CInventory.hpp"

// Owns all processing components within a single ROS node.
class CArucoProcessor : public rclcpp::Node
{
    public:

        // Initialise the camera, detector, encounter state and output publishers.
        CArucoProcessor();

        // Check for a new image and update the inventory once per camera frame.
        void update_callback();

        //Displays each frame like a video in a popup window
        void show_image(const cv::Mat & image);

    private:
        rclcpp::TimerBase::SharedPtr update_timer_;  // Run sensor readiness checks every 10 ms.

        // Publishes both new encounters and the complete ordered encounter log.
        CInventory mInventory;

        // Receives camera/image_raw and stores the latest grayscale frame.
        CCamera mCamera;

        // Detects DICT_4X4_50 marker IDs in the camera frame.
        CArucoReader mArucoReader;

        // Defaults for the ROS parameters that configure mEncounter.
        const double kDefaultAbsenceTimeoutS = 1.0;  // Out of view this long ends an encounter.
        const int kDefaultMinConsecutiveFrames = 1;  // Frames in a row needed to confirm a tag.

        const double kNoStamp = -1.0;  // Stamp value meaning no frame has been processed yet.

        double mLastStampS;  // Stamp of the last frame processed, so the same frame is not counted twice.

        // Separates repeated observations from new passes over the same tag.
        CEncounter mEncounter;

        // IDs detected in the image currently being processed.
        std::vector<int> mCurrentVector;

};

#endif // CARUCO_PROCESSOR_HPP_
