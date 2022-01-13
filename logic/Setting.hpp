#pragma once

#include <string>
#include <unordered_set>

enum struct Setting
{
    INVALID = 0,
    Swordless,
    SkipTrials
};

// right now assuming all settings are boolean (will change later)
using Settings = std::unordered_set<Setting>;

Setting nameToSetting(const std::string& name);
