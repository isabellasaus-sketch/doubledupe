// MTRX3760 2026 Project 2: Package Deal
// File: CTestMaker.cpp
// Author(s): Christina Miljak, Davin Pranata, Joshua Samuel David, Joshua Kwok, Kaif Masiudddin Mohammed, Kieran Wong
//
// Generates an ArUco tag PNG and provides a standalone marker utility main.

#include "CTestMaker.hpp"


// Initialise the required ArUco dictionary and select the marker ID.
CTestMaker:: CTestMaker()
    :mDictionary(cv::aruco::getPredefinedDictionary(cv::aruco::DICT_4X4_50)),
    mTagNum(6)

{
    return;
}


// Render the marker with a white border and write it as a PNG file.
void CTestMaker::make_aruco()
{
  // Image to hold the marker
  cv::Mat markerImage;

  // draw marker number mTagNum (0 to 49 in DICT_4X4_50)
  cv::aruco::drawMarker(mDictionary, mTagNum, 200, markerImage, 1);

  // add a white border to the marker image for contrast and reliability of detection.
  cv::copyMakeBorder(markerImage, markerImage, 50, 50, 50, 50, cv::BORDER_CONSTANT, cv::Scalar(255, 255, 255));

  // save the marker as markers/marker<mTagNum>.png
  const std::string filename = "markers/marker" + std::to_string(mTagNum) + ".png";
  if (!cv::imwrite(filename, markerImage))
  {
    std::cerr << "Failed to write " << filename << std::endl;
  }
}


// Write the marker PNG, then exit. No ROS node is started.
int main(void)
{

    CTestMaker my_test;

    my_test.make_aruco();

  return 0;
}
