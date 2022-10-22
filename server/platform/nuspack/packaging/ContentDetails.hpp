#pragma once

#include <cstdint>



struct ContentDetails {
    const bool isHashed = false;
    const uint16_t groupID = 0;
    const uint64_t parentID = 0;
    const uint16_t entriesFlag = 0;

    ContentDetails(const bool& hashed_, const uint16_t& groupID_, const uint64_t& parentID_, const uint16_t& entriesFlags_) :
        isHashed(hashed_),
        groupID(groupID_),
        parentID(parentID_),
        entriesFlag(entriesFlags_)
    {}

    bool operator==(const ContentDetails& rhs) const {
        return (isHashed == rhs.isHashed) && (groupID == rhs.groupID) && (parentID == rhs.parentID) && (entriesFlag == rhs.entriesFlag);
    }
};