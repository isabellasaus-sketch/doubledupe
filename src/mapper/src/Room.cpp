// MTRX3760 2026 Project 2: Package Deal
// File: Room.cpp
// Author(s): Isabella Armitstead
// Description: Implements the set once type label of a room.

#include "mapper/Room.hpp"

bool Room::hasType() const
{
    return !type_.empty();
}

const std::string& Room::type() const
{
    return type_;
}

void Room::setType(const std::string& label)
{
    // A room's type is set once and never overwritten.
    if (!hasType()) {
        type_ = label;
    }
}