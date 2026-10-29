// MTRX3760 2026 Project 2: Package Deal
// File: CArucoReader.hpp
// Author(s): Christina Miljak, Davin Pranata, Joshua Samuel David, Joshua Kwok, Kaif Masiudddin Mohammed, Kieran Wong
//
// Declares the OpenCV detector for the DICT_4X4_50 ArUco dictionary.

#ifndef CARUCO_READER_HPP_
#define CARUCO_READER_HPP_

#include <string>
#include <vector>

#include <opencv2/aruco.hpp>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>


// Wraps marker detection so the ROS processor does not depend on OpenCV details.
class CArucoReader {
public:
    // Initialise the required 4x4 dictionary and detection parameters.
    CArucoReader();

    // IDs of all markers in the image. Empty if none are found or the image is empty.
    std::vector<int> detect_id(const cv::Mat & input_image) const;

    // IDs of all markers in the image file. Empty if none are found or the file can't be read.
    std::vector<int> detect_file_id(const std::string & file_path) const;

private:
    // Dictionary of valid markers (DICT_4X4_50).
    cv::Ptr<cv::aruco::Dictionary> m_dictionary;

    // Detector tuning parameters.
    cv::Ptr<cv::aruco::DetectorParameters> m_parameters;

};

#endif // CARUCO_READER_HPP_
