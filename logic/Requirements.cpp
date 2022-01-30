
#include "Requirements.hpp"
#include "GameItem.hpp"
#include <unordered_map>
#include <algorithm>
#include <functional>

// RequirementParser::RequirementParser()
// {

// }

// bool RequirementParser::parseElement(RequirementType type, const std::vector<json>& args, std::vector<Requirement::Argument>& out) const
// {
//     Requirement reqOut;
//     std::string argStr;
//     GameItem argItem = GameItem::INVALID;
//     Location argLoc = Location::INVALID;
//     int countValue = 0;
//     if (args.size() == 0) return false; // all ops require at least one arg
//     switch(type)
//     {
//     case RequirementType::OR: // and/or expects list of sub-requirements
//     case RequirementType::AND:
//         for (const auto& arg : args) {
//             if (!arg.is_object()) return false;
//             if (!parseRequirement(arg, reqOut)) return false;
//             out.push_back(reqOut);
//         }
//         return true;
//     case RequirementType::NOT:
//         if (!args[0].is_object()) return false;
//         if (!parseRequirement(args[0], reqOut)) return false;
//         out.push_back(reqOut);
//         break;
//     case RequirementType::HAS_ITEM:
//         // has item expects a single string arg that maps to an item
//         if (!args[0].is_string()) return false;
//         argStr = args[0].get<std::string>();
//         argItem = nameToGameItem(argStr);
//         if (argItem == GameItem::INVALID) return false;
//         out.push_back(argItem);
//         return true;
//     case RequirementType::COUNT:
//         // count expects a value count and a string mapping to an item
//         if (args.size() != 2) return false;
//         if (!args[0].is_number()) return false;
//         countValue = args[0].get<int>();
//         if (!args[1].is_string()) return false;
//         argStr = args[1].get<std::string>();
//         argItem = nameToGameItem(argStr);
//         if (argItem == GameItem::INVALID) return false;
//         out.push_back(countValue);
//         out.push_back(argItem);
//         return true;
//     case RequirementType::CAN_ACCESS:
//         if(!args[0].is_string()) return false;
//         argLoc = nameToLocationId(args[0].get<std::string>());
//         if (argLoc == Location::INVALID) return false;
//         out.push_back(argLoc);
//         return true;
//     case RequirementType::SETTING:
//         // setting just expects a settings name
//         // TODO: make setting an enum
//         if (!args[0].is_string()) return false;
//         argStr = args[0].get<std::string>();
//         out.push_back(argStr);
//         return true;
//     case RequirementType::MACRO:
//         // macro just stores index into macro vector
//         if (!args[0].is_string()) return false;
//         argStr = args[0].get<std::string>();
//         if (macroNameMap.count(argStr) == 0) return false; // macro does not exist
//         out.push_back(macroNameMap.at(argStr));
//         return true;
//     case RequirementType::NONE:
//     default:
//         return false;
//     }
//     return true;
// }

// bool RequirementParser::parseRequirement(const json &requirementsObject, Requirement& out) const
// {
//     static std::unordered_map<std::string, RequirementType> typeMap = {
//         {"or", RequirementType::OR},
//         {"and", RequirementType::AND},
//         {"not", RequirementType::NOT},
//         {"has_item", RequirementType::HAS_ITEM},
//         {"count", RequirementType::COUNT},
//         {"can_access", RequirementType::CAN_ACCESS},
//         {"setting", RequirementType::SETTING},
//         {"macro", RequirementType::MACRO}
//     };
//     if (!requirementsObject.is_object())
//     {
//         return false;
//     }
//     if (!requirementsObject.contains("type"))
//     {
//         return false;
//     }
//     if (!requirementsObject.contains("args") || !requirementsObject["args"].is_array())
//     {
//         return false;
//     }
//     if(typeMap.count(requirementsObject["type"].get<std::string>()) == 0)
//     {
//         return false;
//     }
//     out.type = typeMap[requirementsObject["type"].get<std::string>()];
//     if (!parseElement(out.type, requirementsObject["args"].get<std::vector<json>>(), out.args))
//     {
//         return false;
//     }
//     return true;
// }

// bool RequirementParser::parseMacro(const json& macroObject, std::string& nameOut, Requirement& reqOut)
// {
//     if (!macroObject.is_object()) return false;
//     // n.b. current macros.json has capital letter first
//     if (!macroObject.contains("name")) return false;
//     if (!macroObject.contains("expression")) return false;
//     if (!macroObject["name"].is_string()) return false;
//     nameOut = macroObject["name"].get<std::string>();
//     if (!macroObject["expression"].is_object()) return false;
//     if (!parseRequirement(macroObject["expression"], reqOut)) return false;
//     return true;
// }

// bool RequirementParser::addMacro(const json &macroObject)
// {
//     std::string name;
//     Requirement req;
//     if (!parseMacro(macroObject, name, req)) return false;
//     if (macroNameMap.count(name) > 0) return false; // duplicate macro name
//     macros.push_back(req);
//     macroNameMap.emplace(name, macros.size() - 1);
//     return true;
// }
// // TODO: move this to an "Evaluator" class to avoid passing so many reference args?
// // Evaluator would have the locations, macros, and requirements in vectors and store indices
// // to avoid string mapping too
// bool RequirementParser::evaluate(const Requirement& requirement,
//                                  const std::unordered_map<std::string, std::reference_wrapper<Requirement>>& locReqMap,
//                                  const std::unordered_map<GameItem, uint32_t>& ownedItems,
//                                  const std::unordered_set<std::string>& settings)
// {
//     uint32_t expectedCount = 0;
//     GameItem item = GameItem::INVALID;
//     switch(requirement.type)
//     {
//     case RequirementType::OR:
//         return std::any_of(
//             requirement.args.begin(),
//             requirement.args.end(),
//             [&](const Requirement::Argument& arg){
//                 return evaluate(std::get<Requirement>(arg), locReqMap, ownedItems, settings);
//             }
//         );
//     case RequirementType::AND:
//         return std::all_of(
//             requirement.args.begin(),
//             requirement.args.end(),
//             [&](const Requirement::Argument& arg){
//                 return evaluate(std::get<Requirement>(arg), locReqMap, ownedItems, settings);
//             }
//         );
//     case RequirementType::NOT:
//         return !evaluate(std::get<Requirement>(requirement.args[0]), locReqMap, ownedItems, settings);
//     case RequirementType::HAS_ITEM:
//         // we can expect ownedItems will contain entires for every item type
//         return ownedItems.at(std::get<GameItem>(requirement.args[0])) > 0;
//     case RequirementType::COUNT:
//         expectedCount = std::get<int>(requirement.args[0]);
//         return ownedItems.at(std::get<GameItem>(requirement.args[1])) > expectedCount;
//     case RequirementType::CAN_ACCESS:
//         // TODO: check locName exists here?
//         return evaluate(locReqMap.at(std::get<std::string>(requirement.args[0])), locReqMap, ownedItems, settings);
//     case RequirementType::SETTING:
//         return settings.count(std::get<std::string>(requirement.args[0]));
//     case RequirementType::MACRO:
//         return evaluate(macros[std::get<MacroIndex>(requirement.args[0])], locReqMap, ownedItems, settings);
//     case RequirementType::NONE:
//     default:
//         // actually needs to be some error state?
//         return false;
//     }
//     return false;
// }
