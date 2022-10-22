
#include "Requirements.hpp"

#include <unordered_map>
#include <algorithm>
#include <functional>

#include <logic/GameItem.hpp>
#include <logic/PoolFunctions.hpp>

static std::unordered_map<std::string, RequirementType> nameToTypeMap = {
    {"or", RequirementType::OR},
    {"and", RequirementType::AND},
    {"not", RequirementType::NOT},
    {"has_item", RequirementType::HAS_ITEM},
    {"count", RequirementType::COUNT},
    {"can_access", RequirementType::CAN_ACCESS},
    {"setting", RequirementType::SETTING},
    {"macro", RequirementType::MACRO}
};

static std::unordered_map<RequirementType, std::string> typeToNameMap = {
    {RequirementType::OR, "or"},
    {RequirementType::AND, "and"},
    {RequirementType::NOT, "not"},
    {RequirementType::HAS_ITEM, "has_item"},
    {RequirementType::COUNT, "count"},
    {RequirementType::CAN_ACCESS, "can_access"},
    {RequirementType::SETTING, "setting"},
    {RequirementType::MACRO, "macro"},
};

static std::string tabs(int numTabs)
{
    std::string returnStr = "";
    for (int i = 0; i < numTabs; i++)
    {
        returnStr += "\t";
    }
    return returnStr;
}

std::string printRequirement(Requirement& req, int nestingLevel /*= 0*/)
{
    std::string returnStr = "";
    uint32_t expectedCount = 0;
    Item item;
    Requirement nestedReq;
    returnStr += tabs(nestingLevel);
    switch(req.type)
    {
    case RequirementType::OR:
        returnStr += "or\n";
        for (Requirement::Argument& arg : req.args)
        {
            nestedReq = std::get<Requirement>(arg);
            returnStr += printRequirement(nestedReq, nestingLevel + 1);
        }
        return returnStr;
    case RequirementType::AND:
        returnStr += "and\n";
        for (Requirement::Argument& arg : req.args)
        {
            nestedReq = std::get<Requirement>(arg);
            returnStr += printRequirement(nestedReq, nestingLevel + 1);
        }
        return returnStr;
    case RequirementType::NOT:
        returnStr += "not\n";
        returnStr += printRequirement(std::get<Requirement>(req.args[0]), nestingLevel + 1);
        return returnStr;
    case RequirementType::HAS_ITEM:
        item = std::get<Item>(req.args[0]);
        returnStr += item.getName() + "\n";
        return returnStr;
    case RequirementType::COUNT:
        returnStr += "count: ";
        expectedCount = std::get<int>(req.args[0]);
        item = std::get<Item>(req.args[1]);
        returnStr += std::to_string(expectedCount) + " " + item.getName() + "\n";
        return returnStr;
    case RequirementType::CAN_ACCESS:
        returnStr += "can_access: " + std::get<std::string>(req.args[0]) + "\n";
        return returnStr;
    case RequirementType::SETTING:
        // Settings are resolved to a true/false value when building the world
        returnStr += "setting: " + std::to_string(std::get<int>(req.args[0])) + "\n";
        return returnStr;
    case RequirementType::MACRO:
        returnStr += "macro: " + std::to_string(std::get<MacroIndex>(req.args[0])) + "\n";
    default:
        return returnStr;
    }
}
