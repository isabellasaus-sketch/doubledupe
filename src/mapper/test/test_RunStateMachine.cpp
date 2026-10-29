// MTRX3760 2026 Project 2: Package Deal
// File: test_RunStateMachine.cpp
// Author(s): Thea Macarthur-Lassen
//
#include <gtest/gtest.h>
#include "mapper/RunStateMachine.hpp"

TEST(RunStateMachineTest, StartsReady)
{
    RunStateMachine machine;
    EXPECT_TRUE(machine.isReady());
    EXPECT_EQ(machine.name(), "Ready");
}

TEST(RunStateMachineTest, NormalLifecycle)
{
    RunStateMachine machine;
    machine.begin();
    EXPECT_TRUE(machine.isExploring());
    machine.finish();
    EXPECT_TRUE(machine.isFinished());
    EXPECT_EQ(machine.name(), "Finished");
}

TEST(RunStateMachineTest, CannotFinishBeforeBegin)
{
    RunStateMachine machine;
    machine.finish();
    EXPECT_TRUE(machine.isReady());
}

TEST(RunStateMachineTest, BeginTwiceIsHarmless)
{
    RunStateMachine machine;
    machine.begin();
    machine.begin();
    EXPECT_TRUE(machine.isExploring());
}

TEST(RunStateMachineTest, FinishedCannotRestart)
{
    RunStateMachine machine;
    machine.begin();
    machine.finish();
    machine.begin();
    EXPECT_TRUE(machine.isFinished());
}