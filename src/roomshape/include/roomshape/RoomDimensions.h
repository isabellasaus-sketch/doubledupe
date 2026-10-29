// MTRX3760 2026 Project 2: Package Deal
// File: RoomDimensions.h
// Author(s): 520174067, 510146584
//
// Defines the shared data type used to represent measured room dimensions.

#ifndef ROOMSHAPE_ROOMDIMENSIONS_H
#define ROOMSHAPE_ROOMDIMENSIONS_H

namespace roomshape {

//---RoomDimensions-------------------------------------------------------------
// Shared result passed from the scan processor to the classifier.
// These are bounding-rectangle sides, not necessarily individual wall lengths.
struct RoomDimensions {
  double ShortSide{0.0};    // Smaller bounding side in metres.
  double LongSide{0.0};     // Larger bounding side in metres.
  double Orientation{0.0};  // Tested bounding-axis angle in scan frame, radians.
  bool Valid{false};        // False means no usable geometry estimate.
};

}  // namespace roomshape

#endif  // ROOMSHAPE_ROOMDIMENSIONS_H
