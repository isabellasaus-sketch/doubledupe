// MTRX3760 2026 Project 2: Package Deal
// File: CTagState.hpp
// Author(s): Christina Miljak, Davin Pranata, Joshua Samuel David, Joshua Kwok, Kaif Masiudddin Mohammed, Kieran Wong
//
// Declares CTagState, which tracks one tag ID across frames and decides when
// it has been seen enough times to count as an encounter, and when that
// encounter has ended. Has no ROS dependency.

#ifndef CTAGSTATE_HPP_
#define CTAGSTATE_HPP_

// The sighting history of a single tag ID, owned by CEncounter.
class CTagState {
public:
    // Record that the tag appeared in a frame captured at stamp_s (seconds).
    void observe(double stamp_s);

    // Record that the tag was missing from a frame, which breaks its run of consecutive frames.
    void mark_missed();

    // True only the first time the tag has been seen in min_consecutive_frames frames in a row.
    // Once it has returned true the tag stays in its encounter and this returns false.
    bool confirm_encounter(int min_consecutive_frames);

    // True when this record can be dropped at stamp_s: the tag never reached an encounter,
    // or its encounter has ended because it was out of view for longer than absence_timeout_s.
    bool is_expired(double stamp_s, double absence_timeout_s) const;

private:
    double m_last_seen_s = 0.0;    // Last time this tag was seen.
    int m_consecutive_frames = 0;  // Number of consecutive frames this tag has been seen in.
    bool m_in_encounter = false;   // Whether this tag is currently in an encounter.
};

#endif // CTAGSTATE_HPP_
