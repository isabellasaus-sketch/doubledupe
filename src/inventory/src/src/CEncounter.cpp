// MTRX3760 2026 Project 2: Package Deal
// File: CEncounter.cpp
// Author(s): Christina Miljak, Davin Pranata, Joshua Samuel David, Joshua Kwok, Kaif Masiudddin Mohammed, Kieran Wong
//
// Implements the state machine for detecting and logging distinct marker encounters.

#include "CEncounter.hpp"


// Store the timeout and require at least one frame for confirmation.
CEncounter::CEncounter(double absence_timeout_s, int min_consecutive_frames)
    : m_absence_timeout_s(absence_timeout_s),
      m_min_consecutive_frames(std::max(1, min_consecutive_frames))
{
}


// Update visible and absent tags; return only encounters newly confirmed now.
std::vector<int> CEncounter::update(const std::vector<int> & visible_ids, double stamp_s)
{
    std::vector<int> new_encounters;

    // A set removes duplicate IDs so one tag can't count twice in a single frame.
    const std::set<int> visible(visible_ids.begin(), visible_ids.end());

    // Tags from this frame: update state and check for new encounters.
    for (const int id : visible) {
        CTagState & tag = m_tags[id];  // Default constructed the first time an ID is seen.
        tag.observe(stamp_s);

        // If this tag has been seen enough times in a row, it is now an encounter.
        if (tag.confirm_encounter(m_min_consecutive_frames)) {
            m_log.push_back(id);
            new_encounters.push_back(id);
        }
    }

    // Tags not seen in this frame: remove them from state if timed out.
    // Confirmed tags dropped when timed out.
    std::vector<int> expired_ids;

    for (std::pair<const int, CTagState> & entry : m_tags) {
        if (visible.count(entry.first) != 0) {
            continue;
        }

        CTagState & tag = entry.second;
        tag.mark_missed();

        if (tag.is_expired(stamp_s, m_absence_timeout_s)) {
            expired_ids.push_back(entry.first);
        }
    }

    // Remove ended or unconfirmed observations so a later sighting can start afresh.
    for (const int id : expired_ids) {
        m_tags.erase(id);
    }

    return new_encounters;
}

// Expose the encounter history without copying or modifying the stored list.
const std::vector<int> & CEncounter::get_log() const
{
    return m_log;
}

// Clear all tracked encounters and their history.
void CEncounter::reset()
{
    m_tags.clear();
    m_log.clear();
}
