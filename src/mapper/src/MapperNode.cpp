/*
 * MTRX3760 2026 Project 2: Package Deal
 * File: MapperNode.cpp
 * Author(s): Charlotte Scamps
 * Description: ROS-facing mapper node logic. This node translates incoming ROS
 * messages into graph updates and decides when the mapper is allowed to start
 * and stop the wall follower.
 */

#include "mapper/MapperNode.hpp"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <memory>

MapperNode::MapperNode()
: Node("mapper"),
  graph_(),
  runState_(),
  follower_(*this)
{
  // Parameters are kept intentionally lightweight for a single mapper run.
  startTileId_ = declare_parameter<int>("start_tile_id", -1);
  cooldownS_ = declare_parameter<double>("cooldown_s", 1.0);
  outputPath_ = declare_parameter<std::string>("output_path", "mapper.dot");

  // Listen for room-traversal encounters from the inventory pipeline.
  encounter_sub_ = create_subscription<std_msgs::msg::Int32>(
    "inventory/encounter",
    10,
    std::bind(&MapperNode::encounterCallback, this, std::placeholders::_1));

  // Listen for room-type classifications from the room-shape pipeline.
  room_type_sub_ = create_subscription<std_msgs::msg::String>(
    "room_shape/type",
    rclcpp::QoS(1).reliable().transient_local(),
    std::bind(&MapperNode::roomTypeCallback, this, std::placeholders::_1));

  // Latched so a late `ros2 topic echo` still sees the current run state.
  status_pub_ = create_publisher<std_msgs::msg::String>(
    "mapper/status", rclcpp::QoS(1).reliable().transient_local());

  // The follower usually comes up after the mapper, so keep checking until
  // its services answer instead of trying once at construction.
  start_timer_ = create_wall_timer(
    std::chrono::milliseconds(500), [this]() { checkWallFollowerStart(); });

  if (startTileId_ < 0) {
    RCLCPP_WARN(get_logger(),
      "start_tile_id is not set; the run will finish as soon as every passage "
      "has been crossed both ways");
  }
  publishStatus();
}

MapperNode::~MapperNode()
{
  if (runState_.isExploring()) {
    writeDot();
  }
}

void MapperNode::encounterCallback(const std_msgs::msg::Int32::SharedPtr msg)
{
  if (!msg) {
    return;
  }

  // Ignore invalid tile information before the mapper tries to update the graph.
  const int tagId = msg->data;
  if (tagId < 0 || isInCooldown(tagId)) {
    return;
  }

  // The start tile is not a passage; it only marks where the run can end.
  if (tagId == startTileId_) {
    if (runState_.isExploring() && graph_.isFullyTraversed()) {
      RCLCPP_INFO(get_logger(), "Back on the start tile with every passage crossed");
      finishRun();
    }
    return;
  }

  // Only record new passage information while the mapper is actively exploring.
  if (!runState_.isExploring()) {
    return;
  }

  if (!graph_.crossPassage(tagId)) {
    RCLCPP_WARN(get_logger(),
      "Tag %d does not touch the current room; missed crossings were filled in", tagId);
  }
  RCLCPP_INFO(get_logger(), "Crossed tag %d. %s", tagId, graph_.statusText().c_str());

  if (startTileId_ < 0 && graph_.isFullyTraversed()) {
    finishRun();
  } else {
    publishStatus();
  }
}

void MapperNode::checkWallFollowerStart()
{
  // The wall follower should only be started when the mapper is allowed to act
  // and the follower services are ready.
  if (!runState_.isReady()) {
    start_timer_->cancel();
    return;
  }
  if (!follower_.isAvailable()) {
    RCLCPP_INFO_THROTTLE(get_logger(), *get_clock(), 5000,
      "Waiting for the wall follower services");
    return;
  }

  start_timer_->cancel();
  runState_.begin();
  follower_.start();
  RCLCPP_INFO(get_logger(), "Run started; wall follower asked to follow the left wall");
  publishStatus();
}

void MapperNode::roomTypeCallback(const std_msgs::msg::String::SharedPtr msg)
{
  if (!msg) {
    return;
  }

  // A label can arrive before the run begins (it is latched), and it still
  // belongs to the start room, so only a finished run ignores labels.
  if (!runState_.isFinished()) {
    graph_.assignTypeLabel(msg->data);
  }
}

void MapperNode::finishRun()
{
  runState_.finish();
  follower_.stop();
  writeDot();
  publishStatus();
}

bool MapperNode::isInCooldown(int tagId)
{
  const rclcpp::Time now = this->now();
  const auto previous = lastEncounterTime_.find(tagId);
  if (previous != lastEncounterTime_.end() &&
    (now - previous->second).seconds() < cooldownS_)
  {
    return true;
  }
  lastEncounterTime_.insert_or_assign(tagId, now);
  return false;
}

void MapperNode::writeDot() const
{
  std::ofstream file(outputPath_);
  if (!file) {
    RCLCPP_ERROR(get_logger(), "Could not write the map to %s", outputPath_.c_str());
    return;
  }
  file << graph_.toDot();
  std::error_code error;
  const std::filesystem::path fullPath = std::filesystem::absolute(outputPath_, error);
  RCLCPP_INFO(get_logger(), "Map written to %s\n%s",
    fullPath.string().c_str(), graph_.toDot().c_str());
}

void MapperNode::publishStatus()
{
  std_msgs::msg::String status;
  status.data = runState_.name() + ". " + graph_.statusText();
  status_pub_->publish(status);
}
