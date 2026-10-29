// MTRX3760 2026 Project 2: Package Deal
// File: WallFollowerClient.hpp
// Author(s): Thea Macarthur-Lassen
//
// The mapper's only view of the adopted wall follower: starts and stops it
// through its services and reports its state topic. Every service call is
// asynchronous, so none of them can block the node's executor. A stop is
// repeated until the follower reports PAUSED, because a robot that keeps
// driving after the run is over is the worst failure this class can have.
#ifndef MAPPER_WALL_FOLLOWER_CLIENT_HPP
#define MAPPER_WALL_FOLLOWER_CLIENT_HPP

#include <chrono>
#include <cstdint>
#include <string>
#include <rclcpp/rclcpp.hpp>
#include <std_srvs/srv/set_bool.hpp>
#include <std_srvs/srv/trigger.hpp>
#include <wall_follower/msg/wall_follower_state.hpp>

class WallFollowerClient
{
public:
    // Creates the service clients, the state subscription and the (idle) stop
    // retry timer on the given node. The node must outlive this object and
    // must not be moved.
    explicit WallFollowerClient(rclcpp::Node& node);

    // Asks the follower to follow the left wall, then to start. The second call
    // is sent from the first one's reply, so the order is guaranteed. A later
    // stop() cancels a start that is still in flight.
    void start();
    // Asks the follower to stop and keeps asking until it reports PAUSED (or
    // the attempts run out, which is logged as an error).
    void stop();

    // True once all three follower services are reachable, so start() can be sent.
    bool isAvailable() const;
    // True only if a state message arrived recently AND it reports a valid scan.
    bool isScanValid() const;
    // PAUSED, FOLLOWING, WAITING_FOR_SCAN, STATE STALE or NO STATE RECEIVED.
    std::string followerStateName() const;

private:
    typedef std_srvs::srv::SetBool SetBool;
    typedef std_srvs::srv::Trigger Trigger;
    typedef wall_follower::msg::WallFollowerState FollowerState;

    // Callbacks capture this object's address, so copying would leave dangling ones.
    WallFollowerClient(const WallFollowerClient&) = delete;
    WallFollowerClient& operator=(const WallFollowerClient&) = delete;

    void sendStart();
    void sendStop();
    void cancelStopRetry();
    void onStopRetryTick();
    bool isFresh() const;
    static std::string stateName(std::uint8_t state);
    void onStateMessage(FollowerState::ConstSharedPtr message);
    void onFollowLeftReply(std::uint64_t generation,
                           rclcpp::Client<SetBool>::SharedFuture future);
    void onStartReply(rclcpp::Client<Trigger>::SharedFuture future);
    void onStopReply(rclcpp::Client<Trigger>::SharedFuture future);

    // Same relative names the wall follower uses, so namespaces and remaps match.
    static const char* const FollowLeftServiceName;
    static const char* const StartServiceName;
    static const char* const StopServiceName;
    static const char* const StateTopicName;
    static const std::size_t StateQueueDepth;
    // A state message older than this means the follower is gone or stalled.
    static const double StateTimeoutSeconds;
    // How often an unconfirmed stop is repeated, and how many times in total.
    static const int StopRetryPeriodMilliseconds;
    static const unsigned MaxStopAttempts;

    rclcpp::Node& node_;
    rclcpp::Client<SetBool>::SharedPtr followLeftClient_;
    rclcpp::Client<Trigger>::SharedPtr startClient_;
    rclcpp::Client<Trigger>::SharedPtr stopClient_;
    rclcpp::Subscription<FollowerState>::SharedPtr stateSubscription_;
    rclcpp::TimerBase::SharedPtr stopRetryTimer_;
    bool stateReceived_;
    bool scanValid_;
    std::uint8_t followerState_;
    // Raised by every start() and stop(). A reply that carries an older value
    // belongs to a request that has been overtaken and must not act.
    std::uint64_t requestGeneration_;
    bool stopWanted_;
    unsigned stopAttempts_;
    // Wall-clock arrival time, so the check still works if simulation time is paused.
    std::chrono::steady_clock::time_point lastStateTime_;
};
#endif