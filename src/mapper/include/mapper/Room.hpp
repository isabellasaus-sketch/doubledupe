// MTRX3760 2026 Project 2: Package Deal
// File: Room.hpp
// Author(s): Isabella Armitstead
// Description: Represents a room with an optional, set once type label.

#ifndef MAPPER_ROOM_HPP
#define MAPPER_ROOM_HPP

#include <string>

class Room {
public:
    Room() = default;

    bool hasType() const;
    const std::string& type() const;
    void setType(const std::string& label);

private:
    // An empty label means this room has not been classified yet.
    std::string type_;
};

#endif