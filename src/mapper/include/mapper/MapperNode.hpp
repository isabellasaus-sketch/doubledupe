/*
 * MTRX3760 2026 Project 2: Package Deal
 * File: MapperNode.hpp
 * Author(s): Charlotte Scamps
 * Description: ROS-facing mapper node contract. The node owns the graph, the run
 * state, and the wall-follower client so the rest of the code stays focused on
 * the actual room-traversal logic.
 */

#ifndef MAPPER_NODE_HPP_
#define MAPPER_NODE_HPP_

#include "mapper/RunStateMachine.hpp"
#include "mapper/TopologyGraph.hpp"
#include "mapper/WallFollowerClient.hpp"

#include <map>
#include <string>

#include <rclcpp/rclcpp.hpp>
#include <std_msgs/msg/int32.hpp>
#include <std_msgs/msg/string.hpp>

// This is the ROS-facing controller for the mapper.
// It listens to incoming robot data and decides whether the graph or wall
// follower needs to react.
class MapperNode : public rclcpp::Node
{
public:
  MapperNode();
  // Saves whatever map was built if the node is shut down mid-run.
  ~MapperNode() override;

private:
  // Inventory encounter data tells us which tile or passage we have reached.
  void encounterCallback(const std_msgs::msg::Int32::SharedPtr msg);
  // Room-shape labels let the mapper update its topology.
  void roomTypeCallback(const std_msgs::msg::String::SharedPtr msg);
  // Begins the run once the wall-follower services are ready to accept the
  // start request. Polled by a timer while the run is still Ready.
  void checkWallFollowerStart();
  // Stops the robot and writes the finished map.
  void finishRun();
  // A repeat of the same tag inside the cooldown is the same physical crossing.
  bool isInCooldown(int tagId);
  void writeDot() const;
  void publishStatus();

  TopologyGraph graph_;
  RunStateMachine runState_;
  WallFollowerClient follower_;

  // Tag on the start tile. It is not a passage, and seeing it again once every
  // passage has been crossed both ways ends the run. -1 means "not set", in
  // which case the run ends as soon as the graph is fully traversed.
  int startTileId_;
  double cooldownS_;
  std::string outputPath_;
  std::map<int, rclcpp::Time> lastEncounterTime_;

  rclcpp::Subscription<std_msgs::msg::Int32>::SharedPtr encounter_sub_;
  rclcpp::Subscription<std_msgs::msg::String>::SharedPtr room_type_sub_;
  rclcpp::Publisher<std_msgs::msg::String>::SharedPtr status_pub_;
  rclcpp::TimerBase::SharedPtr start_timer_;
};

#endif  // MAPPER_NODE_HPP_
