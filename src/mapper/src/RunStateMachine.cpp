// MTRX3760 2026 Project 2: Package Deal
// File: RunStateMachine.cpp
// Author(s): Thea Macarthur-Lassen
//
// Implements the three-state run lifecycle. Illegal transitions are ignored
// rather than reported, so late or duplicate events cannot corrupt the run.


#include "mapper/RunStateMachine.hpp"

RunStateMachine::RunStateMachine()
    : state_(State::ready)
{
}
 
bool RunStateMachine::isReady() const
{
    return state_ == State::ready;
}
 
bool RunStateMachine::isExploring() const
{
    return state_ == State::exploring;
}
 
bool RunStateMachine::isFinished() const
{
    return state_ == State::finished;
}
 
void RunStateMachine::begin()
{
    if(state_ == State::ready)
    {
        state_ = State::exploring;
    }
}
 
void RunStateMachine::finish()
{
    if(state_ == State::exploring)
    {
        state_ = State::finished;
    }
}
 
std::string RunStateMachine::name() const
{
    std::string name = "Unknown";
    switch(state_)
    {
        case State::ready:
            name = "Ready";
            break;
        case State::exploring:
            name = "Exploring";
            break;
        case State::finished:
            name = "Finished";
            break;
        default:
            break;
    }
    return name;
}
