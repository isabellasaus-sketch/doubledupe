// MTRX3760 2026 Project 2: Package Deal
// File: WallFollowerClient.cpp
// Author(s): Thea Macarthur-Lassen
//
// Implements the asynchronous start/stop requests to the adopted wall follower,
// the repeat-until-confirmed stop, and the freshness check on its state topic.
#include "mapper/WallFollowerClient.hpp"
#include <functional>
#include <memory>

const char* const WallFollowerClient::FollowLeftServiceName = "wall_follower/follow_left";
const char* const WallFollowerClient::StartServiceName = "wall_follower/start";
const char* const WallFollowerClient::StopServiceName = "wall_follower/stop";
const char* const WallFollowerClient::StateTopicName = "wall_follower/state";
const std::size_t WallFollowerClient::StateQueueDepth = 10;
const double WallFollowerClient::StateTimeoutSeconds = 1.0;
const int WallFollowerClient::StopRetryPeriodMilliseconds = 500;
const unsigned WallFollowerClient::MaxStopAttempts = 10;

WallFollowerClient::WallFollowerClient(rclcpp::Node& node)
    : node_(node), followLeftClient_(), startClient_(), stopClient_(),
      stateSubscription_(), stopRetryTimer_(), stateReceived_(false),
      scanValid_(false), followerState_(FollowerState::PAUSED),
      requestGeneration_(0), stopWanted_(false), stopAttempts_(0), lastStateTime_()
{
    followLeftClient_ = node_.create_client<SetBool>(FollowLeftServiceName);
    startClient_ = node_.create_client<Trigger>(StartServiceName);
    stopClient_ = node_.create_client<Trigger>(StopServiceName);
    stateSubscription_ = node_.create_subscription<FollowerState>(
        StateTopicName, rclcpp::QoS(rclcpp::KeepLast(StateQueueDepth)),
        std::bind(&WallFollowerClient::onStateMessage, this, std::placeholders::_1));
    stopRetryTimer_ = node_.create_wall_timer(
        std::chrono::milliseconds(StopRetryPeriodMilliseconds),
        [this]() { onStopRetryTick(); });
    stopRetryTimer_->cancel();
}

void WallFollowerClient::start()
{
    // Overtakes any earlier request, and a pending stop no longer applies.
    ++requestGeneration_;
    cancelStopRetry();
    if(followLeftClient_->service_is_ready() && startClient_->service_is_ready())
    {
        std::shared_ptr<SetBool::Request> request = std::make_shared<SetBool::Request>();
        request->data = true;
        // A lambda, not std::bind: rclcpp cannot read the argument type of a
        // std::bind result when it checks the callback.
        followLeftClient_->async_send_request(
            request,
            [this, generation = requestGeneration_](
                rclcpp::Client<SetBool>::SharedFuture future)
            {
                onFollowLeftReply(generation, future);
            });
    }
    else
    {
        RCLCPP_ERROR(node_.get_logger(), "Wall follower services not ready; start ignored");
    }
}

void WallFollowerClient::sendStart()
{
    std::shared_ptr<Trigger::Request> request = std::make_shared<Trigger::Request>();
    startClient_->async_send_request(
        request,
        [this](rclcpp::Client<Trigger>::SharedFuture future) { onStartReply(future); });
}

void WallFollowerClient::stop()
{
    // A start still waiting for its follow_left reply must not fire after this.
    ++requestGeneration_;
    stopWanted_ = true;
    stopAttempts_ = 0;
    sendStop();
    stopRetryTimer_->reset();
}

void WallFollowerClient::sendStop()
{
    ++stopAttempts_;
    if(stopClient_->service_is_ready())
    {
        std::shared_ptr<Trigger::Request> request = std::make_shared<Trigger::Request>();
        stopClient_->async_send_request(
            request,
            [this](rclcpp::Client<Trigger>::SharedFuture future) { onStopReply(future); });
    }
    else
    {
        RCLCPP_ERROR(node_.get_logger(),
                     "Wall follower stop service not ready (attempt %u of %u)",
                     stopAttempts_, MaxStopAttempts);
    }
}

void WallFollowerClient::cancelStopRetry()
{
    stopWanted_ = false;
    stopRetryTimer_->cancel();
}

void WallFollowerClient::onStopRetryTick()
{
    const bool confirmed = isFresh() && followerState_ == FollowerState::PAUSED;
    if(!stopWanted_)
    {
        cancelStopRetry();
    }
    else if(confirmed)
    {
        RCLCPP_INFO(node_.get_logger(), "Wall follower confirmed stopped");
        cancelStopRetry();
    }
    else if(stopAttempts_ >= MaxStopAttempts)
    {
        RCLCPP_ERROR(node_.get_logger(),
                     "Wall follower never confirmed the stop after %u attempts; "
                     "stop the robot by hand", stopAttempts_);
        cancelStopRetry();
    }
    else
    {
        sendStop();
    }
}

bool WallFollowerClient::isAvailable() const
{
    return followLeftClient_->service_is_ready() && startClient_->service_is_ready()
        && stopClient_->service_is_ready();
}

bool WallFollowerClient::isScanValid() const
{
    return isFresh() && scanValid_;
}

std::string WallFollowerClient::followerStateName() const
{
    std::string name = "NO STATE RECEIVED";
    if(stateReceived_)
    {
        name = "STATE STALE";
    }
    if(isFresh())
    {
        name = stateName(followerState_);
    }
    return name;
}

std::string WallFollowerClient::stateName(std::uint8_t state)
{
    std::string name = "UNKNOWN";
    switch(state)
    {
        case FollowerState::PAUSED:
            name = "PAUSED";
            break;
        case FollowerState::FOLLOWING:
            name = "FOLLOWING";
            break;
        case FollowerState::WAITING_FOR_SCAN:
            name = "WAITING_FOR_SCAN";
            break;
        default:
            break;
    }
    return name;
}

bool WallFollowerClient::isFresh() const
{
    const std::chrono::duration<double> age = std::chrono::steady_clock::now() - lastStateTime_;
    return stateReceived_ && age.count() <= StateTimeoutSeconds;
}

void WallFollowerClient::onStateMessage(FollowerState::ConstSharedPtr message)
{
    // The follower publishes constantly, so only log when the state changes.
    if(!stateReceived_ || message->state != followerState_)
    {
        RCLCPP_INFO(node_.get_logger(), "Wall follower state: %s",
                    stateName(message->state).c_str());
    }
    followerState_ = message->state;
    scanValid_ = message->scan_valid;
    lastStateTime_ = std::chrono::steady_clock::now();
    stateReceived_ = true;
}

void WallFollowerClient::onFollowLeftReply(std::uint64_t generation,
                                           rclcpp::Client<SetBool>::SharedFuture future)
{
    const std::shared_ptr<SetBool::Response> response = future.get();
    if(generation != requestGeneration_)
    {
        RCLCPP_INFO(node_.get_logger(),
                    "Ignoring a follow_left reply from a start that was overtaken");
    }
    else if(response->success)
    {
        sendStart();
    }
    else
    {
        RCLCPP_ERROR(node_.get_logger(), "follow_left rejected (%s); start NOT sent",
                     response->message.c_str());
    }
}

void WallFollowerClient::onStartReply(rclcpp::Client<Trigger>::SharedFuture future)
{
    const std::shared_ptr<Trigger::Response> response = future.get();
    if(!response->success)
    {
        RCLCPP_ERROR(node_.get_logger(), "start rejected: %s", response->message.c_str());
    }
}

void WallFollowerClient::onStopReply(rclcpp::Client<Trigger>::SharedFuture future)
{
    const std::shared_ptr<Trigger::Response> response = future.get();
    if(!response->success)
    {
        RCLCPP_ERROR(node_.get_logger(), "stop rejected: %s", response->message.c_str());
    }
}