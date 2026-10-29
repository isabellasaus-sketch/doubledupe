// MTRX3760 2026 Project 2: Package Deal
// File: Passage.cpp
// Author(s): Isabella Armitstead
// Description: Implements a tagged connection between two rooms.

#include "mapper/Passage.hpp"

Passage::Passage(int tagId, std::size_t roomA, std::size_t roomB)
    : tagId_(tagId), roomA_(roomA), roomB_(roomB)
{
}

int Passage::tagId() const
{
    return tagId_;
}

std::size_t Passage::roomA() const
{
    return roomA_;
}

std::size_t Passage::roomB() const
{
    return roomB_;
}

bool Passage::connectsRoom(std::size_t room) const
{
    return room == roomA_ || room == roomB_;
}

std::size_t Passage::otherEnd(std::size_t room) const
{
    return room == roomA_ ? roomB_ : roomA_;
}

void Passage::recordCrossing()
{
    ++crossingCount_;
}

bool Passage::isFullyCrossed() const
{
    return crossingCount_ >= kCrossingsWhenFullyCrossed;
}