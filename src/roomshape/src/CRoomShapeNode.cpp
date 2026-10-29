// MTRX3760 2026 Project 2: Package Deal
// File: CRoomShapeNode.cpp
// Author(s): 520174067, 510146584
//
// Implements the RoomShape ROS 2 node and its executable entry point.

//---Includes-------------------------------------------------------------------
#include "roomshape/CRoomShapeNode.h"

#include <cstddef>
#include <functional>
#include <memory>
#include <string>

namespace roomshape {

//---CRoomShapeNode-------------------------------------------------------------
CRoomShapeNode::CRoomShapeNode()
    : Node("roomshape_node"),
      mMatchTolerance(0.12),
      mRequiredStableScans(3),
      mMinimumValidPoints(80),
      mCandidateRoomType(""),
      mCandidateCount(0),
      mLastPublishedRoomType("") {
  // Parameters are read once at startup; workers use these validated values.
  mMatchTolerance = declare_parameter<double>("match_tolerance", 0.18);

  mRequiredStableScans = declare_parameter<int>("required_stable_scans", 3);

  mMinimumValidPoints = declare_parameter<int>("minimum_valid_points", 80);

  // Reject nonpositive settings before constructing the processing workers.
  if (mMatchTolerance <= 0.0) {
    RCLCPP_WARN(get_logger(), "Invalid match_tolerance. Using 0.12 metres.");
    mMatchTolerance = 0.12;
  }

  if (mRequiredStableScans < 1) {
    RCLCPP_WARN(get_logger(), "Invalid required_stable_scans. Using 3.");
    mRequiredStableScans = 3;
  }

  if (mMinimumValidPoints < 1) {
    RCLCPP_WARN(get_logger(), "Invalid minimum_valid_points. Using 80.");
    mMinimumValidPoints = 80;
  }

  mScanProcessor = std::make_unique<CRoomScanProcessor>(
      static_cast<std::size_t>(mMinimumValidPoints));

  mRoomClassifier = std::make_unique<CRoomClassifier>(mMatchTolerance);

  // Retain the latest stable label for compatible late-joining subscribers.
  // This is an event-style output, not a new message for every scan.
  rclcpp::QoS OutputQoS(rclcpp::KeepLast(1));
  OutputQoS.reliable();
  OutputQoS.transient_local();

  mRoomPublisher =
      create_publisher<std_msgs::msg::String>("room_shape/type", OutputQoS);

  // SensorDataQoS accepts best-effort scan publishers, including the simulator.
  mScanSubscription = create_subscription<sensor_msgs::msg::LaserScan>(
      "scan", rclcpp::SensorDataQoS(),
      std::bind(&CRoomShapeNode::ScanCallback, this, std::placeholders::_1));

  RCLCPP_INFO(get_logger(), "RoomShape node has started.");
  RCLCPP_INFO(get_logger(), "Waiting for LaserScan data on topic: scan");
}

//------------------------------------------------------------------------------
void CRoomShapeNode::ScanCallback(
    const sensor_msgs::msg::LaserScan::SharedPtr aScan) {
  // Keep geometry, label selection and temporal stability as separate steps.
  const RoomDimensions Dimensions = mScanProcessor->ProcessScan(*aScan);

  const std::string RoomType = mRoomClassifier->Classify(Dimensions);

  if (Dimensions.Valid) {
    RCLCPP_DEBUG(get_logger(), "Measured room: short=%.3f m, long=%.3f m",
        Dimensions.ShortSide, Dimensions.LongSide);
  }

  UpdateStableResult(RoomType);
}

//------------------------------------------------------------------------------
void CRoomShapeNode::UpdateStableResult(const std::string& arRoomType) {
  // An empty result means the scan did not match a supported room.
  // Reset only the candidate; the previously published label is retained.
  if (arRoomType.empty()) {
    mCandidateRoomType.clear();
    mCandidateCount = 0;
    return;
  }

  // Only consecutive matches count; a different label starts a new sequence.
  if (arRoomType == mCandidateRoomType) {
    ++mCandidateCount;
  } else {
    mCandidateRoomType = arRoomType;
    mCandidateCount = 1;
  }

  if (mCandidateCount < mRequiredStableScans) {
    return;
  }

  // Suppress duplicates even after the stability threshold has been reached.
  if (mCandidateRoomType == mLastPublishedRoomType) {
    return;
  }

  std_msgs::msg::String Message;
  Message.data = mCandidateRoomType;

  mRoomPublisher->publish(Message);
  mLastPublishedRoomType = mCandidateRoomType;

  RCLCPP_INFO(get_logger(), "Stable room type: %s", mCandidateRoomType.c_str());
}

}  // namespace roomshape
