
#pragma once

#include <string>
#include <vector>
#include <variant>
#include <unordered_set>
#include <unordered_map>

#include <options.hpp>
#include <logic/GameItem.hpp>

class World;
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

enum struct RequirementError
{
    NONE = 0,
    EXTRA_OR_MISSING_PARENTHESIS,
    LOGIC_SYMBOL_DOES_NOT_EXIST,
    SAME_NESTING_LEVEL,
    COULD_NOT_DETERMINE_TYPE,
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
RequirementError parseRequirementString(const std::string& str, Requirement& req, World* world);
std::string errorToName(const RequirementError& err);
