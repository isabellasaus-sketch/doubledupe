// MTRX3760 2026 Project 2: Package Deal
// File: TopologyGraph.hpp
// Author(s): Isabella Armitstead
// Description: Owns the rooms and passages and tracks the robot's room.

#ifndef MAPPER_TOPOLOGY_GRAPH_HPP
#define MAPPER_TOPOLOGY_GRAPH_HPP

#include <cstddef>
#include <string>
#include <vector>

#include "mapper/Room.hpp"
#include "mapper/Passage.hpp"

class TopologyGraph {
public:
    TopologyGraph();

    // Records a crossing of the passage marked by this tag. Returns false when
    // a known tag does not touch the current room, meaning earlier crossings
    // were missed; the graph then resynchronises (see resyncToPassage) and
    // still records this crossing.
    bool crossPassage(int tagId);
    void assignTypeLabel(const std::string& label);
    bool isFullyTraversed() const;
    std::string toDot() const;
    std::string statusText() const;

private:

    // Marks "no previous room" (the robot has not left the start room yet).
    static constexpr std::size_t kNoRoom = static_cast<std::size_t>(-1);

    // DOT label for a room whose type was never received.
    static constexpr const char* kUnknownRoomLabel = "unknown";

    // Gives the current room the latest label if it is untyped and the label
    // is not just the previous room's stale type.
    void tryAssignLabelToCurrentRoom();

    // Text used for a room in the DOT output.
    static std::string roomLabel(const Room& room);

    // Returns the passage marked by this tag, or nullptr if it is unseen.
    Passage* findPassage(int tagId);

    // The robot is at a passage that does not touch its room, so crossings were
    // missed. In a tree it must have walked to the passage's nearer end; move
    // there and credit each passage on the way with the crossing it missed.
    void resyncToPassage(const Passage& passage);

    // Indices into passages_ along the unique tree path between two rooms.
    std::vector<std::size_t> pathBetween(std::size_t from, std::size_t to) const;

    std::vector<Room> rooms_;
    std::vector<Passage> passages_;
    std::size_t currentRoom_ = 0;
    std::size_t previousRoom_ = kNoRoom;
    // Latest label received. Kept because the room shape package only
    // publishes on change, and a label can arrive before or after a crossing.
    std::string latestLabel_;
};

#endif