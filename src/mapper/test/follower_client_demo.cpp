// MTRX3760 2026 Project 2: Package Deal
// File: follower_client_demo.cpp
// Author(s): Thea Macarthur-Lassen
//
// Scratch tool for testing WallFollowerClient against a running wall follower:
// logs its state every second, starts it after 5 s and stops it after 25 s.
// Not part of the final system; keep it out of the merge to main.
#include <chrono>
#include <functional>
#include <memory>
#include <rclcpp/rclcpp.hpp>
#include "mapper/WallFollowerClient.hpp"

class FollowerClientDemo : public rclcpp::Node
{
public:
    FollowerClientDemo()
        : Node("follower_client_demo"), client_(*this), ticks_(0), timer_()
    {
        timer_ = create_wall_timer(
            std::chrono::seconds(1),
            std::bind(&FollowerClientDemo::onTick, this));
    }

private:
    static const int StartTick;
    static const int StopTick;

    void onTick()
    {
        ++ticks_;
        RCLCPP_INFO(get_logger(), "t=%d follower=%s scan_valid=%s", ticks_,
                    client_.followerStateName().c_str(),
                    client_.isScanValid() ? "yes" : "no");
        if(ticks_ == StartTick && client_.isScanValid())
        {
            client_.start();
        }
        if(ticks_ == StopTick)
        {
            client_.stop();
        }
    }

    WallFollowerClient client_;
    int ticks_;
    rclcpp::TimerBase::SharedPtr timer_;
};

const int FollowerClientDemo::StartTick = 5;
const int FollowerClientDemo::StopTick = 25;

int main(int argc, char** argv)
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<FollowerClientDemo>());
    rclcpp::shutdown();
    return 0;
}