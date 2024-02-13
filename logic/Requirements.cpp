
#include "Requirements.hpp"

#include <logic/GameItem.hpp>
#include <logic/PoolFunctions.hpp>
#include <logic/World.hpp>
#include <command/Log.hpp>

#include <unordered_map>
#include <algorithm>
#include <functional>



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
        [[fallthrough]];
    default:
        return returnStr;
    }
}

// Takes a logic expression string and stores it as a requirement within the passed in Requirement
// object. This means we only have to parse the string once and then evaluating it many times
// later is a lot faster. An example of a logic expression string is: "Grappling_Hook and (Deku_Leaf or Hookshot)"
RequirementError parseRequirementString(const std::string& str, Requirement& req, World* world)
{
    RequirementError err;
    std::string logicStr (str);

    // First, we make sure that the expression has no missing or extra parenthesis
    // and that the nesting level at the beginning is the same at the end.
    //
    // Logic expressions are split up via spaces, but we only want to evaluate the parts of
    // the expression at the highest nesting level for the string that was passed in.
    // (We'll recursively call the function later to evaluate deeper levels.) So we replace
    // all the spaces on the highest nesting level with an arbitrarily chosen delimeter
    // (in this case: '+').
    int nestingLevel = 1;
    const char delimeter = '+';
    for (auto& ch : logicStr)
    {
        if (ch == '(')
        {
            nestingLevel++;
        }
        else if (ch == ')')
        {
            nestingLevel--;
        }

        if (nestingLevel == 1 && ch == ' ')
        {
            ch = delimeter;
        }
    }

    // If the nesting level isn't the same as what we started with, then the logic
    // expression is invalid.
    if (nestingLevel != 1)
    {
        ErrorLog::getInstance().log("Extra or missing parenthesis within expression: \"" + str + "\"");
        return RequirementError::EXTRA_OR_MISSING_PARENTHESIS;
    }

    // Next we split up the expression by the delimeter in the previous step
    size_t pos = 0;
    std::vector<std::string> splitLogicStr = {};
    while ((pos = logicStr.find(delimeter)) != std::string::npos)
    {
        // When parsing setting checks, take the entire expression
        // and the three components individually
        auto& chBefore = logicStr[pos-1];
        auto& chAfter = logicStr[pos+1];
        if (chBefore != '!' && chAfter != '!' && chBefore != '=' && chAfter != '=')
        {
            splitLogicStr.push_back(logicStr.substr(0, pos));
            logicStr.erase(0, pos + 1);
        }
        else
        {
            logicStr.erase(logicStr.begin() + pos);
        }
    }
    splitLogicStr.push_back(logicStr);

    // Once we have the different parts of our expression, we can use the number
    // of parts we have to determine what kind of expression it is.

    // If we only have one part, then we have either an event, an item, a macro,
    // a can_access check, a setting, or a count
    if (splitLogicStr.size() == 1)
    {

        std::string argStr = splitLogicStr[0];
        std::replace(argStr.begin(), argStr.end(), '_', ' ');
        // First, see if we have nothing
        if (argStr == "Nothing")
        {
            req.type = RequirementType::NOTHING;
            return RequirementError::NONE;
        }

        // Then an event...
        if (argStr[0] == '\'')
        {
            req.type = RequirementType::EVENT;
            std::string eventName (argStr.begin() + 1, argStr.end() - 1); // Remove quotes
            world->addEvent(eventName);

            EventId eventId = world->eventMap[eventName];

            req.args.push_back(eventId);
            return RequirementError::NONE;
        }

        // Then a macro...
        if (world->macroNameMap.contains(argStr))
        {
            req.type = RequirementType::MACRO;
            req.args.push_back(world->macroNameMap.at(argStr));
            return RequirementError::NONE;
        }
        // Then an item...
        else if (nameToGameItem(argStr) != GameItem::INVALID)
        {
            req.type = RequirementType::HAS_ITEM;
            req.args.push_back(world->getItem(argStr));
            return RequirementError::NONE;
        }
        // Then a can_access check...
        else if (argStr.find("can access") != std::string::npos)
        {
            req.type = RequirementType::CAN_ACCESS;
            std::string areaName (argStr.begin() + argStr.find('(') + 1, argStr.end() - 1);
            auto area = world->getArea(areaName);
            req.args.push_back(area->name);
            return RequirementError::NONE;
        }
        // Then a boolean setting...
        else if (world->getSettings().evaluateOption(argStr) != -1)
        {
            req.type = RequirementType::SETTING;
            req.args.push_back(world->getSettings().evaluateOption(argStr));
            return RequirementError::NONE;
        }
        // Then a setting that has more than just an on/off option...
        else if (argStr.find("!=") != std::string::npos || argStr.find("==") != std::string::npos)
        {
            req.type = RequirementType::SETTING;
            bool equalComparison = argStr.find("==") != std::string::npos;

            // Split up the comparison using the second comparison character (which will always be '=')
            auto compPos = argStr.rfind('=');
            std::string comparedOptionStr (argStr.begin() + (compPos + 1), argStr.end());
            std::string settingName (argStr.begin(), argStr.begin() + (compPos - 1));

            int comparedOption = nameToSettingInt(comparedOptionStr);
            Option setting = nameToSetting(settingName);
            int actualOption = world->getSettings().getSetting(setting);

            // If the comparison is true
            if ((equalComparison && actualOption == comparedOption) || (!equalComparison && actualOption != comparedOption))
            {
                req.args.push_back(true);
            }
            else
            {
                req.args.push_back(false);
            }
            return RequirementError::NONE;
        }
        // And finally a count...
        else if (argStr.find("count") != std::string::npos)
        {
            req.type = RequirementType::COUNT;
            // Since a count has two arguments (a number and an item), we have
            // to split up the string in the parenthesis into those arguments.

            // Get rid of parenthesis
            std::string countArgs (argStr.begin() + argStr.find('(') + 1, argStr.end() - 1);
            // Erase any spaces
            // countArgs.erase(std::remove(countArgs.begin(), countArgs.end(), ' '), countArgs.end());

            // Split up the arguments
            pos = 0;
            splitLogicStr = {};
            while ((pos = countArgs.find(", ")) != std::string::npos)
            {
                splitLogicStr.push_back(countArgs.substr(0, pos));
                countArgs.erase(0, pos + 2);
            }
            splitLogicStr.push_back(countArgs);

            // Get the arguments
            int count = std::stoi(splitLogicStr[0]);
            std::string itemName = splitLogicStr[1];
            req.args.push_back(count);
            req.args.push_back(world->getItem(itemName));
            return RequirementError::NONE;
        }

        // Check Impossible last since it's least likely
        else if (argStr == "Impossible")
        {
            req.type = RequirementType::IMPOSSIBLE;
            return RequirementError::NONE;
        }

        ErrorLog::getInstance().log("Unrecognized logic symbol: \"" + argStr + "\"");
        return RequirementError::LOGIC_SYMBOL_DOES_NOT_EXIST;
    }

    // If our expression has two parts, then the only type of requirement
    // that can currently be is a "not" requirement
    if (splitLogicStr.size() == 2)
    {
        if (splitLogicStr[0] == "not")
        {
            req.type = RequirementType::NOT;
            req.args.emplace_back(Requirement());
            // The second part of the not expression is another expression
            // so we have to evaluate that one as well.
            auto reqStr = splitLogicStr[1];
            // Get rid of parenthesis around the expression if it has them
            if (reqStr[0] == '(')
            {
                reqStr = reqStr.substr(1, reqStr.length() - 2);
            }
            // Evaluate the deeper expression and add it to the requirement object if it's valid
            if ((err = parseRequirementString(reqStr, std::get<Requirement>(req.args.back()), world)) != RequirementError::NONE) return err;
        }
        else
        {
            ErrorLog::getInstance().log("Unrecognized 2 part expression: " + str);
            return RequirementError::LOGIC_SYMBOL_DOES_NOT_EXIST;
        }
    }

    // If we have more than two parts to our expression, then we have either "and"
    // or "or".
    bool andType = elementInPool("and", splitLogicStr);
    bool orType = elementInPool("or", splitLogicStr);

    // If we have both of them, there's a problem with the logic expression
    if (andType && orType)
    {
        ErrorLog::getInstance().log("\"and\" & \"or\" in same nesting level when parsing \"" + str + "\"");
        return RequirementError::SAME_NESTING_LEVEL;
    }

    if (andType || orType)
    {
        // Set the appropriate type
        if (andType)
        {
            req.type = RequirementType::AND;
        }
        else if (orType)
        {
            req.type = RequirementType::OR;
        }

        // Once we know the type, we can erase the "and"s or "or"s and are left with just the deeper
        // expressions to be logically operated on.
        filterAndEraseFromPool(splitLogicStr, [](const std::string& arg){return arg == "and" || arg == "or";});

        // If we have any deeper "not" expressions, we have to recombine them here since they got separated
        // by the delimeter earlier
        for (auto itr = splitLogicStr.begin(); itr != splitLogicStr.end(); itr++)
        {
            if (*itr == "not")
            {
                *itr = *itr + " " + *(itr + 1);
                splitLogicStr.erase(itr + 1);
            }
        }

        // For each deeper expression, parse it and add it as an argument to the
        // Requirement
        for (auto& reqStr : splitLogicStr)
        {
            req.args.emplace_back(Requirement());
            // Get rid of parenthesis surrounding each deeper expression
            if (reqStr[0] == '(')
            {
                reqStr = reqStr.substr(1, reqStr.length() - 2);
            }
            if ((err = parseRequirementString(reqStr, std::get<Requirement>(req.args.back()), world)) != RequirementError::NONE) return err;
        }
    }


    if (req.type != RequirementType::NONE)
    {
        return RequirementError::NONE;
    }
    else
    // If we've reached this point, we weren't able to determine a logical operator within the expression
    {
        ErrorLog::getInstance().log("Could not determine logical operator type from expression: \"" + str + "\"");
        return RequirementError::COULD_NOT_DETERMINE_TYPE;
    }
}

std::string errorToName(const RequirementError& err)
{
    switch (err)
    {
        case RequirementError::NONE:
            return "NONE";
        case RequirementError::EXTRA_OR_MISSING_PARENTHESIS:
            return "EXTRA_OR_MISSING_PARENTHESIS";
        case RequirementError::LOGIC_SYMBOL_DOES_NOT_EXIST:
            return "LOGIC_SYMBOL_DOES_NOT_EXIST";
        case RequirementError::SAME_NESTING_LEVEL:
            return "SAME_NESTING_LEVEL";
        case RequirementError::COULD_NOT_DETERMINE_TYPE:
            return "COULD_NOT_DETERMINE_TYPE";
        default:
            return "UNKNOWN";
    }
}
