// MTRX3760 2026 Project 2: Package Deal
// File: CTagState.cpp
// Author(s): Christina Miljak, Davin Pranata, Joshua Samuel David, Joshua Kwok, Kaif Masiudddin Mohammed, Kieran Wong
//
// Implements the per-tag sighting rules used by CEncounter.

#include "CTagState.hpp"


// Remember when the tag was last seen and extend its run of consecutive frames.
void CTagState::observe(double stamp_s)
{
    m_last_seen_s = stamp_s;
    ++m_consecutive_frames;
}

// A missed frame ends the run, so a flicker has to start counting from zero again.
void CTagState::mark_missed()
{
    m_consecutive_frames = 0;
}

// Promote the tag to an encounter once, when its run of frames is long enough.
bool CTagState::confirm_encounter(int min_consecutive_frames)
{
    if (m_in_encounter || m_consecutive_frames < min_consecutive_frames) {
        return false;
    }

    m_in_encounter = true;
    return true;
}

// Unconfirmed tags are dropped at once. Confirmed tags are kept until they have been absent
// for longer than the timeout, so a brief dropout does not start a second encounter.
bool CTagState::is_expired(double stamp_s, double absence_timeout_s) const
{
    const bool timed_out = (stamp_s - m_last_seen_s) > absence_timeout_s;
    return !m_in_encounter || timed_out;
}
