// MTRX3760 2026 Project 2: Package Deal
// File: CInventory.hpp
// Author(s): Christina Miljak, Davin Pranata, Joshua Samuel David, Joshua Kwok, Kaif Masiudddin Mohammed, Kieran Wong
//
// Declares CInventory, which publishes each tag encounter as it happens and
// the ordered history of all encounters so far.

#ifndef CINVENTORY_HPP_
#define CINVENTORY_HPP_

#include <vector>

#include <rclcpp/rclcpp.hpp>
#include <std_msgs/msg/int32.hpp>
#include <std_msgs/msg/int32_multi_array.hpp>

#include "CEncounter.hpp"

// Owns the ROS interfaces for reporting tag encounters to other packages.
class CInventory {
public:
    // Creates the publishers on the node. Only the publishers and the node's logger
    // are kept, not the node itself.
    explicit CInventory(rclcpp::Node & node);

    // Record newly confirmed encounters: publishes each one, then the updated log.
    void add_encounters(const std::vector<int> & aNewIds);

    // Every encounter so far, in the order they happened.
    const std::vector<int> & get_total_tags() const;

    // Logs the final inventory when the node shuts down.
    ~CInventory();


private:
    // Publishes the whole ordered log on inventory/total_tags.
    void publish_total_tags() const;

    // Logger of the owning node, so messages appear under that node's name.
    rclcpp::Logger mLogger;

    // Publishes the ID of each tag as it becomes a new encounter (std_msgs/Int32).
    rclcpp::Publisher<std_msgs::msg::Int32>::SharedPtr mEncounterPub;

    // Publishes the whole ordered log (std_msgs/Int32MultiArray).
    rclcpp::Publisher<std_msgs::msg::Int32MultiArray>::SharedPtr mTotalTagsPub;

    // Every encounter so far, in order. An ID appears again after each separate encounter.
    std::vector<int> mTotalTags;
};

#endif // CINVENTORY_HPP_