// MTRX3760 2026 Project 2: Package Deal
// File: CInventory.cpp
// Author(s): Christina Miljak, Davin Pranata, Joshua Samuel David, Joshua Kwok, Kaif Masiudddin Mohammed, Kieran Wong
//
// Publishes each newly confirmed marker encounter and the ordered inventory.

#include "CInventory.hpp"
#include <sstream>


// Passed the node so it can create the publishers.
// The total list is latched (transient local, keep only the latest): a subscriber that starts
// later, or after the images have stopped, still receives the current list straight away.
CInventory::CInventory(rclcpp::Node & node)
    : mLogger(node.get_logger()),
      mEncounterPub(node.create_publisher<std_msgs::msg::Int32>("inventory/encounter", 10)),
      mTotalTagsPub(node.create_publisher<std_msgs::msg::Int32MultiArray>(
          "inventory/total_tags", rclcpp::QoS(1).reliable().transient_local())),
      mTotalTags()
{
    // Publish the empty list so the latched topic always holds something to read.
    publish_total_tags();
}
// Print the complete ordered log once, when the node shuts down.
CInventory::~CInventory()
{
    std::ostringstream list;
    for (size_t i = 0; i < mTotalTags.size(); ++i) {
        list << (i ? ", " : "") << mTotalTags[i];
    }
    RCLCPP_INFO(mLogger, "total tags are: [%s]", list.str().c_str());
}


// Publish each new ID individually, then refresh the complete history topic.
void CInventory::add_encounters(const std::vector<int> & aNewIds)
{
    if (aNewIds.empty()) {
        return;
    }

    for (const int id : aNewIds) {
        mTotalTags.push_back(id);

        std_msgs::msg::Int32 msg;
        msg.data = id;
        mEncounterPub->publish(msg);
        RCLCPP_INFO(mLogger, "Encounter: tag %d", id);
    }
    publish_total_tags();
}

// Allow callers to inspect the ordered tag history without modifying it.
const std::vector<int> & CInventory::get_total_tags() const
{
    return mTotalTags;
}

// Publish the current complete log using the existing transient-local publisher.
void CInventory::publish_total_tags() const
{
    std_msgs::msg::Int32MultiArray msg;
    msg.data = mTotalTags;
    mTotalTagsPub->publish(msg);
}
