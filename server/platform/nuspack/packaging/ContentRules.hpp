#pragma once

#include <string>
#include <vector>

#include "./ContentDetails.hpp"



struct ContentRule {
    std::string pattern;
    ContentDetails details;
    bool contentPerMatch = false; //each match gets new content

    ContentRule(const std::string& pattern_, const ContentDetails& details_, const bool& perMatch_=false) :
        pattern(pattern_),
        details(details_),
        contentPerMatch(perMatch_)
    {}

    bool operator==(const ContentRule& rhs) const {
        return (pattern == rhs.pattern) && (details == rhs.details) && (contentPerMatch == rhs.contentPerMatch);
    }
};

class ContentRules {
public:
    std::vector<ContentRule> rules;

    ContentRule& addRule(const ContentRule& rule);
    ContentRule& createNewRule(const std::string& pattern, const ContentDetails& details, const bool& contentPerMatch = false);
};

ContentRules getCommonRules(const uint16_t& group, const uint64_t& titleID);
