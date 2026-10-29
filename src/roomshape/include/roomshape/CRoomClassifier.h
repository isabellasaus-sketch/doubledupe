// MTRX3760 2026 Project 2: Package Deal
// File: CRoomClassifier.h
// Author(s): 490550744, 530141790
//
// Declares the room classifier and the supported room definition data.

#ifndef ROOMSHAPE_CROOMCLASSIFIER_H
#define ROOMSHAPE_CROOMCLASSIFIER_H

//---Includes-------------------------------------------------------------------
#include <string>
#include <vector>

#include "roomshape/RoomDimensions.h"

namespace roomshape {

//---CRoomClassifier------------------------------------------------------------
// Matches sorted bounding sides against the five supported room definitions.
// This class has no ROS dependencies; it does not count walls or detect corners.
class CRoomClassifier {
 public:
  // The tolerance applies separately to each side, in metres.
  explicit CRoomClassifier(double aMatchTolerance = 0.18);

  // Returns the lowest-error label within tolerance. Invalid or unmatched
  // dimensions return an empty string; stability filtering belongs to the node.
  std::string Classify(const RoomDimensions& arDimensions) const;

 private:
  // Internal reference dimensions used only by this classifier.
  struct RoomDefinition {
    std::string Name;
    double ShortSide{0.0};
    double LongSide{0.0};
  };

  // CalculateError sums the absolute errors of the two bounding sides.
  double CalculateError(const RoomDimensions& arDimensions,
      const RoomDefinition& arDefinition) const;

  // Classification settings and reference dimensions (metres).
  double mMatchTolerance;
  std::vector<RoomDefinition> mRoomDefinitions;
};

}  // namespace roomshape

#endif  // ROOMSHAPE_CROOMCLASSIFIER_H
