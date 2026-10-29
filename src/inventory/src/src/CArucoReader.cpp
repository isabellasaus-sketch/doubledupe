// MTRX3760 2026 Project 2: Package Deal
// File: CArucoReader.cpp
// Author(s): Christina Miljak, Davin Pranata, Joshua Samuel David, Joshua Kwok, Kaif Masiudddin Mohammed, Kieran Wong
//
// Uses OpenCV to detect DICT_4X4_50 tags in images and image files.

#include "CArucoReader.hpp"



// CArucoReader CTor builds dict and params for marker detection.
// Dictionary is hardcoded still
CArucoReader::CArucoReader()
    : m_dictionary(cv::aruco::getPredefinedDictionary(cv::aruco::DICT_4X4_50)),
      m_parameters(cv::aruco::DetectorParameters::create()) //default detection parameters
{
}


// Detect all marker thru input image and return ids.
// If no marker is found or image is empty, return empty vector.
std::vector<int> CArucoReader::detect_id(const cv::Mat & input_image) const
{
    std::vector<int> marker_ids;
    // No pixels means no possible detections; avoid calling OpenCV.
    if (input_image.empty()) {
        return marker_ids;
    }
    std::vector<std::vector<cv::Point2f>> marker_corners; // Unused, but required by detectMarkers
    cv::aruco::detectMarkers(input_image, m_dictionary, marker_corners, marker_ids, m_parameters);
    return marker_ids;
}

// Detect all marker thru image file and return ids.
std::vector<int> CArucoReader::detect_file_id(const std::string & file_path) const
{
    return detect_id(cv::imread(file_path, cv::IMREAD_COLOR));
}
