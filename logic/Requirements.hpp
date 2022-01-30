
#pragma once

#include <string>
#include <vector>
#include <variant>
#include <unordered_set>
#include <unordered_map>
#include "../libs/jsonexcept.hpp"
#include "../libs/json.hpp"
#include "../options.hpp"
#include "GameItem.hpp"
#include "Area.hpp"

enum struct RequirementType
{
    NONE = 0,
    OR,
    AND,
    NOT,
    HAS_ITEM,
    COUNT,
    CAN_ACCESS,
    SETTING,
    MACRO,
    INVALID
};

using json = nlohmann::json;
using MacroIndex = size_t;
struct Requirement;

struct Requirement
{
    using Argument = std::variant<int, std::string, Requirement, GameItem, Area, Option, MacroIndex>;
    RequirementType type = RequirementType::INVALID;
    std::vector<Argument> args;
};
