// MTRX3760 2026 Project 2: Package Deal
// File: CRoomScanProcessor.h
// Author(s): 530493932, 500474251
//
// Declares conversion of LaserScan data into estimated room dimensions.

#ifndef ROOMSHAPE_CROOMSCANPROCESSOR_H
#define ROOMSHAPE_CROOMSCANPROCESSOR_H

//---Includes-------------------------------------------------------------------
#include <cstddef>
#include <sensor_msgs/msg/laser_scan.hpp>
#include <vector>

#include "roomshape/RoomDimensions.h"

namespace roomshape {

//---CRoomScanProcessor---------------------------------------------------------
// Filters one LaserScan and estimates its minimum-area bounding rectangle.
// Processing stays in the scan frame and does not require odometry or a map.
class CRoomScanProcessor {
 public:
  // Require enough usable returns before attempting a geometry estimate.
  explicit CRoomScanProcessor(std::size_t aMinimumValidPoints = 80);

  // Returns sorted sides in metres. Too few valid points or no nonzero-area
  // rectangle leaves Valid false; no observations are retained between scans.
  RoomDimensions ProcessScan(const sensor_msgs::msg::LaserScan& arScan) const;

 private:
  // Internal Cartesian scan point; callers receive only RoomDimensions.
  struct Point2D {
    double X{0.0};  // Scan-frame X coordinate in metres.
    double Y{0.0};  // Scan-frame Y coordinate in metres.
  };

  // ConvertScanToPoints filters invalid returns and converts polar coordinates.
  std::vector<Point2D> ConvertScanToPoints(
      const sensor_msgs::msg::LaserScan& arScan) const;

  // Searches 0..89 degrees in one-degree steps, so orientation is approximate.
  RoomDimensions FindMinimumBoundingRectangle(
      const std::vector<Point2D>& arPoints) const;

  // Minimum number of returns remaining after range filtering.
  std::size_t mMinimumValidPoints;

  // Geometry search constants.
  static constexpr double kPi = 3.14159265358979323846;
  static constexpr int kNumberOfTestAngles = 90;
};

}  // namespace roomshape

#endif  // ROOMSHAPE_CROOMSCANPROCESSOR_H
