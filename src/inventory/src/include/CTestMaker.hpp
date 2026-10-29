// MTRX3760 2026 Project 2: Package Deal
// File: CTestMaker.hpp
// Author(s): Christina Miljak, Davin Pranata, Joshua Samuel David, Joshua Kwok, Kaif Masiudddin Mohammed, Kieran Wong
//
// Declares the utility that creates printable ArUco marker images.

#ifndef CTEST_MAKER_HPP_
#define CTEST_MAKER_HPP_

#include <opencv2/aruco.hpp>
#include <opencv2/core.hpp>
#include <cv_bridge/cv_bridge.hpp>
#include <iostream>
#include <opencv2/imgcodecs.hpp>


// Provides a small utility for generating a fixed-ID ArUco tile image.
class CTestMaker {
    public:

        CTestMaker();

        // Draw the configured marker and save it to the markers directory.
        void make_aruco();

    private:

        //the arcuo marker dictionary which we are using: 5 x 5 squares and 50 of them.
        cv::Ptr<cv::aruco::Dictionary> mDictionary;

        //the id number of the tag that you want to make
        int mTagNum;
};

#endif // CTEST_MAKER_HPP_
