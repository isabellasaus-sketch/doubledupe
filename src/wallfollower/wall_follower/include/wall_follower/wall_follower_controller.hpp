// MTRX3760 2026 Project 2: Package Deal
// File: wall_follower_controller.hpp
// Author(s): Joe Egger, Max Entwistle, An Nguyen, Max Yip, Selina Ngyuen, Sarina Zaman
//
// Declares the wall-following controller and the velocity command it computes
// from front, front-right, and right lidar distances.

#ifndef WALL_FOLLOWER__WALL_FOLLOWER_CONTROLLER_HPP_
#define WALL_FOLLOWER__WALL_FOLLOWER_CONTROLLER_HPP_

#include <wall_follower/wall_follower_params.hpp>

struct VelocityCommand
{
    // Forward linear velocity in meters per second.
  double linear;
    // Rotational velocity around the vertical axis in radians per second.
  double angular;
};

class WallFollowerController
{
    public:
        // Creates a controller with parameters from config file.
        WallFollowerController(
            const wall_follower_params::Params &params = wall_follower_params::Params());
        // Releases the controller.
        ~WallFollowerController();

        // Chooses a velocity command from the three measured lidar distances, in meters.
        VelocityCommand CalculateCommand(
            double distance_front,
            double distance_front_right,
            double distance_right) const;

    private:
        double linear_velocity_;
        double angular_velocity_;
        double front_obstacle_distance_;
        double front_stop_distance_;
        double wall_lost_distance_;
        double desired_distance_90_;
        double desired_distance_45_;
        double wall_distance_tolerance_;
        double turn_speed_factor_;
        double outer_corner_angular_velocity_;
        double outer_corner_linear_velocity_;

        // Turns away from a front obstacle and slows or stops as it gets closer.
        VelocityCommand AvoidFrontObstacle(double dist_front) const;
        // Detects a wall ending from the right and front-right distances.
        bool IsOuterCorner(double dist_front_right, double dist_right) const;
        // Returns the fixed velocity command used to round an outer corner.
        VelocityCommand RoundOuterCorner() const;
        // Computes a wall-tracking command from right-wall distance and orientation errors.
        VelocityCommand TrackWall(double dist_front_right, double dist_right) const;
};

#endif  // WALL_FOLLOWER__WALL_FOLLOWER_CONTROLLER_HPP_