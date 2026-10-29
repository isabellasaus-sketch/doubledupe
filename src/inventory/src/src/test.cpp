// MTRX3760 2026 Project 2: Package Deal
// File: test.cpp
// Author(s): Christina Miljak, Davin Pranata, Joshua Samuel David, Joshua Kwok, Kaif Masiudddin Mohammed, Kieran Wong
//
// Runs standalone C++ checks for the marker encounter state machine.

#include <cstdlib>
#include <iostream>
#include <vector>

#include "CEncounter.hpp"

// Keep test result counters and assertions local to this translation unit.
namespace {
int gFailures = 0;

// Print each assertion result and record any failure for the final exit code.
void expect(bool condition, const char * what)
{
    std::cout << (condition ? "PASS: " : "FAIL: ") << what << std::endl;
    if (!condition) {
        ++gFailures;
    }
}
}  // namespace

// Exercise confirmation, dropout, re-encounter and duplicate handling.
int main()
{
    const double kTimeoutS = 1.0;
    const int kMinFrames = 3;

    // A tag becomes one encounter only after enough consecutive frames.
    {
        CEncounter enc(kTimeoutS, kMinFrames);
        expect(enc.update({7}, 0.0).empty(), "frame 1 of 3 is not yet an encounter");
        expect(enc.update({7}, 0.1).empty(), "frame 2 of 3 is not yet an encounter");
        expect(enc.update({7}, 0.2) == std::vector<int>{7}, "frame 3 of 3 confirms the encounter");
        expect(enc.update({7}, 0.3).empty(), "staying in view is still one encounter");
        expect(enc.get_log() == std::vector<int>{7}, "log holds one entry");
    }
    {
        CEncounter enc(kTimeoutS, kMinFrames);
        enc.update({7}, 0.0);
        enc.update({}, 0.1);  // drops out before being confirmed
        enc.update({7}, 0.2);
        expect(enc.update({7}, 0.3).empty(), "a flicker resets the consecutive-frame count");
        expect(enc.get_log().empty(), "log stays empty after a flicker");
    }
    {
        CEncounter enc(kTimeoutS, kMinFrames);
        for (int i = 0; i < 3; ++i) { enc.update({7}, 0.1 * i); }
        enc.update({}, 0.5);
        for (int i = 0; i < 3; ++i) { enc.update({7}, 0.6 + 0.1 * i); }
        expect(enc.get_log().size() == 1, "reappearing within the timeout is the same encounter");
    }
    {
        CEncounter enc(kTimeoutS, kMinFrames);
        for (int i = 0; i < 3; ++i) { enc.update({7}, 0.1 * i); }
        enc.update({}, 2.0);  // gone for longer than the timeout
        for (int i = 0; i < 3; ++i) { enc.update({7}, 3.0 + 0.1 * i); }
        expect(enc.get_log() == std::vector<int>({7, 7}), "reappearing after the timeout is a new encounter");
    }
    {
        CEncounter enc(kTimeoutS, kMinFrames);
        for (int i = 0; i < 3; ++i) { enc.update({4, 4, 9}, 0.1 * i); }
        expect(enc.get_log() == std::vector<int>({4, 9}), "duplicate IDs in one frame count once, in ID order");
        enc.reset();
        expect(enc.get_log().empty(), "reset clears the log");
    }

    return gFailures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
