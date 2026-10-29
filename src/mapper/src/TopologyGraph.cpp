// MTRX3760 2026 Project 2: Package Deal
// File: TopologyGraph.cpp
// Author(s): Isabella Armitstead
// Description: Builds the topological map: crossings, room labels, DOT output.

#include "mapper/TopologyGraph.hpp"

#include <algorithm>
#include <cctype>
#include <queue>
#include <sstream>

TopologyGraph::TopologyGraph()
    : rooms_(1)
{
    // Start with one room. Its type label is initially empty.
}

 
bool TopologyGraph::crossPassage(int tagId)
{
    Passage* passage = findPassage(tagId);

    if (passage == nullptr) {
        // The arena is a tree, so an unseen tag always leads to a new room.
        const std::size_t newRoom = rooms_.size();
        rooms_.emplace_back();
        passages_.emplace_back(tagId, currentRoom_, newRoom);
        passage = &passages_.back();
    }

    // A known tag should touch the room the robot is in; if not, catch up first.
    const bool consistent = passage->connectsRoom(currentRoom_);
    if (!consistent) {
        resyncToPassage(*passage);
    }

    passage->recordCrossing();
    previousRoom_ = currentRoom_;
    currentRoom_ = passage->otherEnd(currentRoom_);
    tryAssignLabelToCurrentRoom();
    return consistent;
}

void TopologyGraph::resyncToPassage(const Passage& passage)
{
    const std::vector<std::size_t> toA = pathBetween(currentRoom_, passage.roomA());
    const std::vector<std::size_t> toB = pathBetween(currentRoom_, passage.roomB());
    const bool aIsNearer = toA.size() <= toB.size();
    const std::vector<std::size_t>& missed = aIsNearer ? toA : toB;

    for (std::size_t index : missed) {
        passages_[index].recordCrossing();
    }
    // The latest label may belong to a room the graph did not know the robot
    // was in, so the room it arrives in waits for a fresh label instead.
    latestLabel_.clear();
    currentRoom_ = aIsNearer ? passage.roomA() : passage.roomB();
}

std::vector<std::size_t> TopologyGraph::pathBetween(std::size_t from, std::size_t to) const
{
    // Breadth-first search, remembering the passage used to reach each room.
    std::vector<std::size_t> viaPassage(rooms_.size(), kNoRoom);
    std::vector<bool> visited(rooms_.size(), false);
    std::queue<std::size_t> frontier;
    visited[from] = true;
    frontier.push(from);
    while (!frontier.empty() && !visited[to]) {
        const std::size_t room = frontier.front();
        frontier.pop();
        for (std::size_t index = 0; index < passages_.size(); ++index) {
            const Passage& passage = passages_[index];
            if (passage.connectsRoom(room) && !visited[passage.otherEnd(room)]) {
                visited[passage.otherEnd(room)] = true;
                viaPassage[passage.otherEnd(room)] = index;
                frontier.push(passage.otherEnd(room));
            }
        }
    }

    std::vector<std::size_t> path;
    for (std::size_t room = to; room != from && viaPassage[room] != kNoRoom;
         room = passages_[viaPassage[room]].otherEnd(room)) {
        path.push_back(viaPassage[room]);
    }
    std::reverse(path.begin(), path.end());
    return path;
}

void TopologyGraph::assignTypeLabel(const std::string& label)
{
    latestLabel_ = label;
    tryAssignLabelToCurrentRoom();
}

void TopologyGraph::tryAssignLabelToCurrentRoom()
{
    Room& room = rooms_[currentRoom_];
    if (room.hasType() || latestLabel_.empty()) {
        return;
    }

    // Connected rooms never share a type, so a label equal to the previous
    // room's type is the old room's label, not the new room's.
    const bool isPreviousRoomsLabel =
        previousRoom_ != kNoRoom && rooms_[previousRoom_].type() == latestLabel_;
    if (!isPreviousRoomsLabel) {
        room.setType(latestLabel_);
    }
}

bool TopologyGraph::isFullyTraversed() const
{
    // With no passages nothing has been explored yet, so it is not finished.
    return !passages_.empty()
        && std::all_of(passages_.begin(), passages_.end(),
                       [](const Passage& passage) { return passage.isFullyCrossed(); });
}

std::string TopologyGraph::toDot() const
{
    std::ostringstream dot;
    dot << "@startdot\n";
    dot << "graph arena {\n";
    
    // Rooms are numbered from 1, in the order they were discovered.
    for (std::size_t room = 0; room < rooms_.size(); ++room) {
        dot << "  r" << room + 1 << " [label=\"" << roomLabel(rooms_[room]) << "\"];\n";
    }
    for (const Passage& passage : passages_) {
        dot << "  r" << passage.roomA() + 1 << " -- r" << passage.roomB() + 1
            << " [label=\"tag " << passage.tagId() << "\"];\n";
    }

    dot << "}\n";
    dot << "@enddot\n";
    return dot.str();
}

std::string TopologyGraph::roomLabel(const Room& room)
{
    if (!room.hasType()) {
        return kUnknownRoomLabel;
    }

    // Room shape labels look like SMALL_SQUARE; the handout shows "small square".
    std::string label = room.type();
    std::transform(label.begin(), label.end(), label.begin(), [](unsigned char c) {
        return c == '_' ? ' ' : static_cast<char>(std::tolower(c));
    });
    return label;
}

Passage* TopologyGraph::findPassage(int tagId)
{
    for (Passage& passage : passages_) {
        if (passage.tagId() == tagId) {
            return &passage;
        }
    }
    return nullptr;
}

std::string TopologyGraph::statusText() const
{
    return "Rooms: " + std::to_string(rooms_.size())
        + ", passages: " + std::to_string(passages_.size())
        + ", current room: " + std::to_string(currentRoom_ + 1);
}