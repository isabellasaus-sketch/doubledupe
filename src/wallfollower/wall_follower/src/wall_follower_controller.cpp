// MTRX3760 2026 Project 2: Package Deal
// File: wall_follower_controller.cpp
// Author(s): Joe Egger, Max Entwistle, An Nguyen, Max Yip, Selina Ngyuen, Sarina Zaman
//
// Implements obstacle avoidance, outer-corner handling, and proportional
// right-wall tracking for the WallFollowerController.

#include "wall_follower/wall_follower_controller.hpp"

#include <cmath>

WallFollowerController::WallFollowerController(
  const wall_follower_params::Params &params)
: 
  linear_velocity_(params.linear_velocity),
  angular_velocity_(params.angular_velocity),
  front_obstacle_distance_(params.front_obstacle_distance),
  front_stop_distance_(params.front_stop_distance),
  wall_lost_distance_(params.wall_lost_distance),
  desired_distance_90_(params.desired_distance_90),
  desired_distance_45_(params.desired_distance_45),
  wall_distance_tolerance_(params.wall_distance_tolerance),
  turn_speed_factor_(params.turn_speed_factor),
  outer_corner_angular_velocity_(params.outer_corner_angular_velocity),
  outer_corner_linear_velocity_(params.outer_corner_linear_velocity)
{
}

WallFollowerController::~WallFollowerController()
{
}

VelocityCommand WallFollowerController::AvoidFrontObstacle(double dist_front) const
{
  if (dist_front <= front_stop_distance_)
  {
    return VelocityCommand{0.0, angular_velocity_};
  }

  return VelocityCommand{linear_velocity_ * turn_speed_factor_, angular_velocity_};
}

bool WallFollowerController::IsOuterCorner(double dist_front_right, double dist_right) const
{
  // Both readings must indicate open space; the diagonal threshold accounts for 45-degree geometry.
  return ((dist_right > wall_lost_distance_) &&
          (dist_front_right > wall_lost_distance_ * sqrt(2 * (pow(desired_distance_90_, 2)))));
}

VelocityCommand WallFollowerController::RoundOuterCorner() const
{
  return VelocityCommand{outer_corner_linear_velocity_, outer_corner_angular_velocity_};
}

VelocityCommand WallFollowerController::TrackWall(double dist_front_right, double dist_right) const
{
  // Full bang-bang turning command.
  const double angular_velocity = angular_velocity_;

  const bool ideal_90 = dist_right >= desired_distance_90_ - wall_distance_tolerance_ &&
                        dist_right <= desired_distance_90_ + wall_distance_tolerance_;
  const bool ideal_45 = dist_front_right >= desired_distance_45_ - wall_distance_tolerance_ &&
                        dist_front_right <= desired_distance_45_ + wall_distance_tolerance_;

  // Both sensors are within the desired range.
  // Drive straight.
  if (ideal_90 && ideal_45)
  {
    return VelocityCommand{linear_velocity_, 0.0};
  }

  // Either sensor is too close.
  // Turn left.
  if (dist_right < desired_distance_90_ - wall_distance_tolerance_ ||
      dist_front_right < desired_distance_45_ - wall_distance_tolerance_)
  {
    return VelocityCommand{linear_velocity_ * turn_speed_factor_, angular_velocity};
  }

  // Either sensor is too far.
  // Turn right.
  if (dist_right > desired_distance_90_ + wall_distance_tolerance_ ||
      dist_front_right > desired_distance_45_ + wall_distance_tolerance_)
  {
    return VelocityCommand{linear_velocity_ * turn_speed_factor_, -angular_velocity};
  }

  // Fallback.
  return VelocityCommand{linear_velocity_, 0.0};
}

VelocityCommand WallFollowerController::CalculateCommand(
    double distance_front,
    double distance_front_right,
    double distance_right) const
{
  // Prioritize front obstacle avoidance, then outer-corner recovery, then normal wall tracking.
  if (distance_front < front_obstacle_distance_)
  {
    return AvoidFrontObstacle(distance_front);
  }

  // 2. Corner ended? Round the corner
  if (IsOuterCorner(distance_front_right, distance_right))
  {
    return RoundOuterCorner();
  }

  // 3. Normal straight wall tracking
  return TrackWall(distance_front_right, distance_right);
}