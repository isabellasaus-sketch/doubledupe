// MTRX3760 2026 Project 2: Package Deal
// File: CRoomClassifier.cpp
// Author(s): 490550744, 530141790
//
// Implements dimension-based classification of the supported room types.

//---Includes-------------------------------------------------------------------
#include "roomshape/CRoomClassifier.h"

#include <cmath>
#include <limits>
#include <string>

namespace roomshape {

//---CRoomClassifier------------------------------------------------------------
// Reference sizes describe the interior bounding rectangle, in metres.
// A one-metre equilateral triangle has height sqrt(3)/2 and base length 1.
CRoomClassifier::CRoomClassifier(double aMatchTolerance)
    : mMatchTolerance(aMatchTolerance),
      mRoomDefinitions{{"SMALL_SQUARE", 0.60, 0.60},
          {"LARGE_SQUARE", 0.90, 0.90},
          {"RECTANGLE", 0.50, 0.90},
          {"HALLWAY", 0.45, 1.20},
          {"EQUILATERAL_TRIANGLE", 0.693, 0.80}} {}

//------------------------------------------------------------------------------
std::string CRoomClassifier::Classify(
    const RoomDimensions& arDimensions) const {
  // A failed scan estimate must not be compared as a zero-sized room.
  if (!arDimensions.Valid || arDimensions.ShortSide <= 0.0) {
    return "";
  }

  const double Ratio = arDimensions.LongSide / arDimensions.ShortSide;

  std::string BestRoomName;
  double SmallestError = std::numeric_limits<double>::infinity();

  for (const RoomDefinition& Definition : mRoomDefinitions) {
      const double RatioError =
        std::abs(Ratio - Definition.LongSide / Definition.ShortSide);

    // Ratio is the gate; mMatchTolerance is now a ratio tolerance.
    if (RatioError <= mMatchTolerance) {
      const double TotalError = CalculateError(arDimensions, Definition);

      // Rank only eligible rooms; equal errors keep the earlier definition.
      if (TotalError < SmallestError) {
        SmallestError = TotalError;
        BestRoomName = Definition.Name;
      }
    }
  }

  // The name remains empty if no reference passed both side tolerances.
  return BestRoomName;
}

//------------------------------------------------------------------------------
double CRoomClassifier::CalculateError(
    const RoomDimensions& arDimensions,
    const RoomDefinition& arDefinition) const {
  const double RatioError =
      std::abs(arDimensions.LongSide / arDimensions.ShortSide -
            arDefinition.LongSide / arDefinition.ShortSide);

  // Small weight: size only separates rooms with near-identical ratios.
  constexpr double kSizeWeight = 0.1;
  const double SizeError =
      std::abs(arDimensions.LongSide - arDefinition.LongSide);

  return RatioError + kSizeWeight * SizeError;
}

}  // namespace roomshape
