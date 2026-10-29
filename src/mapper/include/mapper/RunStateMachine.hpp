// MTRX3760 2026 Project 2: Package Deal
// File: RunStateMachine.hpp
// Author(s): Thea Macarthur-Lassen
//
// Tracks the life of one mapping run: Ready, Exploring, Finished. Plain C++ with
// no ROS types, so the mapper can ask "may I act?" and tests can drive it directly.
#ifndef MAPPER_RUN_STATE_MACHINE_HPP
#define MAPPER_RUN_STATE_MACHINE_HPP

#include <string>

class RunStateMachine
{
public:
    // A new run always starts Ready.
    RunStateMachine();

    bool isReady() const;
    bool isExploring() const;
    bool isFinished() const;

    // Transition from Ready -> Exploring. Ignored in any other state.
    void begin();
    // Transition from Exploring -> Finished. Ignored in any other state, so a run cannot finish
    // before it has begun and cannot be restarted once finished.
    void finish();

    // Readable state for the status topic and logs.
    std::string name() const;

private:
    enum class State { ready, exploring, finished };
    State state_;
};
#endif