
#pragma once

#include <string>
#include <vector>
#include <variant>
#include <unordered_set>
#include <unordered_map>
#include "../options.hpp"
#include "GameItem.hpp"
#include "Area.hpp"
#include "../options.hpp"


enum struct RequirementType
{
    NONE = 0,
    NOTHING,
    IMPOSSIBLE,
    OR,
    AND,
    NOT,
    HAS_ITEM,
    EVENT,
    COUNT,
    CAN_ACCESS,
    SETTING,
    MACRO,
    INVALID
};

using MacroIndex = size_t;
using EventId = size_t;
struct Requirement;

struct Requirement
{
    using Argument = std::variant<int, size_t, std::string, Requirement, GameItem, Option, Item>;
    RequirementType type = RequirementType::INVALID;
    std::vector<Argument> args;
};

std::string printRequirement(Requirement& req, int nestingLevel = 0);
