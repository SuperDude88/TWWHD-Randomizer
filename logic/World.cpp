
#include "World.hpp"
#include "Requirements.hpp"
#include "PoolFunctions.hpp"
#include "Debug.hpp"
#include "Search.hpp"
#include "Random.hpp"
#include "../libs/json.hpp"
#include "../options.hpp"
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
#define MAPPING_CHECK(str1, str2) if (str1 != str2) {lastError << "\"" << str1 << "\" does not equal" << std::endl << "\"" << str2 << "\""; return WorldLoadingError::MAPPING_MISMATCH;}
#define VALID_CHECK(e, invalid, msg, err) if(e == invalid) {lastError << "\t" << msg; return err;}
#define ITEM_VALID_CHECK(item, msg) VALID_CHECK(item, GameItem::INVALID, msg, WorldLoadingError::GAME_ITEM_DOES_NOT_EXIST)
#define AREA_VALID_CHECK(area, msg) VALID_CHECK(area, Area::INVALID, msg, WorldLoadingError::AREA_DOES_NOT_EXIST)
#define LOCATION_VALID_CHECK(loc, msg) VALID_CHECK(loc, LocationId::INVALID, msg, WorldLoadingError::LOCATION_DOES_NOT_EXIST)
#define OPTION_VALID_CHECK(opt, msg) VALID_CHECK(opt, Option::INVALID, msg, WorldLoadingError::OPTION_DOES_NOT_EXIST)

World::World()
{
    areaEntries.resize(AREA_COUNT);
    locationEntries.resize(LOCATION_COUNT);
}

// Potentially set different settings for different worlds
void World::setSettings(const Settings& settings_)
{
    settings = std::move(settings_);
}

const Settings& World::getSettings() const
{
    return settings;
}

void World::setWorldId(int newWorldId)
{
    worldId = newWorldId;
}

int World::getWorldId() const
{
    return worldId;
}

// Generate the pools of starting items and items to place for this world
void World::setItemPools()
{
    itemPool.clear();
    startingItems.clear();
    auto gameItemPool = generateGameItemPool(settings);
    auto startingGameItems = generateStartingGameItemPool(settings);

    // Set worldId for each item in each pool
    for (auto gameItem : startingGameItems)
    {
        startingItems.emplace_back(gameItem, worldId);
        // If a starting item is in the main item pool, replace it with some junk
        auto itemPoolItr = std::find(gameItemPool.begin(), gameItemPool.end(), gameItem);
        if (itemPoolItr != gameItemPool.end())
        {
            *itemPoolItr = getRandomJunk();
        }
    }

    for (auto gameItem : gameItemPool)
    {
        itemPool.emplace_back(gameItem, worldId);
    }
    logItemPool("Items for world " + std::to_string(worldId), itemPool);
}

ItemPool World::getItemPool() const
{
    return itemPool;
}

void World::addToItemPool(const GameItem gameItem)
{
    if (gameItem != GameItem::INVALID && gameItem != GameItem::NOTHING)
    {
        itemPool.emplace_back(gameItem, worldId);
    }
}

ItemPool World::getStartingItems() const
{
    return startingItems;
}

LocationPool World::getLocations()
{
    LocationPool locations = {};
    // start at 1 since 0 is the INVALID entry
    for (size_t i = 1; i < locationIdAsIndex(LocationId::COUNT); i++) {
        locations.push_back(&locationEntries[i]);
    }
    return locations;
}

void World::determineChartMappings()
{
    // The ordering of this array corresponds each treasure/triforce chart with
    // the island sector it's located in
    static std::array<GameItem, 49> charts = {
        GameItem::TreasureChart25, // Sector 1 Forsaken Fortress
        GameItem::TreasureChart7,  // Sector 2 Star Island
        GameItem::TreasureChart24, // etc...
        GameItem::TreasureChart42,
        GameItem::TreasureChart11,
        GameItem::TreasureChart45,
        GameItem::TreasureChart13,
        GameItem::TreasureChart41,
        GameItem::TreasureChart29,
        GameItem::TreasureChart22,
        GameItem::TreasureChart18,
        GameItem::TreasureChart30,
        GameItem::TreasureChart39,
        GameItem::TreasureChart19,
        GameItem::TreasureChart8,
        GameItem::TreasureChart2,
        GameItem::TreasureChart10,
        GameItem::TreasureChart26,
        GameItem::TreasureChart3,
        GameItem::TreasureChart37,
        GameItem::TreasureChart27,
        GameItem::TreasureChart38,
        GameItem::TriforceChart1,
        GameItem::TreasureChart21,
        GameItem::TreasureChart6,
        GameItem::TreasureChart14,
        GameItem::TreasureChart34,
        GameItem::TreasureChart5,
        GameItem::TreasureChart28,
        GameItem::TreasureChart35,
        GameItem::TriforceChart2,
        GameItem::TreasureChart44,
        GameItem::TreasureChart1,
        GameItem::TreasureChart20,
        GameItem::TreasureChart36,
        GameItem::TreasureChart23,
        GameItem::TreasureChart12,
        GameItem::TreasureChart16,
        GameItem::TreasureChart4,
        GameItem::TreasureChart17,
        GameItem::TreasureChart31,
        GameItem::TriforceChart3,
        GameItem::TreasureChart9,
        GameItem::TreasureChart43,
        GameItem::TreasureChart40,
        GameItem::TreasureChart46,
        GameItem::TreasureChart15,
        GameItem::TreasureChart32,
        GameItem::TreasureChart33, // Sector 49 Five Star Isles
    };

    // Only shuffle around the charts of we're randomizing them
    if (settings.randomize_charts)
    {
        shufflePool(charts);
    }

    for (size_t i = 0; i < charts.size(); i++)
    {
        auto chart = charts[i];
        chartMappings[i] = chart;
        // Change the macro for this island's chart to the one at this index in the array.
        // "Chart for Island <sector number>" macros are type "HAS_ITEM" and have
        // one argument which is the chart GameItem.
        size_t sector = i + 1;
        macros[macroNameMap.at("Chart for Island " + std::to_string(sector))].args[0] = chart;

        debugLog("\tChart for Island " + std::to_string(sector) + " is now " + gameItemToName(chart) + " for world " + std::to_string(worldId));
    }
}

// Returns whether or not the sunken treasure location has a treasure/triforce chart leading to it
bool World::chartLeadsToSunkenTreasure(const Location& location, const std::string& itemPrefix)
{
    auto locationId = location.locationId;
    // If this isn't a sunken treasure location, then a chart won't lead to it
    if (locationId < LocationId::ForsakenFortressSunkenTreasure || locationId > LocationId::FiveStarSunkenTreasure)
    {
        debugLog("Non-sunken treasure location passed into sunken treasure check: " + locationName(&location));
        return false;
    }

    size_t islandNumber = locationIdAsIndex(locationId) - locationIdAsIndex(LocationId::ForsakenFortressSunkenTreasure);
    return gameItemToName(chartMappings[islandNumber]).find(itemPrefix) != std::string::npos;
}

void World::determineProgressionLocations()
{
    for (auto& location : locationEntries)
    {
        // If all of the location categories are set as progression, then this is a location which
        // is allowed to contain progression items (but it won't necessarily get one)
        if (std::all_of(location.categories.begin(), location.categories.end(), [this, location](LocationCategory category)
        {

            return ( category == LocationCategory::Dungeon           && this->settings.progression_dungeons)            ||
                   ( category == LocationCategory::GreatFairy        && this->settings.progression_great_fairies)       ||
                   ( category == LocationCategory::PuzzleSecretCave  && this->settings.progression_puzzle_secret_caves) ||
                   ( category == LocationCategory::CombatSecretCave  && this->settings.progression_combat_secret_caves) ||
                   ( category == LocationCategory::ShortSideQuest    && this->settings.progression_short_sidequests)    ||
                   ( category == LocationCategory::LongSideQuest     && this->settings.progression_long_sidequests)     ||
                   ( category == LocationCategory::SpoilsTrading     && this->settings.progression_spoils_trading)      ||
                   ( category == LocationCategory::Minigame          && this->settings.progression_minigames)           ||
                   ( category == LocationCategory::FreeGift          && this->settings.progression_free_gifts)          ||
                   ( category == LocationCategory::Mail              && this->settings.progression_mail)                ||
                   ( category == LocationCategory::Submarine         && this->settings.progression_submarines)          ||
                   ( category == LocationCategory::EyeReefChests     && this->settings.progression_eye_reef_chests)     ||
                   ( category == LocationCategory::SunkenTreasure    && this->settings.progression_triforce_charts && chartLeadsToSunkenTreasure(location, "TriforceChart")) ||
                   ( category == LocationCategory::SunkenTreasure    && this->settings.progression_treasure_charts && chartLeadsToSunkenTreasure(location, "TreasureChart")) ||
                   ( category == LocationCategory::ExpensivePurchase && this->settings.progression_expensive_purchases) ||
                   ( category == LocationCategory::Misc              && this->settings.progression_misc)                ||
                   ( category == LocationCategory::TingleChest       && this->settings.progression_tingle_chests)       ||
                   ( category == LocationCategory::BattleSquid       && this->settings.progression_battlesquid)         ||
                   ( category == LocationCategory::SavageLabyrinth   && this->settings.progression_savage_labyrinth)    ||
                   ( category == LocationCategory::IslandPuzzle      && this->settings.progression_island_puzzles)      ||
                   ( category == LocationCategory::Obscure           && this->settings.progression_obscure)             ||
                   ((category == LocationCategory::Platform || category == LocationCategory::Raft)    && settings.progression_platforms_rafts) ||
                   ((category == LocationCategory::BigOcto  || category == LocationCategory::Gunboat) && settings.progression_big_octos_gunboats) ||
                   // or if this location is DefeatGanondorf
                   location.locationId == LocationId::DefeatGanondorf;
        }))
        {
            location.progression = true;
        }
    }
}

void World::determineRaceModeDungeons()
{
    if (settings.race_mode)
    {
        static std::array<DungeonId, 6> dungeons = {
            DungeonId::DragonRoostCavern,
            DungeonId::ForbiddenWoods,
            DungeonId::TowerOfTheGods,
            DungeonId::ForsakenFortress,
            DungeonId::EarthTemple,
            DungeonId::WindTemple,
        };

        shufflePool(dungeons);

        for (size_t i = 0; i < dungeons.size(); i++)
        {
            if (i < settings.num_race_mode_dungeons)
            {
                raceModeDungeons.push_back(dungeons[i]);
            }
            else
            {
                // If we've already chosen our race mode dungeons, then set all
                // the other dungeons' locations as non-progression. If dungeons
                // are set as progression locations, we already set them all as
                // progression previously, so here we unset those which aren't
                // progression dungeons.
                for (auto& locationId : dungeonIdToDungeon(dungeons[i]).locations)
                {
                    locationEntries[locationIdAsIndex(locationId)].progression = false;
                }
            }
        }
    }
}

World::WorldLoadingError World::parseElement(RequirementType type, const std::vector<json>& args, std::vector<Requirement::Argument>& out)
{
    std::string argStr;
    GameItem argItem = GameItem::INVALID;
    Area argArea = Area::INVALID;
    int countValue = 0;
    int optionEvaluation = 0;
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
        argArea = nameToArea(args[0].get<std::string>());
        AREA_VALID_CHECK(argArea, "Area " << args[0].get<std::string>() << " does not exist");
        out.push_back(argArea);
        return WorldLoadingError::NONE;
    case RequirementType::SETTING:
        // setting just expects a settings name. Resolve settings to a true/false
        // value now since they won't change during world building/item placement
        argStr = args[0].get<std::string>();
        optionEvaluation = evaluateOption(settings, argStr);
        // -1 means the setting doesn't exist
        if (optionEvaluation == -1)
        {
            OPTION_VALID_CHECK(Option::INVALID, "Setting " << argStr << " does not exist");
        }
        out.push_back(optionEvaluation);
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

World::WorldLoadingError World::loadLocation(const json& locationObject, LocationId& loadedLocation)
{
    // failure indicated by INVALID type for category
    // maybe change to Optional later if thats determined to work
    // on wii u
    OBJECT_CHECK(locationObject, locationObject.dump());
    FIELD_CHECK(locationObject, "Name", WorldLoadingError::LOCATION_MISSING_KEY);
    std::string locationName = locationObject.at("Name").get<std::string>();
    loadedLocation = nameToLocationId(locationName);
    LOCATION_VALID_CHECK(loadedLocation, "Location of name \"" << locationName << "\" does not exist!");
    MAPPING_CHECK(locationName, locationIdToName(nameToLocationId(locationName)));
    Location& newEntry = locationEntries[locationIdAsIndex(loadedLocation)];
    newEntry.locationId = loadedLocation;
    newEntry.worldId = worldId;
    newEntry.categories.clear();
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
    // Set whether or not this location can contain progression items based on its
    // categories
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
    newEntry.originalItem = {nameToGameItem(itemName), worldId};
    ITEM_VALID_CHECK(
        newEntry.originalItem.getGameItemId(),
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
    MAPPING_CHECK(connectedAreaName, areaToName(nameToArea(connectedAreaName)));
    loadedExit.parentArea = parentArea;
    loadedExit.connectedArea = connectedArea;
    loadedExit.worldId = worldId;
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
    // debugLog("Now Loading Area " + areaName);
    loadedArea = nameToArea(areaName);
    AREA_VALID_CHECK(loadedArea, "Area of name \"" << areaName << "\" does not exist!");
    MAPPING_CHECK(areaName, areaToName(nameToArea(areaName)))
    AreaEntry& newEntry = areaEntries[areaAsIndex(loadedArea)];
    newEntry.area = loadedArea;
    newEntry.worldId = worldId;
    WorldLoadingError err = WorldLoadingError::NONE;

    // load locations in this area
    FIELD_CHECK(areaObject, "Locations", WorldLoadingError::AREA_MISSING_KEY);
    const auto& locations = areaObject.at("Locations").get<std::vector<json>>();
    for (const auto& loc : locations) {
        LocationId locOut;
        err = loadLocation(loc, locOut);
        if (err != World::WorldLoadingError::NONE)
        {
            std::cout << "Got error loading location: " << World::errorToName(err) << std::endl;
            return err;
        }
        // debugLog("\tAdding location " + locationIdToName(locOut));
        newEntry.locations.push_back(&locationEntries[locationIdAsIndex(locOut)]);
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
        // debugLog("\tAdding exit -> " + areaToName(exitOut.connectedArea));
        newEntry.exits.push_back(exitOut);
    }

    return WorldLoadingError::NONE;
}

// Load the world based on the given world file and macros file
int World::loadWorld(const std::string& worldFilePath, const std::string& macrosFilePath)
{
    using json = nlohmann::json;

    std::ifstream macroFile(macrosFilePath);
    if (!macroFile.is_open())
    {
        std::cout << "unable to open macro file for world " << std::to_string(worldId) << std::endl;
        return 1;
    }
    std::ifstream worldFile(worldFilePath);
    if (!worldFile.is_open())
    {
        std::cout << "Unable to open world file for world " << std::to_string(worldId) << std::endl;
        return 1;
    }

    json world_j = json::parse(worldFile, nullptr, false);
    auto macroFileObj = json::parse(macroFile, nullptr, false);
    if (macroFileObj.is_discarded())
    {
        std::cout << "unable to parse macros from file for world " << std::to_string(worldId) << std::endl;
        return 1;
    }

    auto err = loadMacros(macroFileObj["Macros"].get<std::vector<json>>());
    if (err != World::WorldLoadingError::NONE)
    {
        std::cout << "Got error loading macros for world " << std::to_string(worldId) << ": " << World::errorToName(err) << std::endl;
        std::cout << getLastErrorDetails() << std::endl;
        return 1;
    }

    if (!world_j.contains("Areas"))
    {
        std::cout << "Improperly formatted world file for world " << std::to_string(worldId) << std::endl;
        return 1;
    }
    for (const auto& area : world_j.at("Areas"))
    {
        Area areaOut;
        err = loadArea(area, areaOut);
        if (err != World::WorldLoadingError::NONE)
        {
            std::cout << "Got error loading area for world " << std::to_string(worldId) << ": " << World::errorToName(err) << std::endl;
            std::cout << getLastErrorDetails() << std::endl;
            return 1;
        }
    }
    return 0;
}

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
    case WorldLoadingError::OPTION_DOES_NOT_EXIST:
        return "OPTION_DOES_NOT_EXIST";
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
    debugLog(out);
    return out;
}

// Will dump a file which can be turned into a visual graph using graphviz
// Although the way it's presented isn't great.
// https://graphviz.org/download/
// Use this command if things are too spread out: fdp   -Tsvg <filename> -o world.svg
// Use this command if things are too squished:   circo -Tsvg <filename> -o world.svg
// Then, open world.svg in a browser and CTRL + F to find the area of interest
void World::dumpWorldGraph(const std::string& filename)
{
    std::ofstream worldGraph;
    worldGraph.open (filename + ".dot");
    worldGraph << "digraph {\n\tcenter=true;\n";

    for (const AreaEntry& areaEntry : areaEntries) {

        std::string color = areaEntry.isAccessible ? "\"black\"" : "\"red\"";

        std::string parentName = areaToName(areaEntry.area);
        worldGraph << "\t\"" << parentName << "\"[shape=\"plain\" fontcolor=" << color << "];" << std::endl;

        // Make edge connections defined by exits
        for (const auto& exit : areaEntry.exits) {
            std::string connectedName = areaToName(exit.connectedArea);
            if (parentName != "INVALID" && connectedName != "INVALID"){
                worldGraph << "\t\"" << parentName << "\" -> \"" << connectedName << "\"" << std::endl;
            }
        }

        // Make edge connections between areas and their locations
        for (const auto& location : areaEntry.locations) {
            std::string connectedLocation = locationIdToName(location->locationId);
            std::string itemAtLocation = location->currentItem.getName();
            if (parentName != "INVALID" && connectedLocation != "INVALID"){
                worldGraph << "\t\"" << connectedLocation << "\"[label=<" << connectedLocation << ":<br/>" << itemAtLocation << "> shape=\"plain\" fontcolor=" << color << "];" << std::endl;
                worldGraph << "\t\"" << parentName << "\" -> \"" << connectedLocation << "\"" << "[dir=forward color=" << color << "]" << std::endl;
            }
        }
    }

    worldGraph << "}";
    worldGraph.close();
    std::cout << "Dumped world graph at " << filename << ".dot" << std::endl;
}
