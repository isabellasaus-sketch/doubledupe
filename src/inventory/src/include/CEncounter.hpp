// MTRX3760 2026 Project 2: Package Deal
// File: CEncounter.hpp
// Author(s): Christina Miljak, Davin Pranata, Joshua Samuel David, Joshua Kwok, Kaif Masiudddin Mohammed, Kieran Wong
//
// Tracks individual marker encounters across frames, including repeated visits.
// Maintains an ordered encounter log independently of ROS.

#ifndef CENCOUNTER_HPP_
#define CENCOUNTER_HPP_

#include "CTagState.hpp"

#include <algorithm>
#include <map>
#include <set>
#include <vector>

// Encapsulates the time-gap and consecutive-frame rules for each tag ID.
class CEncounter {

public:
    CEncounter(double absence_timeout_s, int min_consecutive_frames); //CTor

    // Feed one frame's detections. Returns the IDs that became new encounters on this frame.
    std::vector<int> update(const std::vector<int> & visible_ids, double stamp_s);

    // Every encounter so far, in the order they happened.
    const std::vector<int> & get_log() const;

    // Forget all tag state and the log.
    void reset();

private:

    // Per-ID state for currently or recently observed tags.
    std::map<int, CTagState> m_tags;
    // Ordered log allows an ID to appear again after a separate encounter.
    std::vector<int> m_log;

    double m_absence_timeout_s; // Encounter time threshold, in seconds.
    int m_min_consecutive_frames; // Encounter frame threshold, in frames.
};

#endif // CENCOUNTER_HPP_
