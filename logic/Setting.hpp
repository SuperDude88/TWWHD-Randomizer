#pragma once

#include <string>

enum struct Setting
{
    INVALID = 0,
    Swordless,
    SkipTrials
};

Setting nameToSetting(const std::string& name);