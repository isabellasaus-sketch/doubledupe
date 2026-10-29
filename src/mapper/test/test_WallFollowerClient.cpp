// MTRX3760 2026 Project 2: Package Deal
// File: test_WallFollowerClient.cpp
// Author(s): Thea Macarthur-Lassen
//
// Tests WallFollowerClient against FakeFollower, a stand-in for the adopted
// wall follower that offers the same three services and the same state topic.
// Both nodes sit in a private namespace, so the test never reaches a real
// follower running on the same network. No simulator or robot is needed.
#include <chrono>
#include <functional>
#include <memory>
#include <string>
#include <thread>
#include <vector>
#include <gtest/gtest.h>
#include <rclcpp/rclcpp.hpp>
#include <std_srvs/srv/set_bool.hpp>
#include <std_srvs/srv/trigger.hpp>
#include <wall_follower/msg/wall_follower_state.hpp>
#include "mapper/WallFollowerClient.hpp"

// Behaves like the real follower: start makes it FOLLOWING, stop makes it
// PAUSED. It can be told to ignore its first few stops, to stop publishing,
// or to report an invalid scan, so the client's edge cases can be provoked.
class FakeFollower : public rclcpp::Node
{
public:
    typedef std_srvs::srv::SetBool SetBool;
    typedef std_srvs::srv::Trigger Trigger;
    typedef wall_follower::msg::WallFollowerState FollowerState;

    FakeFollower()
        : Node("fake_follower", "mapper_test"), state_(FollowerState::PAUSED),
          scanValid_(true), publishing_(true), stopsToIgnore_(0), calls_()
    {
        followLeftService_ = create_service<SetBool>(
            "wall_follower/follow_left",
            [this](const std::shared_ptr<SetBool::Request>,
                   std::shared_ptr<SetBool::Response> response)
            {
                calls_.push_back("follow_left");
                response->success = true;
            });
        startService_ = create_service<Trigger>(
            "wall_follower/start",
            [this](const std::shared_ptr<Trigger::Request>,
                   std::shared_ptr<Trigger::Response> response)
            {
                calls_.push_back("start");
                state_ = FollowerState::FOLLOWING;
                response->success = true;
            });
        stopService_ = create_service<Trigger>(
            "wall_follower/stop",
            [this](const std::shared_ptr<Trigger::Request>,
                   std::shared_ptr<Trigger::Response> response)
            {
                calls_.push_back("stop");
                if(stopsToIgnore_ > 0)
                {
                    --stopsToIgnore_;
                }
                else
                {
                    state_ = FollowerState::PAUSED;
                }
                response->success = true;
            });
        publisher_ = create_publisher<FollowerState>("wall_follower/state", 10);
        timer_ = create_wall_timer(
            std::chrono::milliseconds(PublishPeriodMilliseconds),
            [this]() { publishState(); });
    }

    void setScanValid(bool valid) { scanValid_ = valid; }
    void setPublishing(bool publishing) { publishing_ = publishing; }
    void setStopsToIgnore(unsigned count) { stopsToIgnore_ = count; }
    const std::vector<std::string>& calls() const { return calls_; }

    std::size_t count(const std::string& name) const
    {
        std::size_t total = 0;
        for(const std::string& call : calls_)
        {
            if(call == name)
            {
                ++total;
            }
        }
        return total;
    }

private:
    inline static constexpr int PublishPeriodMilliseconds = 20;

    void publishState()
    {
        if(publishing_)
        {
            FollowerState message;
            message.header.stamp = now();
            message.state = state_;
            message.follow_left = true;
            message.scan_valid = scanValid_;
            publisher_->publish(message);
        }
    }

    std::uint8_t state_;
    bool scanValid_;
    bool publishing_;
    unsigned stopsToIgnore_;
    std::vector<std::string> calls_;
    rclcpp::Service<SetBool>::SharedPtr followLeftService_;
    rclcpp::Service<Trigger>::SharedPtr startService_;
    rclcpp::Service<Trigger>::SharedPtr stopService_;
    rclcpp::Publisher<FollowerState>::SharedPtr publisher_;
    rclcpp::TimerBase::SharedPtr timer_;
};

// Runs the client and the fake on one thread, so no locking is needed.
class WallFollowerClientTest : public ::testing::Test
{
protected:
    static void SetUpTestSuite() { rclcpp::init(0, nullptr); }
    static void TearDownTestSuite() { rclcpp::shutdown(); }

    WallFollowerClientTest()
        : fake_(std::make_shared<FakeFollower>()),
          clientNode_(std::make_shared<rclcpp::Node>("client_node", "mapper_test")),
          client_(*clientNode_), executor_()
    {
        executor_.add_node(fake_);
        executor_.add_node(clientNode_);
    }

    void SetUp() override
    {
        // Wait until the fake's services are visible, then let the client's own
        // service clients notice them too.
        const std::chrono::seconds discoveryTimeout(5);
        ASSERT_TRUE(clientNode_->create_client<std_srvs::srv::SetBool>(
            "wall_follower/follow_left")->wait_for_service(discoveryTimeout));
        ASSERT_TRUE(clientNode_->create_client<std_srvs::srv::Trigger>(
            "wall_follower/start")->wait_for_service(discoveryTimeout));
        ASSERT_TRUE(clientNode_->create_client<std_srvs::srv::Trigger>(
            "wall_follower/stop")->wait_for_service(discoveryTimeout));
        ASSERT_TRUE(waitUntil(
            [this]() { return client_.followerStateName() == "PAUSED"; },
            std::chrono::seconds(5)));
        spinFor(std::chrono::milliseconds(300));
    }

    // Spins both nodes until the condition holds or the time runs out.
    bool waitUntil(const std::function<bool()>& condition,
                   std::chrono::milliseconds timeout)
    {
        const std::chrono::steady_clock::time_point deadline =
            std::chrono::steady_clock::now() + timeout;
        bool met = condition();
        while(!met && std::chrono::steady_clock::now() < deadline)
        {
            executor_.spin_some();
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
            met = condition();
        }
        return met;
    }

    void spinFor(std::chrono::milliseconds duration)
    {
        waitUntil([]() { return false; }, duration);
    }

    std::shared_ptr<FakeFollower> fake_;
    std::shared_ptr<rclcpp::Node> clientNode_;
    WallFollowerClient client_;
    rclcpp::executors::SingleThreadedExecutor executor_;
};

TEST_F(WallFollowerClientTest, StartSendsFollowLeftThenStart)
{
    client_.start();
    EXPECT_TRUE(waitUntil([this]() { return fake_->count("start") == 1; },
                          std::chrono::seconds(3)));
    const std::vector<std::string> expected = {"follow_left", "start"};
    EXPECT_EQ(fake_->calls(), expected);
    EXPECT_TRUE(waitUntil(
        [this]() { return client_.followerStateName() == "FOLLOWING"; },
        std::chrono::seconds(3)));
}

TEST_F(WallFollowerClientTest, ScanValidFollowsTheStateMessages)
{
    EXPECT_TRUE(waitUntil([this]() { return client_.isScanValid(); },
                          std::chrono::seconds(3)));
    fake_->setScanValid(false);
    EXPECT_TRUE(waitUntil([this]() { return !client_.isScanValid(); },
                          std::chrono::seconds(3)));
    fake_->setScanValid(true);
    EXPECT_TRUE(waitUntil([this]() { return client_.isScanValid(); },
                          std::chrono::seconds(3)));
}

TEST_F(WallFollowerClientTest, StaleStateIsNotValid)
{
    EXPECT_TRUE(waitUntil([this]() { return client_.isScanValid(); },
                          std::chrono::seconds(3)));
    fake_->setPublishing(false);
    spinFor(std::chrono::milliseconds(1400));
    EXPECT_FALSE(client_.isScanValid());
    EXPECT_EQ(client_.followerStateName(), "STATE STALE");
}

TEST_F(WallFollowerClientTest, StopOvertakesAStartStillInFlight)
{
    // Nothing has been spun yet, so both requests are queued. The follow_left
    // reply that comes back belongs to an overtaken start and must not send start.
    client_.start();
    client_.stop();
    spinFor(std::chrono::milliseconds(1500));
    EXPECT_EQ(fake_->count("follow_left"), 1u);
    EXPECT_EQ(fake_->count("start"), 0u);
    EXPECT_EQ(fake_->count("stop"), 1u);
}

TEST_F(WallFollowerClientTest, StopIsRepeatedUntilTheFollowerIsPaused)
{
    client_.start();
    EXPECT_TRUE(waitUntil(
        [this]() { return client_.followerStateName() == "FOLLOWING"; },
        std::chrono::seconds(3)));

    // The first two stops are acknowledged but have no effect.
    fake_->setStopsToIgnore(2);
    client_.stop();
    EXPECT_TRUE(waitUntil([this]() { return fake_->count("stop") >= 3; },
                          std::chrono::seconds(5)));
    EXPECT_TRUE(waitUntil(
        [this]() { return client_.followerStateName() == "PAUSED"; },
        std::chrono::seconds(3)));

    // Once PAUSED is seen, the retries must end.
    spinFor(std::chrono::milliseconds(1500));
    EXPECT_EQ(fake_->count("stop"), 3u);
}