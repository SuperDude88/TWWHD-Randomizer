
#include "World.hpp"
#include "Requirements.hpp"
#include "Setting.hpp"
#include "PoolFunctions.hpp"
#include <algorithm>
#include <cstdlib>
#include <climits>
#include <vector>
#include <random>
#include <fstream>
#include <iostream>

// some error checking macros for brevity and since we can't use exceptions
#define FIELD_CHECK(j, field, err) if(!j.contains(field)) {lastError << "Unable to retrieve field: \"" << field << '"'; return err;}
#define OBJECT_CHECK(j, msg) if(!j.is_object()) {lastError << msg << ": Not an Object."; return WorldLoadingError::EXPECTED_JSON_OBJECT;}
#define MAPPING_CHECK(str1, str2) if (str1 != str2) {lastError << str1 << " does not equal " << str2; return WorldLoadingError::MAPPING_MISMATCH;}
#define VALID_CHECK(e, invalid, msg, err) if(e == invalid) {lastError << "\t" << msg; return err;}
#define ITEM_VALID_CHECK(item, msg) VALID_CHECK(item, GameItem::INVALID, msg, WorldLoadingError::GAME_ITEM_DOES_NOT_EXIST)
#define AREA_VALID_CHECK(area, msg) VALID_CHECK(area, Area::INVALID, msg, WorldLoadingError::AREA_DOES_NOT_EXIST)
#define LOCATION_VALID_CHECK(loc, msg) VALID_CHECK(loc, Location::INVALID, msg, WorldLoadingError::LOCATION_DOES_NOT_EXIST)
#define SETTING_VALID_CHECK(set, msg) VALID_CHECK(set, Setting::INVALID, msg, WorldLoadingError::SETTING_DOES_NOT_EXIST)


World::World()
{

}

World World::copy()
{
    World newWorld = World();
    newWorld.locationEntries = this->locationEntries;
    newWorld.areaEntries = this->areaEntries;
    newWorld.macros = this->macros;
    newWorld.macroNameMap = this->macroNameMap;
    newWorld.worldId = this->worldId;
    newWorld.settings = this->settings;
    newWorld.itemPool = this->itemPool;
    return newWorld;
}

// Somehow set different settings for different worlds
void World::setSettings(Settings& settings_)
{
    settings = settings_;
}

// Generate the item pool for this specific world
void World::setItemPool()
{
    itemPool = generateItemPool(settings, worldId);
}

World::WorldLoadingError World::parseElement(RequirementType type, const std::vector<json>& args, std::vector<Requirement::Argument>& out)
{
    std::string argStr;
    GameItem argItem = GameItem::INVALID;
    Location argLoc = Location::INVALID;
    Setting argSetting = Setting::INVALID;
    int countValue = 0;
    WorldLoadingError err = WorldLoadingError::NONE;
    if (args.size() == 0) return WorldLoadingError::INCORRECT_ARG_COUNT; // all ops require at least one arg
    switch(type)
    {
    case RequirementType::OR: // and/or expects list of sub-requirements
    case RequirementType::AND:
        for (const auto& arg : args) {
            out.emplace_back(Requirement());
            OBJECT_CHECK(arg, "Requirement element of type " << static_cast<int>(type) << " has unexpected type");
            if ((err = parseRequirement(arg, std::get<Requirement>(out.back()))) != WorldLoadingError::NONE) return err;
        }
        return WorldLoadingError::NONE;
    case RequirementType::NOT:
        out.emplace_back(Requirement());
        OBJECT_CHECK(args[0], "Requirement element of type " << static_cast<int>(type) << " has unexpected type");
        if ((err = parseRequirement(args[0], std::get<Requirement>(out.back()))) != WorldLoadingError::NONE) return err;
        return WorldLoadingError::NONE;
    case RequirementType::HAS_ITEM:
        // has item expects a single string arg that maps to an item
        argStr = args[0].get<std::string>();
        argItem = nameToGameItem(argStr);
        ITEM_VALID_CHECK(argItem, "Game Item of name \"" << argStr << " Does Not Exist");
        out.push_back(argItem);
        return WorldLoadingError::NONE;
    case RequirementType::COUNT:
        // count expects a value count and a string mapping to an item
        if (args.size() != 2) return WorldLoadingError::INCORRECT_ARG_COUNT;
        countValue = args[0].get<int>();
        argStr = args[1].get<std::string>();
        argItem = nameToGameItem(argStr);
        ITEM_VALID_CHECK(argItem, "Game Item of name \"" << argStr << " Does Not Exist");
        out.push_back(countValue);
        out.push_back(argItem);
        return WorldLoadingError::NONE;
    case RequirementType::CAN_ACCESS:
        argLoc = nameToLocation(args[0].get<std::string>());
        LOCATION_VALID_CHECK(argLoc, "Location " << args[0].get<std::string>() << "does not exist");
        out.push_back(argLoc);
        return WorldLoadingError::NONE;
    case RequirementType::SETTING:
        // setting just expects a settings name
        argStr = args[0].get<std::string>();
        argSetting = nameToSetting(argStr);
        SETTING_VALID_CHECK(argSetting, "Setting " << argStr << " Does Not Exist.");
        out.push_back(argSetting);
        return WorldLoadingError::NONE;
    case RequirementType::MACRO:
        // macro just stores index into macro vector
        argStr = args[0].get<std::string>();
        if (macroNameMap.count(argStr) == 0)
        {
            lastError << "Macro of name " << argStr << " Does Not Exist.";
            return WorldLoadingError::MACRO_DOES_NOT_EXIST;
        }
        out.push_back(macroNameMap.at(argStr));
        return WorldLoadingError::NONE;
    case RequirementType::NONE:
    default:
        return WorldLoadingError::REQUIREMENT_TYPE_DOES_NOT_EXIST;
    }
    return WorldLoadingError::REQUIREMENT_TYPE_DOES_NOT_EXIST;
}

World::WorldLoadingError World::parseRequirement(const json &requirementsObject, Requirement& out)
{
    static std::unordered_map<std::string, RequirementType> typeMap = {
        {"or", RequirementType::OR},
        {"and", RequirementType::AND},
        {"not", RequirementType::NOT},
        {"has_item", RequirementType::HAS_ITEM},
        {"count", RequirementType::COUNT},
        {"can_access", RequirementType::CAN_ACCESS},
        {"setting", RequirementType::SETTING},
        {"macro", RequirementType::MACRO}
    };

    WorldLoadingError err = WorldLoadingError::NONE;

    FIELD_CHECK(requirementsObject, "type", WorldLoadingError::REQUIREMENT_MISISNG_KEY);
    const std::string& typeStr = requirementsObject.at("type").get<std::string>();
    if(typeMap.count(typeStr) == 0)
    {
        lastError << "Requirement Type " << typeStr << " Does Not Exist.";
        return WorldLoadingError::REQUIREMENT_TYPE_DOES_NOT_EXIST;
    }
    out.type = typeMap[typeStr];
    FIELD_CHECK(requirementsObject, "args", WorldLoadingError::REQUIREMENT_MISISNG_KEY);
    const auto& args = requirementsObject.at("args").get<std::vector<json>>();
    if ((err = parseElement(out.type, args, out.args)) != WorldLoadingError::NONE)
    {
        return err;
    }
    return WorldLoadingError::NONE;
}

World::WorldLoadingError World::parseMacro(const json& macroObject, Requirement& reqOut)
{

    WorldLoadingError err = WorldLoadingError::NONE;
    // readd prechecks?
    FIELD_CHECK(macroObject, "Expression", WorldLoadingError::MACRO_MISSING_KEY);
    if ((err = parseRequirement(macroObject.at("Expression"), reqOut)) != WorldLoadingError::NONE) return err;
    return WorldLoadingError::NONE;
}

World::WorldLoadingError World::loadMacros(const std::vector<json>& macroObjectList)
{
    std::string name;
    WorldLoadingError err = WorldLoadingError::NONE;
    uint32_t macroCount = 0;

    // first pass to get all macro names
    for (const auto& macroObject : macroObjectList)
    {
        FIELD_CHECK(macroObject, "Name", WorldLoadingError::MACRO_MISSING_KEY);
        name = macroObject.at("Name").get<std::string>();
        if (macroNameMap.count(name) > 0)
        {
            lastError << "Macro of name " << name << " has already been added";
            return WorldLoadingError::DUPLICATE_MACRO_NAME;
        }
        macroNameMap.emplace(name, macroCount++);
    }
    for (const auto& macroObject : macroObjectList)
    {
        macros.emplace_back();
        if ((err = parseMacro(macroObject, macros.back())) != WorldLoadingError::NONE)
        {
            lastError << " | Encountered parsing macro of name " << macroObject.at("Name").get<std::string>();
            return err;
        }
    }
    return WorldLoadingError::NONE;
}

World::WorldLoadingError World::loadLocation(const json& locationObject, Location& loadedLocation)
{
    // failure indicated by INVALID type for category
    // maybe change to Optional later if thats determined to work
    // on wii u
    OBJECT_CHECK(locationObject, locationObject.dump());
    FIELD_CHECK(locationObject, "Name", WorldLoadingError::LOCATION_MISSING_KEY);
    std::string locationName = locationObject.at("Name").get<std::string>();
    loadedLocation = nameToLocation(locationName);
    LOCATION_VALID_CHECK(loadedLocation, "Location of name \"" << locationName << "\" does not exist!");
    LocationEntry& newEntry = locationEntries[locationAsIndex(loadedLocation)];
    newEntry.location = loadedLocation;
    WorldLoadingError err = WorldLoadingError::NONE;
    FIELD_CHECK(locationObject, "Category", WorldLoadingError::LOCATION_MISSING_KEY);
    const auto& categories = locationObject.at("Category").get<std::vector<json>>();
    for (const auto& categoryName : categories)
    {
        const std::string& categoryNameStr = categoryName.get<std::string>();
        const auto& cat = nameToLocationCategory(categoryNameStr);
        if (cat == LocationCategory::INVALID)
        {
            lastError << "Encountered unknown location category " << categoryNameStr;
            lastError << " while parsing location " << locationName;
            return WorldLoadingError::INVALID_LOCATION_CATEGORY;
        }
        newEntry.categories.insert(cat);
    }
    FIELD_CHECK(locationObject, "Needs", WorldLoadingError::LOCATION_MISSING_KEY);
    if((err = parseRequirement(locationObject.at("Needs"), newEntry.requirement)) != WorldLoadingError::NONE)
    {
        lastError << "| Encountered parsing location " << locationName;
        return err;
    }
    FIELD_CHECK(locationObject, "Path", WorldLoadingError::LOCATION_MISSING_KEY);
    newEntry.method.filePath = locationObject.at("Path").get<std::string>();
    FIELD_CHECK(locationObject, "Type", WorldLoadingError::LOCATION_MISSING_KEY);
    const std::string& modificationType = locationObject.at("Type").get<std::string>();
    newEntry.method.type = nameToModificationType(modificationType);
    if (newEntry.method.type == LocationModificationType::INVALID)
    {
        lastError << "Error processing location " << locationName << ": ";
        lastError << "Modificaiton Type \"" << modificationType << "\" Does Not Exist";
        return WorldLoadingError::INVALID_MODIFICATION_TYPE;
    }
    FIELD_CHECK(locationObject, "Offsets", WorldLoadingError::LOCATION_MISSING_KEY);
    const auto& offsets = locationObject.at("Offsets").get<std::vector<json>>();
    for (const auto& offset_j : offsets)
    {
        if (offset_j.is_number())
        {
            newEntry.method.offsets.push_back(offset_j.get<uint32_t>());
        }
        else
        {
            unsigned long offsetValue = std::strtoul(offset_j.get<std::string>().c_str(), nullptr, 16);
            if (offsetValue == 0 || offsetValue == ULONG_MAX)
            {
                lastError << "Encountered an invalid offset for location " << locationName;
                return WorldLoadingError::INVALID_OFFSET_VALUE;
            }
            newEntry.method.offsets.push_back(offsetValue);
        }
    }
    FIELD_CHECK(locationObject, "OriginalItem", WorldLoadingError::LOCATION_MISSING_KEY);
    const std::string& itemName = locationObject.at("OriginalItem").get<std::string>();
    newEntry.originalItem = nameToGameItem(itemName);
    ITEM_VALID_CHECK(
        newEntry.originalItem,
        "Error processing location " << locationName << ": Item of name " << itemName << " Does Not Exist."
    );
    return WorldLoadingError::NONE;
}

World::WorldLoadingError World::loadExit(const json& exitObject, Exit& loadedExit, Area& parentArea)
{
    // failure indicated by INVALID type for category
    // maybe change to Optional later if thats determined to work
    // on wii u
    OBJECT_CHECK(exitObject, exitObject.dump());
    FIELD_CHECK(exitObject, "ConnectedArea", WorldLoadingError::EXIT_MISSING_KEY);
    std::string connectedAreaName = exitObject.at("ConnectedArea").get<std::string>();
    auto connectedArea = nameToArea(connectedAreaName);
    AREA_VALID_CHECK(connectedArea, "Connected area of name \"" << connectedAreaName << "\" does not exist!");
    loadedExit.parentArea = parentArea;
    loadedExit.connectedArea = connectedArea;
    WorldLoadingError err = WorldLoadingError::NONE;
    // load exit requirements
    FIELD_CHECK(exitObject, "Needs", WorldLoadingError::EXIT_MISSING_KEY);
    if((err = parseRequirement(exitObject.at("Needs"), loadedExit.requirement)) != WorldLoadingError::NONE)
    {
        lastError << "| Encountered parsing exit " << areaToName(parentArea) << " -> " << areaToName(connectedArea) << std::endl;
        return err;
    }
    return WorldLoadingError::NONE;
}

World::WorldLoadingError World::loadArea(const json& areaObject, Area& loadedArea)
{
    // failure indicated by INVALID type for category
    // maybe change to Optional later if thats determined to work
    // on wii u
    OBJECT_CHECK(areaObject, areaObject.dump());
    FIELD_CHECK(areaObject, "Name", WorldLoadingError::AREA_MISSING_KEY);
    std::string areaName = areaObject.at("Name").get<std::string>();

    #ifdef ENABLE_DEBUG
         // std::cout << "Now Loading Area " << areaName << std::endl;
    #endif

    loadedArea = nameToArea(areaName);
    AREA_VALID_CHECK(loadedArea, "Area of name \"" << areaName << "\" does not exist!");
    AreaEntry& newEntry = areaEntries[areaAsIndex(loadedArea)];
    newEntry.area = loadedArea;
    WorldLoadingError err = WorldLoadingError::NONE;

    // load locations in this area
    FIELD_CHECK(areaObject, "Locations", WorldLoadingError::AREA_MISSING_KEY);
    const auto& locations = areaObject.at("Locations").get<std::vector<json>>();
    for (const auto& loc : locations) {
        Location locOut;
        err = loadLocation(loc, locOut);
        if (err != World::WorldLoadingError::NONE)
        {
            std::cout << "Got error loading location: " << World::errorToName(err) << std::endl;
            return err;
        }
        #ifdef ENABLE_DEBUG
             // std::cout << "\tAdding location " << locationToName(locOut) << std::endl;
        #endif
        newEntry.locations.insert(locOut);
    }

    // load exits in this area
    FIELD_CHECK(areaObject, "Exits", WorldLoadingError::AREA_MISSING_KEY);
    const auto& exits = areaObject.at("Exits").get<std::vector<json>>();
    for (const auto& exit : exits) {
        Exit exitOut;
        err = loadExit(exit, exitOut, loadedArea);
        if (err != World::WorldLoadingError::NONE)
        {
            std::cout << "Got error loading exit: " << World::errorToName(err) << std::endl;
            return err;
        }
        #ifdef ENABLE_DEBUG
            // std::cout << "\tAdding exit -> " << areaToName(exitOut.connectedArea) << std::endl;
        #endif
        newEntry.exits.push_back(exitOut);
    }

    return WorldLoadingError::NONE;
}

bool World::evaluateRequirement(const Requirement& req, const ItemPool& ownedItems, const Settings& settings)
{
    uint32_t expectedCount = 0;
    GameItem item;
    switch(req.type)
    {
    case RequirementType::OR:
        return std::any_of(
            req.args.begin(),
            req.args.end(),
            [&](const Requirement::Argument& arg){
                return evaluateRequirement(std::get<Requirement>(arg), ownedItems, settings);
            }
        );
    case RequirementType::AND:
        return std::all_of(
            req.args.begin(),
            req.args.end(),
            [&](const Requirement::Argument& arg){
                if (arg.index() != 2)
                {
                    return false;
                }
                return evaluateRequirement(std::get<Requirement>(arg), ownedItems, settings);
            }
        );
    case RequirementType::NOT:
        if (req.args[0].index() != 2)
        {
            return false;
        }
        return !evaluateRequirement(std::get<Requirement>(req.args[0]), ownedItems, settings);
    case RequirementType::HAS_ITEM:
        // we can expect ownedItems will contain entires for every item type
        if (req.args[0].index() != 3)
        {
            return false;
        }
        item = std::get<GameItem>(req.args[0]);
        if (item == GameItem::Nothing) return true;
        return ownedItems.count(item) > 0;
    case RequirementType::COUNT:
        expectedCount = std::get<int>(req.args[0]);
        item = std::get<GameItem>(req.args[1]);
        if (item == GameItem::Nothing) return true;
        return ownedItems.count(item) >= expectedCount;
    case RequirementType::CAN_ACCESS:
        return evaluateRequirement(locationEntries[locationAsIndex(std::get<Location>(req.args[0]))].requirement, ownedItems, settings);
    case RequirementType::SETTING:
        // TODO: assuming all boolean settings for now
        return settings.count(std::get<Setting>(req.args[0])) > 0;
    case RequirementType::MACRO:
        return evaluateRequirement(macros[std::get<MacroIndex>(req.args[0])], ownedItems, settings);
    case RequirementType::NONE:
    default:
        // actually needs to be some error state?
        return false;
    }
    return false;
}

bool World::isAccessible(Location location, const ItemPool& ownedItems, const Settings& settings)
{
    const LocationEntry& entry = locationEntries[locationAsIndex(location)];
    return evaluateRequirement(entry.requirement, ownedItems, settings);
}

void World::getAccessibleLocations(const ItemPool& ownedItems,
                                             const Settings& settings,
                                             std::vector<Location>& accessibleLocations,
                                             std::vector<Location>& allowedLocations)
{
    // Add starting inventory items to use when checking logic
    ItemPool itemsInInventory = ownedItems;
    AddElementsToPool(itemsInInventory, getStartingInventory(settings, 0));
    for (auto& location : allowedLocations)
    {
        if (evaluateRequirement(locationEntries[locationAsIndex(location)].requirement, itemsInInventory, settings))
        {
            accessibleLocations.push_back(location);
        }
    }
}

std::vector<Location> World::assumedSearch(ItemPool& ownedItems,
                                                     const Settings& settings,
                                                     std::vector<Location>& allowedLocations)
{
    ItemPool newItems;
    std::vector<Location> reachable;
    do
    {
        reachable.clear();
        getAccessibleLocations(ownedItems, settings, reachable, allowedLocations);
        std::unordered_set<GameItem> newItems;
        // newItems = R.GetItems()
        for (const auto& r : reachable)
        {
            auto currentItem = locationEntries[locationAsIndex(r)].currentItem;
            // invalid => not yet filled
            if (currentItem == GameItem::INVALID) continue;
            newItems.insert(currentItem);
        }
        // NewItems = NewItems - I
        for (const auto& o : ownedItems)
        {
            newItems.erase(o);
        }
        // I.Add(NewItems);
        for ( const auto& n : newItems)
        {
            ownedItems.insert(n);
        }
    } while(!newItems.empty());
    return reachable;
}

void World::assumedFill(const ItemPool& itemsToPlace, std::vector<Location>& allowedLocations, const Settings& settings)
{
    std::vector<GameItem> ownedItemsList;
    ItemPool ownedItemsSet = itemsToPlace;
    ItemPool availableItems{};
    std::vector<Location> nullReachable;

    for (const auto& item : itemsToPlace)
    {
        ownedItemsList.push_back(item);
    }

    // null out all locations
    for (size_t locIdx = 0; locIdx < locationEntries.size(); locIdx++)
    {
        locationEntries[locIdx].currentItem = GameItem::INVALID;
    }

    size_t nullLocationsRemaining = locationEntries.size();
    // put seed here later
    auto randomEngine = std::default_random_engine();
    std::shuffle(ownedItemsList.begin(), ownedItemsList.end(), randomEngine);
    while(nullLocationsRemaining > 0 && !ownedItemsList.empty())
    {
        auto removed = ownedItemsList.back();
        std::cout << "Removed Item: " << gameItemToName(removed) << std::endl;
        ownedItemsList.pop_back();
        ownedItemsSet.erase(removed);
        auto reachable = assumedSearch(ownedItemsSet, settings, allowedLocations);
        nullReachable.clear();
        for (uint32_t idx = 0; idx < reachable.size(); idx++)
        {
            auto reachableLocation = reachable[idx];
            if (locationEntries[locationAsIndex(reachableLocation)].currentItem == GameItem::INVALID)
            {
                nullReachable.push_back(reachableLocation);
            }
        }
        std::cout << "Filtered Locations. Number remaining: " << std::to_string(nullReachable.size()) << std::endl;
        auto rand = std::uniform_int_distribution<size_t>(0, nullReachable.size() - 1);
        auto randomNullLoc = nullReachable[rand(randomEngine)];
        std::cout << "Random Location Chosen: " << locationToName(randomNullLoc) << std::endl;
        locationEntries[locationAsIndex(randomNullLoc)].currentItem = removed;
        availableItems.insert(removed);
        nullLocationsRemaining--;
    }
}

// Places items completely randomly without checking to see if the game is still
// beatable. This is used to place all of the junk items after all the advancement
// items have been placed using assumedFill since junk items won't change reachability.
void World::fastFill(const ItemPool& itemsToPlace, const std::vector<Location>& locationsToFill)
{

}

// Place all the items within the world. Certain groups of items will be placed first
// depending on settings and failure based restrictions
// World::FillError World::fillWorld(const ItemPool& completeItemPool, const Settings& settings)
// {
//     int retries = 0;
//     while (retries < 10) {
//         retries++;
//         std::cout << "Beginning Fill Attempt " << std::to_string(retries) << std::endl;
//
//         // Get all locations with no current items
//         std::vector<Location> allLocations = {};
//         for (auto& locationEntry : locationEntries) {
//             if (locationEntry.originalItem != GameItem::INVALID) {
//                 allLocations.push_back(locationEntry.location);
//             }
//         }
//
//         std::cout << "All Locations obtained. Total Size: " << std::to_string(allLocations.size()) << std::endl;
//
//         // Place all advancement items first using assumed fill
//         assumedFill(completeItemPool, allLocations, settings);
//
//         // Then place all junk items using fast fill
//
//         // If the world is still beatable, we have a successful placement
//         return FillError::NONE;
//     }
//     return FillError::RAN_OUT_OF_RETRIES;
// }

const char* World::errorToName(WorldLoadingError err)
{
    switch(err)
    {
    case WorldLoadingError::NONE:
        return "NONE";
    case WorldLoadingError::DUPLICATE_MACRO_NAME:
        return "DUPLICATE_MACRO_NAME";
    case WorldLoadingError::MACRO_DOES_NOT_EXIST:
        return "MACRO_DOES_NOT_EXIST";
    case WorldLoadingError::REQUIREMENT_TYPE_DOES_NOT_EXIST:
        return "REQUIREMENT_TYPE_DOES_NOT_EXIST";
    case WorldLoadingError::MAPPING_MISMATCH:
        return "MAPPING MISMATCH";
    case WorldLoadingError::GAME_ITEM_DOES_NOT_EXIST:
        return "GAME_ITEM_DOES_NOT_EXIST";
    case WorldLoadingError::AREA_DOES_NOT_EXIST:
        return "AREA_DOES_NOT_EXIST";
    case WorldLoadingError::LOCATION_DOES_NOT_EXIST:
        return "LOCATION_DOES_NOT_EXIST";
    case WorldLoadingError::EXIT_MISSING_KEY:
        return "EXIT_MISSING_KEY";
    case WorldLoadingError::SETTING_DOES_NOT_EXIST:
        return "SETTING_DOES_NOT_EXIST";
    case WorldLoadingError::INCORRECT_ARG_COUNT:
        return "INCORRECT_ARG_COUNT";
    case WorldLoadingError::EXPECTED_JSON_OBJECT:
        return "EXPECTED_JSON_OBJECT";
    case WorldLoadingError::AREA_MISSING_KEY:
        return "AREA_MISSING_KEY";
    case WorldLoadingError::LOCATION_MISSING_KEY:
        return "LOCATION_MISSING_KEY";
    case WorldLoadingError::MACRO_MISSING_KEY:
        return "MACRO_MISSING_KEY";
    case WorldLoadingError::REQUIREMENT_MISISNG_KEY:
        return "REQUIREMENT_MISISNG_KEY";
    case WorldLoadingError::INVALID_LOCATION_CATEGORY:
        return "INVALID_LOCATION_CATEGORY";
    case WorldLoadingError::INVALID_MODIFICATION_TYPE:
        return "INVALID_MODIFICATION_TYPE";
    case WorldLoadingError::INVALID_OFFSET_VALUE:
        return "INVALID_OFFSET_VALUE";
    case WorldLoadingError::INVALID_GAME_ITEM:
        return "INVALID_GAME_ITEM";
    default:
        return "UNKNOWN";
    }
}

std::string World::getLastErrorDetails()
{
    std::string out = lastError.str();
    lastError.str(std::string());
    lastError.clear();
    return out;
}

// Will dump a file which can be turned into a visual graph using graphviz
// https://graphviz.org/download/
// Use command: dot -Tsvg <filename> -o world.svg
// Then, open world.svg in a browser and CTRL + F to find the area of interest
void World::dumpWorldGraph(const std::string& filename)
{
  std::ofstream worldGraph;
  worldGraph.open (filename + ".dot");
  worldGraph << "digraph {\n\tcenter=true;\n";


  for (const AreaEntry& areaEntry : areaEntries) {

      std::string parentName = areaToName(areaEntry.area);
      worldGraph << "\t\"" << parentName << "\"[shape=\"plain\"];" << std::endl;

      // Make edge connections defined by exits
      for (const auto& exit : areaEntry.exits) {
          std::string connectedName = areaToName(exit.connectedArea);
          if (parentName != "INVALID" && connectedName != "INVALID"){
              worldGraph << "\t\"" << parentName << "\" -> \"" << connectedName << "\"" << std::endl;
          }
      }

      // Make edge connections between areas and their locations
      for (const auto& location : areaEntry.locations) {
          std::string connectedLocation = locationToName(location);
          if (parentName != "INVALID" && connectedLocation != "INVALID"){
              worldGraph << "\t\"" << connectedLocation << "\"[shape=\"plain\" fontcolor=\"blue\"];" << std::endl;
              worldGraph << "\t\"" << parentName << "\" -> \"" << connectedLocation << "\"" << "[dir=forward color=\"blue\"]" << std::endl;
          }
      }
  }

  worldGraph << "}";
  worldGraph.close();
  std::cout << "Dumped world graph at " << filename << ".dot" << std::endl;
}
