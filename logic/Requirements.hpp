
#pragma once

#include <string>
#include <vector>
#include <variant>

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
    HAS_ITEM,
    EVENT,
    COUNT,
    HEALTH,
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
using ItemMultiSet = std::unordered_multiset<Item>;
using EventSet = std::unordered_set<EventId>;
struct Requirement;

struct Requirement
{
    using Argument = std::variant<int, size_t, std::string, Requirement, GameItem, Option, Item>;
    RequirementType type = RequirementType::INVALID;
    std::vector<Argument> args;

    void simplifyParenthesis();
    void sortArgs();
    std::unordered_set<GameItem> getItems(World* world);
};

bool evaluateRequirement(World* world, const Requirement& req, const ItemMultiSet* ownedItems, const EventSet* ownedEvents);
std::string printRequirement(const Requirement& req, World* world, int nestingLevel = 0);
RequirementError parseRequirementString(const std::string& str, Requirement& req, World* world);
std::string errorToName(const RequirementError& err);
