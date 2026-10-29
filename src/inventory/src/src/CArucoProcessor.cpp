// MTRX3760 2026 Project 2: Package Deal
// File: CArucoProcessor.cpp
// Author(s): Christina Miljak, Davin Pranata, Joshua Samuel David, Joshua Kwok, Kaif Masiudddin Mohammed, Kieran Wong, Thea Macarthur-Lassen
//
// Connects camera input, marker detection, encounter tracking and inventory output.

#include "CArucoProcessor.hpp"
using namespace std::chrono_literals;

// Initialise the node and its components, then schedule frame processing.
CArucoProcessor::CArucoProcessor()
    :Node("ArucoProcessor"),
    mInventory(*this),
    mCamera(*this),
    mLastStampS(kNoStamp),
    mEncounter(
      declare_parameter<double>("absence_timeout_s", kDefaultAbsenceTimeoutS), 
      declare_parameter<int>("min_consecutive_frames", kDefaultMinConsecutiveFrames)),
    mCurrentVector()

{
  //set up a timer to call update_callback() every 10 ms
  update_timer_ = this->create_wall_timer(10ms,  std::bind(&CArucoProcessor::update_callback, this));

  RCLCPP_INFO(
    this->get_logger(),
    "CArucoProcessor node has been initialised");

}

// Ignore absent or repeated frames, then report newly confirmed encounters.
void CArucoProcessor::update_callback()
{

  //get the current image from the camera
  const cv::Mat currentImage = mCamera.get_image();

  //if nothing is publishing camera/image_raw or theres no image yet, then return
  if (currentImage.empty()) {
    return;   // no frame received yet, try again on the next tick
  }

  //the timer ticks faster than the camera publishes, so only process each frame once
  const double stampS = mCamera.get_stamp_s();
  if (stampS == mLastStampS) {
    return;
  }
  mLastStampS = stampS;

  show_image(currentImage); //draw the image in a window to make a video.

  //detect the aruco markers from the current frame and return as vector
  mCurrentVector = mArucoReader.detect_id(currentImage);

  //report any new encounters, timed by when the frame was captured.
  mInventory.add_encounters(mEncounter.update(mCurrentVector, stampS));

}

void CArucoProcessor::show_image(const cv::Mat & image)
{
  // add a try block just in case your device doesn't have a screen.
  try {
    cv::imshow("camera image", image);
    cv::waitKey(1);   // required, or the window never draws
  } catch (const cv::Exception & error) {
    RCLCPP_ERROR(get_logger(), "Cannot open a window (%s); disabling it", error.what());
  }
}
