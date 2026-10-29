// MTRX3760 2026 Project 2: Package Deal
// File: CRoomScanProcessor.cpp
// Author(s): 530493932, 500474251
//
// Implements LaserScan filtering, point conversion and geometry estimation.

//---Includes-------------------------------------------------------------------
#include "roomshape/CRoomScanProcessor.h"

#include <algorithm>
#include <cmath>
#include <limits>

namespace roomshape {

//---CRoomScanProcessor---------------------------------------------------------
CRoomScanProcessor::CRoomScanProcessor(std::size_t aMinimumValidPoints)
    : mMinimumValidPoints(aMinimumValidPoints) {}

//------------------------------------------------------------------------------
RoomDimensions CRoomScanProcessor::ProcessScan(
    const sensor_msgs::msg::LaserScan& arScan) const {
  const std::vector<Point2D> Points = ConvertScanToPoints(arScan);

  // Sparse scans should not produce apparently precise room dimensions.
  if (Points.size() < mMinimumValidPoints) {
    return RoomDimensions{};
  }

  return FindMinimumBoundingRectangle(Points);
}

//------------------------------------------------------------------------------
std::vector<CRoomScanProcessor::Point2D>
CRoomScanProcessor::ConvertScanToPoints(
    const sensor_msgs::msg::LaserScan& arScan) const {
  std::vector<Point2D> Points;
  Points.reserve(arScan.ranges.size());

  for (std::size_t Index = 0; Index < arScan.ranges.size(); ++Index) {
    const double Range = static_cast<double>(arScan.ranges[Index]);

    // Ignore invalid, infinite and out-of-range measurements.
    if (!std::isfinite(Range) ||
        Range < static_cast<double>(arScan.range_min) ||
        Range > static_cast<double>(arScan.range_max)) {
      continue;
    }

    // Preserve the original beam index after filtering: each return has its
    // own angle relative to the LaserScan frame, not the world frame.
    const double Angle = static_cast<double>(arScan.angle_min) +
        static_cast<double>(Index) *
        static_cast<double>(arScan.angle_increment);

    // Ignore beams pointing back through the opening the robot came in by.
    constexpr double kRearIgnoreHalfAngle = 30.0 * kPi / 180.0;  // tune
    const double Wrapped = std::atan2(std::sin(Angle), std::cos(Angle));
    if (std::abs(Wrapped) > kPi - kRearIgnoreHalfAngle) {
      continue;
    }

    Point2D Point;
    Point.X = Range * std::cos(Angle);
    Point.Y = Range * std::sin(Angle);
    Points.push_back(Point);
  }

  return Points;
}



//------------------------------------------------------------------------------
RoomDimensions CRoomScanProcessor::FindMinimumBoundingRectangle(
    const std::vector<Point2D>& arPoints) const {
  RoomDimensions BestDimensions;
  double SmallestArea = std::numeric_limits<double>::infinity();

  // A rectangle repeats after 90 degrees with its sides exchanged.
  // Search that interval at one-degree resolution rather than assuming the
  // robot is aligned with a wall. This is an approximate minimum-area search.
  for (int Degree = 0; Degree < kNumberOfTestAngles; ++Degree) {
    const double Angle = static_cast<double>(Degree) * kPi / 180.0;
    const double Cosine = std::cos(Angle);
    const double Sine = std::sin(Angle);

    double MinimumX = std::numeric_limits<double>::infinity();
    double MaximumX = -std::numeric_limits<double>::infinity();
    double MinimumY = std::numeric_limits<double>::infinity();
    double MaximumY = -std::numeric_limits<double>::infinity();

    for (const Point2D& Point : arPoints) {
      // Project onto the trial axes by rotating coordinates through -Angle.
      const double RotatedX = Cosine * Point.X + Sine * Point.Y;
      const double RotatedY = -Sine * Point.X + Cosine * Point.Y;

      MinimumX = std::min(MinimumX, RotatedX);
      MaximumX = std::max(MaximumX, RotatedX);
      MinimumY = std::min(MinimumY, RotatedY);
      MaximumY = std::max(MaximumY, RotatedY);
    }

    const double Width = MaximumX - MinimumX;
    const double Height = MaximumY - MinimumY;

    // Ignore degenerate rectangles rather than marking them as valid rooms.
    if (Width <= 0.0 || Height <= 0.0) {
      continue;
    }

    const double Area = Width * Height;

    if (Area < SmallestArea) {
      SmallestArea = Area;
      // Sorting removes the width/height ambiguity for the classifier.
      BestDimensions.ShortSide = std::min(Width, Height);
      BestDimensions.LongSide = std::max(Width, Height);
      BestDimensions.Orientation = Angle;
      BestDimensions.Valid = true;
    }
  }

  return BestDimensions;
}

}  // namespace roomshape