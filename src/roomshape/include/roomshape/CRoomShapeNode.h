// MTRX3760 2026 Project 2: Package Deal
// File: CRoomShapeNode.h
// Author(s): 520174067, 510146584
//
// Declares the ROS 2 node that coordinates scan processing and classification.

#ifndef ROOMSHAPE_CROOMSHAPENODE_H
#define ROOMSHAPE_CROOMSHAPENODE_H

//---Includes-------------------------------------------------------------------
#include <memory>
#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/laser_scan.hpp>
#include <std_msgs/msg/string.hpp>
#include <string>

#include "roomshape/CRoomClassifier.h"
#include "roomshape/CRoomScanProcessor.h"

namespace roomshape {

//---CRoomShapeNode-------------------------------------------------------------
// Connects scan processing and classification to the ROS 2 topics.
// Publishes only changed labels supported by consecutive matching scans.
class CRoomShapeNode : public rclcpp::Node {
 public:
  // Read startup parameters, construct the workers and create ROS endpoints.
  CRoomShapeNode();

 private:
  // Receives and processes one LiDAR scan.
  void ScanCallback(const sensor_msgs::msg::LaserScan::SharedPtr aScan);

  // Reset on an unknown result; otherwise publish after the stability threshold.
  // An unknown scan does not retract the previously published stable label.
  void UpdateStableResult(const std::string& arRoomType);

  // Startup configuration for classification and result stability.
  double mMatchTolerance;      // Maximum error per bounding side, in metres.
  int mRequiredStableScans;    // Consecutive matching labels needed to publish.
  int mMinimumValidPoints;     // Usable LiDAR returns needed per scan.

  // Owned scan-processing and classification components.
  std::unique_ptr<CRoomScanProcessor> mScanProcessor;
  std::unique_ptr<CRoomClassifier> mRoomClassifier;

  // ROS communication handles.
  rclcpp::Subscription<sensor_msgs::msg::LaserScan>::SharedPtr
      mScanSubscription;

  rclcpp::Publisher<std_msgs::msg::String>::SharedPtr mRoomPublisher;

  // Candidate tracking and the most recently published stable label.
  std::string mCandidateRoomType;
  int mCandidateCount;  // Consecutive scans matching mCandidateRoomType.
  std::string mLastPublishedRoomType;
};

}  // namespace roomshape

#endif  // ROOMSHAPE_CROOMSHAPENODE_H
