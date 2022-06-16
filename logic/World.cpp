
#include "World.hpp"
#include "Requirements.hpp"
#include "PoolFunctions.hpp"
#include "Debug.hpp"
#include "Search.hpp"
#include "Random.hpp"
#include "../options.hpp"
#define RYML_SINGLE_HDR_DEFINE_NOW
#include "../libs/ryml.hpp"
#include <algorithm>
#include <cstdlib>
#include <climits>
#include <vector>
#include <random>
#include <fstream>
#include <iostream>

// some error checking macros for brevity and since we can't use exceptions
#define FIELD_CHECK(j, field, err) if(!j.contains(field)) {lastError << "Unable to retrieve field: \"" << field << '"'; return err;}
#define YAML_FIELD_CHECK(ref, key, err) if(!ref.has_child(key)) {lastError << "Unable to find key: \"" << key << '"'; return err;}
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

World::World(size_t numWorlds_)
{
    areaEntries.resize(AREA_COUNT);
    locationEntries.resize(LOCATION_COUNT);
    numWorlds = numWorlds_;
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

ItemPool World::getStartingItems() const
{
    return startingItems;
}

LocationPool World::getLocations(bool onlyProgression /*= false*/)
{
    LocationPool locations = {};
    // start at 1 since 0 is the INVALID entry
    for (size_t i = 1; i < locationIdAsIndex(LocationId::COUNT); i++) {
        if (!onlyProgression || locationEntries[i].progression)
        {
            locations.push_back(&locationEntries[i]);
        }
    }
    return locations;
}

AreaEntry& World::getArea(const Area& area)
{
    return areaEntries[areaAsIndex(area)];
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

    // Only shuffle around the charts if we're randomizing them
    if (settings.randomize_charts)
    {
        shufflePool(charts);
    }

    for (size_t i = 0; i < charts.size(); i++)
    {
        auto chart = charts[i];
        chartMappings[i] = chart;
        // Change the macro for this island's chart to the one at this index in the array.
        // "ChartForIsland<sector number>" macros are type "HAS_ITEM" and have
        // one argument which is the chart GameItem.
        size_t sector = i + 1;
        macros[macroNameMap.at("ChartForIsland" + std::to_string(sector))].args[0] = chart;

        debugLog("\tChartForIsland" + std::to_string(sector) + " is now " + gameItemToName(chart) + " for world " + std::to_string(worldId));
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

World::WorldLoadingError World::determineRaceModeDungeons()
{
    if (settings.race_mode)
    {
        auto dungeons = getDungeonList();

        shufflePool(dungeons);

        int setRaceModeDungeons = 0;
        // Loop through all the dungeons and see if any of them have items plandomized
        // within them. If they have major items plandomized, then select those dungeons
        // as race mode dungeons
        if (settings.plandomizer)
        {
            for (const auto& dungeonId : dungeons)
            {
                auto dungeon = dungeonIdToDungeon(dungeonId);
                for (auto& locationId : dungeon.locations)
                {
                    auto dungeonLocation = &locationEntries[locationIdAsIndex(locationId)];
                    bool dungeonLocationForcesRaceMode = plandomizerLocations.count(dungeonLocation) == 0 ? false : !plandomizerLocations[dungeonLocation].isJunkItem();
                    if (dungeonLocationForcesRaceMode)
                    {
                        // However, if the dungeon's race mode location is junk then
                        // that's an error on the user's part.
                        Location* raceModeLocation = &locationEntries[locationIdAsIndex(dungeon.locations.back())];
                        bool raceModeLocationIsAcceptable = plandomizerLocations.count(raceModeLocation) == 0 ? true : !plandomizerLocations[dungeonLocation].isJunkItem();
                        if (!raceModeLocationIsAcceptable)
                        {
                            std::cout << "Plandomizer Error: Junk item placed at race mode location in dungeon \"" << dungeonIdToName(dungeonId) << "\" with potentially major item" << std::endl;
                            return WorldLoadingError::PLANDOMIZER_ERROR;
                        }
                        debugLog("Chose race mode dungeon : " + dungeonIdToName(dungeonId));
                        raceModeDungeons.insert({dungeonId, HintRegion::INVALID});
                        setRaceModeDungeons++;
                        break;
                    }
                }
            }
        }

        // If too many are set, return an error
        if (setRaceModeDungeons > settings.num_race_mode_dungeons)
        {
            std::cout << "Plandomizer Error: Too many race mode locations set with potentially major items" << std::endl;
            std::cout << "set race mode locations: " << std::to_string(setRaceModeDungeons) << std::endl;
            std::cout << "Set number of race mode dungeons: " << std::to_string(settings.num_race_mode_dungeons) << std::endl;
            return WorldLoadingError::PLANDOMIZER_ERROR;
        }

        // Now check again and fill in any more dungeons that may be necessary
        // Also set non-race mode dungeons locations as non-progress
        for (const auto& dungeonId : dungeons)
        {
            // If this dungeon was already selected, then skip it
            if (raceModeDungeons.count(dungeonId) > 0)
            {
                continue;
            }
            auto dungeon = dungeonIdToDungeon(dungeonId);
            // If this dungeon has a junk item placed as its race mode
            // location, then skip it
            auto dungeonLocation = &locationEntries[locationIdAsIndex(dungeon.locations.back())];
            bool raceModeLocationIsAcceptable = plandomizerLocations.count(dungeonLocation) == 0 ? false : plandomizerLocations[dungeonLocation].isJunkItem();
            if (!raceModeLocationIsAcceptable && setRaceModeDungeons < settings.num_race_mode_dungeons)
            {
                debugLog("Chose race mode dungeon : " + dungeonIdToName(dungeonId));
                raceModeDungeons.insert({dungeonId, HintRegion::INVALID});
                setRaceModeDungeons++;
            }
            else
            {
                // If we've already chosen our race mode dungeons, then set all
                // the other dungeons' locations as non-progression. If dungeons
                // are set as progression locations, we already set them all as
                // progression previously, so here we unset those which aren't
                // progression dungeons.
                for (auto& locationId : dungeon.locations)
                {
                    locationEntries[locationIdAsIndex(locationId)].progression = false;
                }
                // Also set any locations outside the dungeon which are dependent
                // on beating it as non-progression locations
                for (auto& locationId : dungeon.outsideDependentLocations)
                {
                    locationEntries[locationIdAsIndex(locationId)].progression = false;
                }
            }
        }
        if (setRaceModeDungeons < settings.num_race_mode_dungeons)
        {
            std::cout << "Plandomizer Error: Not enough race mode locations for set number of race mode dungeons" << std::endl;
            std::cout << "Possible race mode locations: " << std::to_string(setRaceModeDungeons) << std::endl;
            std::cout << "Set number of race mode dungeons: " << std::to_string(settings.num_race_mode_dungeons) << std::endl;
            return WorldLoadingError::PLANDOMIZER_ERROR;
        }
    }
    return WorldLoadingError::NONE;
}

// Helper function for dealing with YAML library strings
static std::string substrToString(const ryml::csubstr& substr)
{
    return std::string(substr.data(), substr.size());
}

// Takes a logic expression string and stores it as a requirement within the passed in Requirement
// object. This means we only have to parse the string once and then evaluating it many times
// later is a lot faster. An example of a logic expression string is: "GrapplingHook and (DekuLeaf or Hookshot)"
World::WorldLoadingError World::parseRequirementString(const std::string& str, Requirement& req)
{
    WorldLoadingError err;
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
        std::cout << "Extra or missing parenthesis within expression: \"" << str << "\"" << std::endl;
        return WorldLoadingError::EXTRA_OR_MISSING_PARENTHESIS;
    }

    // Next we split up the expression by the delimeter in the previous step
    size_t pos = 0;
    std::vector<std::string> splitLogicStr = {};
    while ((pos = logicStr.find(delimeter)) != std::string::npos)
    {
        splitLogicStr.push_back(logicStr.substr(0, pos));
        logicStr.erase(0, pos + 1);
    }
    splitLogicStr.push_back(logicStr);

    // Once we have the different parts of our expression, we can use the number
    // of parts we have to determine what kind of expression it is.

    // If we only have one part, then we have either an event, an item, a macro,
    // a can_access check, a setting, or a count
    if (splitLogicStr.size() == 1)
    {

        std::string argStr = splitLogicStr[0];
        // First see if we have an event
        if (argStr[0] == '\'')
        {
            req.type = RequirementType::EVENT;
            std::string eventName (argStr.begin() + 1, argStr.end() - 1);
            // Add "WX" where X is the world Id to the end of the string to
            // differentiate between events of different worlds
            eventName += "W" + std::to_string(worldId);
            req.args.push_back(eventName);
            return WorldLoadingError::NONE;
        }

        // Then a macro...
        if (macroNameMap.count(argStr) > 0)
        {
            req.type = RequirementType::MACRO;
            req.args.push_back(macroNameMap.at(argStr));
            return WorldLoadingError::NONE;
        }
        // Then an item...
        else if (nameToGameItem(argStr) != GameItem::INVALID)
        {
            req.type = RequirementType::HAS_ITEM;
            req.args.push_back(nameToGameItem(argStr));
            return WorldLoadingError::NONE;
        }
        // Then a can_access check...
        else if (argStr.find("can_access") != std::string::npos)
        {
            req.type = RequirementType::CAN_ACCESS;
            std::string areaName (argStr.begin() + argStr.find('(') + 1, argStr.end() - 1);
            Area argArea = nameToArea(areaName);
            AREA_VALID_CHECK(argArea, "Area " << areaName << " does not exist");
            req.args.push_back(argArea);
            return WorldLoadingError::NONE;
        }
        // Then a setting...
        else if (evaluateOption(settings, argStr) != -1)
        {
            req.type = RequirementType::SETTING;
            req.args.push_back(evaluateOption(settings, argStr));
            return WorldLoadingError::NONE;
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
            countArgs.erase(std::remove(countArgs.begin(), countArgs.end(), ' '), countArgs.end());

            // Split up the arguments
            pos = 0;
            splitLogicStr = {};
            while ((pos = countArgs.find(",")) != std::string::npos)
            {
                splitLogicStr.push_back(countArgs.substr(0, pos));
                countArgs.erase(0, pos + 1);
            }
            splitLogicStr.push_back(countArgs);

            // Get the arguments
            int count = std::stoi(splitLogicStr[0]);
            std::string itemName = splitLogicStr[1];
            auto argItem = nameToGameItem(itemName);
            ITEM_VALID_CHECK(argItem, "Game Item of name \"" << itemName << " Does Not Exist");
            req.args.push_back(count);
            req.args.push_back(argItem);
            return WorldLoadingError::NONE;
        }

        std::cout << "Unrecognized logic symbol: \"" << argStr << "\"" << std::endl;
        return WorldLoadingError::LOGIC_SYMBOL_DOES_NOT_EXIST;
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
            if ((err = parseRequirementString(reqStr, std::get<Requirement>(req.args.back()))) != WorldLoadingError::NONE) return err;
        }
        else
        {
            std::cout << "Unrecognized 2 part expression: " << str << std::endl;
            return WorldLoadingError::LOGIC_SYMBOL_DOES_NOT_EXIST;
        }
    }

    // If we have more than two parts to our expression, then we have either "and"
    // or "or".
    bool andType = elementInPool("and", splitLogicStr);
    bool orType = elementInPool("or", splitLogicStr);

    // If we have both of them, there's a problem with the logic expression
    if (andType && orType)
    {
        std::cout << "\"and\" & \"or\" in same nesting level when parsing \"" << str << "\"" << std::endl;
        return WorldLoadingError::SAME_NESTING_LEVEL;
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
            if ((err = parseRequirementString(reqStr, std::get<Requirement>(req.args.back()))) != WorldLoadingError::NONE) return err;
        }
    }


    if (req.type != RequirementType::NONE)
    {
        return WorldLoadingError::NONE;
    }
    else
    // If we've reached this point, we weren't able to determine a logical operator within the expression
    {
        std::cout << "Could not determine logical operator type from expression: \"" << str << "\"" << std::endl;
        return WorldLoadingError::COULD_NOT_DETERMINE_TYPE;
    }
}

World::WorldLoadingError World::parseMacro(const std::string& macroLogicExpression, Requirement& reqOut)
{

    WorldLoadingError err = WorldLoadingError::NONE;
    // readd prechecks?
    if ((err = parseRequirementString(macroLogicExpression, reqOut)) != WorldLoadingError::NONE) return err;
    return WorldLoadingError::NONE;
}

World::WorldLoadingError World::loadMacros(const ryml::Tree& macroListTree)
{
    std::string name;
    WorldLoadingError err = WorldLoadingError::NONE;
    uint32_t macroCount = 0;

    // first pass to get all macro names
    for (const ryml::NodeRef& macro : macroListTree.rootref().children())
    {
        if (!macro.has_key())
        {
            return WorldLoadingError::MACRO_MISSING_KEY;
        }
        macroNameMap.emplace(std::string(macro.key().data(), macro.key().size()), macroCount++);
    }
    for (const ryml::NodeRef& macro : macroListTree.rootref().children())
    {
        macros.emplace_back();
        if (!macro.has_val())
        {
            std::cout << "Macro \"" << macro.key() << "\" has no value" << std::endl;
            return WorldLoadingError::MACRO_MISSING_VAL;
        }

        std::string logicExpression = substrToString(macro.val());
        if ((err = parseMacro(logicExpression, macros.back())) != WorldLoadingError::NONE)
        {
            lastError << " | Encountered parsing macro of name " << macro;
            return err;
        }
    }
    return WorldLoadingError::NONE;
}

World::WorldLoadingError World::loadLocation(const ryml::NodeRef& locationObject, LocationId& loadedLocation)
{
    // failure indicated by INVALID type for category
    // maybe change to Optional later if thats determined to work
    // on wii u
    YAML_FIELD_CHECK(locationObject, "Name", WorldLoadingError::LOCATION_MISSING_KEY);
    const auto& locName = locationObject["Name"].val();
    std::string locationName = std::string(locName.data(), locName.size());
    loadedLocation = nameToLocationId(locationName);
    LOCATION_VALID_CHECK(loadedLocation, "Location of name \"" << locationName << "\" does not exist!");
    MAPPING_CHECK(locationName, locationIdToName(nameToLocationId(locationName)));

    // Add the pretty name to the map of pretty names to use later
    YAML_FIELD_CHECK(locationObject, "PrettyName", WorldLoadingError::LOCATION_MISSING_KEY);
    const auto& prettyName = locationObject["PrettyName"].val();
    std::string locationPrettyName = std::string(prettyName.data(), prettyName.size());
    storeNewLocationPrettyName(loadedLocation, locationPrettyName);

    Location& newEntry = locationEntries[locationIdAsIndex(loadedLocation)];
    newEntry.locationId = loadedLocation;
    newEntry.worldId = worldId;
    newEntry.categories.clear();
    YAML_FIELD_CHECK(locationObject, "Category", WorldLoadingError::LOCATION_MISSING_KEY);
    for (const ryml::NodeRef& category : locationObject["Category"].children())
    {
        const auto& catName = category.val();
        const std::string& categoryNameStr = std::string(catName.data(), catName.size());
        const auto& cat = nameToLocationCategory(categoryNameStr);
        if (cat == LocationCategory::INVALID)
        {
            lastError << "Encountered unknown location category \"" << categoryNameStr << "\"";
            lastError << " while parsing location " << locationName << " in world " << std::to_string(worldId + 1);
            return WorldLoadingError::INVALID_LOCATION_CATEGORY;
        }
        newEntry.categories.insert(cat);
    }
    YAML_FIELD_CHECK(locationObject, "Path", WorldLoadingError::LOCATION_MISSING_KEY);
    const auto& path = locationObject["Path"].val();
    newEntry.method.filePath = std::string(path.data(), path.size());
    YAML_FIELD_CHECK(locationObject, "Type", WorldLoadingError::LOCATION_MISSING_KEY);
    const auto& type = locationObject["Type"].val();
    const std::string& modificationType = std::string(type.data(), type.size());
    newEntry.method.type = nameToModificationType(modificationType);
    if (newEntry.method.type == LocationModificationType::INVALID)
    {
        lastError << "Error processing location " << locationName << " in world " << std::to_string(worldId + 1) << ": ";
        lastError << "Modificaiton Type \"" << modificationType << "\" Does Not Exist";
        return WorldLoadingError::INVALID_MODIFICATION_TYPE;
    }
    YAML_FIELD_CHECK(locationObject, "Offsets", WorldLoadingError::LOCATION_MISSING_KEY);
    for (const ryml::NodeRef& offset : locationObject["Offsets"].children())
    {
        const auto& offsetStr = std::string(offset.val().data(), offset.val().size());
        unsigned long offsetValue = std::strtoul(offsetStr.c_str(), nullptr, 0);
        if (offsetValue == 0 || offsetValue == ULONG_MAX)
        {
            lastError << "Encountered an invalid offset for location " << locationName << " in world " << std::to_string(worldId + 1);
            return WorldLoadingError::INVALID_OFFSET_VALUE;
        }
        newEntry.method.offsets.push_back(offsetValue);
    }
    YAML_FIELD_CHECK(locationObject, "OriginalItem", WorldLoadingError::LOCATION_MISSING_KEY);
    const auto& item = locationObject["OriginalItem"].val();
    const std::string& itemName = std::string(item.data(), item.size());
    newEntry.originalItem = {nameToGameItem(itemName), worldId};
    ITEM_VALID_CHECK(
        newEntry.originalItem.getGameItemId(),
        "Error processing location " << locationName << " in world " << std::to_string(worldId + 1) << ": Item of name " << itemName << " Does Not Exist."
    );
    return WorldLoadingError::NONE;
}

World::WorldLoadingError World::loadEventRequirement(const std::string& eventName, const std::string& logicExpression, EventAccess& eventAccess)
{
    // failure indicated by INVALID type for category
    // maybe change to Optional later if thats determined to work
    // on wii u
    // Add the world number to the end of each event to differentiate the same
    // event in different worlds
    eventAccess.event = eventName + "W" + std::to_string(worldId);
    eventAccess.worldId = worldId;
    WorldLoadingError err = WorldLoadingError::NONE;
    if((err = parseRequirementString(logicExpression, eventAccess.requirement)) != WorldLoadingError::NONE)
    {
        lastError << "| Encountered parsing event " << eventName;
        return err;
    }
    return WorldLoadingError::NONE;
}

World::WorldLoadingError World::loadLocationRequirement(const std::string& locationName, const std::string& logicExpression, LocationAccess& locAccess)
{
    // failure indicated by INVALID type for category
    // maybe change to Optional later if thats determined to work
    // on wii u
    const auto& loadedLocationId = nameToLocationId(locationName);
    LOCATION_VALID_CHECK(loadedLocationId, "Location of name \"" << locationName << "\" does not exist!");
    MAPPING_CHECK(locationName, locationIdToName(nameToLocationId(locationName)));
    locAccess.location = &locationEntries[locationIdAsIndex(loadedLocationId)];
    WorldLoadingError err = WorldLoadingError::NONE;
    if((err = parseRequirementString(logicExpression, locAccess.requirement)) != WorldLoadingError::NONE)
    {
        lastError << "| Encountered parsing location " << locationName;
        return err;
    }
    return WorldLoadingError::NONE;
}

World::WorldLoadingError World::loadExit(const std::string& connectedAreaName, const std::string& logicExpression, Entrance& loadedExit, Area& parentArea)
{
    // failure indicated by INVALID type for category
    // maybe change to Optional later if thats determined to work
    // on wii u
    auto connectedArea = nameToArea(connectedAreaName);
    AREA_VALID_CHECK(connectedArea, "Connected area of name \"" << connectedAreaName << "\" does not exist!");
    MAPPING_CHECK(connectedAreaName, areaToName(nameToArea(connectedAreaName)));
    loadedExit.setParentArea(parentArea);
    loadedExit.setConnectedArea(connectedArea);
    loadedExit.setWorldId(worldId);
    loadedExit.setWorld(this);
    WorldLoadingError err = WorldLoadingError::NONE;
    // load exit requirements
    if((err = parseRequirementString(logicExpression, loadedExit.getRequirement())) != WorldLoadingError::NONE)
    {
        lastError << "| Encountered parsing exit " << areaToName(parentArea) << " -> " << areaToName(connectedArea) << std::endl;
        return err;
    }
    return WorldLoadingError::NONE;
}

World::WorldLoadingError World::loadArea(const ryml::NodeRef& areaObject, Area& loadedArea)
{
    // failure indicated by INVALID type for category
    // maybe change to Optional later if thats determined to work
    // on wii u
    YAML_FIELD_CHECK(areaObject, "Name", WorldLoadingError::AREA_MISSING_KEY);
    const std::string areaName = substrToString(areaObject["Name"].val());
    // debugLog("Now Loading Area " + areaName);
    loadedArea = nameToArea(areaName);
    AREA_VALID_CHECK(loadedArea, "Area of name \"" << areaName << "\" does not exist!");
    MAPPING_CHECK(areaName, areaToName(nameToArea(areaName)))

    // Store the pretty name to use for later
    YAML_FIELD_CHECK(areaObject, "PrettyName", WorldLoadingError::AREA_MISSING_KEY);
    const std::string areaPrettyName = substrToString(areaObject["PrettyName"].val());
    storeNewAreaPrettyName(loadedArea, areaPrettyName);

    AreaEntry& newEntry = areaEntries[areaAsIndex(loadedArea)];
    newEntry.area = loadedArea;
    newEntry.worldId = worldId;
    WorldLoadingError err = WorldLoadingError::NONE;

    // Check to see if this area is assigned to an island
    if (areaObject.has_child("Island"))
    {
        const std::string islandName = substrToString(areaObject["Island"].val());
        auto island = nameToHintRegion(islandName);
        MAPPING_CHECK(islandName, hintRegionToName(nameToHintRegion(islandName)));
        newEntry.island = island;
    }

    // Check to see if this area is assigned to a dungeon
    if (areaObject.has_child("Dungeon"))
    {
        const std::string dungeonName = substrToString(areaObject["Dungeon"].val());
        auto dungeon = nameToHintRegion(dungeonName);
        MAPPING_CHECK(dungeonName, hintRegionToName(nameToHintRegion(dungeonName)));
        newEntry.dungeon = dungeon;
    }

    // load events and their requirements in this area if there are any
    if (areaObject.has_child("Events"))
    {
        for (const ryml::NodeRef& location : areaObject["Events"].children()) {
            EventAccess eventOut;
            const std::string eventName = substrToString(location.key());
            const std::string logicExpression = substrToString(location.val());
            err = loadEventRequirement(eventName, logicExpression, eventOut);
            if (err != World::WorldLoadingError::NONE)
            {
                std::cout << "Got error loading event: " << World::errorToName(err) << std::endl;
                return err;
            }
            // debugLog("\tAdding location " + locationIdToName(locOut));
            newEntry.events.push_back(eventOut);
        }
    }

    // load locations and their requirements in this area if there are any
    if (areaObject.has_child("Locations"))
    {
        for (const ryml::NodeRef& location : areaObject["Locations"].children()) {
            LocationAccess locOut;
            const std::string locationName = substrToString(location.key());
            const std::string logicExpression = substrToString(location.val());
            err = loadLocationRequirement(locationName, logicExpression, locOut);
            if (err != World::WorldLoadingError::NONE)
            {
                std::cout << "Got error loading location: " << World::errorToName(err) << std::endl;
                return err;
            }
            // debugLog("\tAdding location " + locationIdToName(locOut));
            newEntry.locations.push_back(locOut);
        }
    }


    // load exits in this area if there are any
    if (areaObject.has_child("Exits"))
    {
        for (const ryml::NodeRef& exit : areaObject["Exits"].children()) {
            Entrance exitOut;
            const std::string connectedAreaName = substrToString(exit.key());
            const std::string logicExpression = substrToString(exit.val());
            err = loadExit(connectedAreaName, logicExpression, exitOut, loadedArea);
            if (err != World::WorldLoadingError::NONE)
            {
                std::cout << "Got error loading exit: " << World::errorToName(err) << std::endl;
                return err;
            }
            // debugLog("\tAdding exit -> " + areaToName(exitOut.connectedArea));
            newEntry.exits.push_back(exitOut);
        }
    }

    return WorldLoadingError::NONE;
}

World::WorldLoadingError World::loadPlandomizerLocations()
{
    std::string plandoFilepath = "";
    #ifdef LOCAL_TESTING
        plandoFilepath = "../data/plandomizer.yaml";
    #endif
    #ifndef LOCAL_TESTING
        // plandoFilepath = "user/friendly/path/plandomizer.yaml"
    #endif

    std::string plandoStr;
    if (getFileContents(plandoFilepath, plandoStr) != 0)
    {
        std::cout << "Will skip using plando file" << std::endl;
        return WorldLoadingError::NONE;
    }

    const ryml::Tree plandoTree = ryml::parse_in_place(ryml::to_substr(plandoStr));
    std::string worldName = "World " + std::to_string(worldId + 1);
    ryml::substr world = ryml::to_substr(worldName);
    ryml::NodeRef plandoLocations;
    // Grab the YAML object which holds the locations for this world.
    // If there's only one world, then allow the plandomizer file to not
    // have the world specification
    for (const ryml::NodeRef& ref : plandoTree.rootref().children())
    {
        switch (numWorlds)
        {
            // If only 1 world, just look for "locations"
            case 1:
                if (ref.has_child("locations"))
                {
                    plandoLocations = ref["locations"];
                    break;
                }
                [[fallthrough]];
            // If more than one world, look for "World 1", "World 2", etc.
            default:
                if (ref.has_child(world))
                {
                    if (ref[world].has_child("locations"))
                    {
                        plandoLocations = ref[world]["locations"];
                    }
                }
                else
                {
                    debugLog("No plando locations found for world " + std::to_string(worldId + 1));
                    return WorldLoadingError::NONE;
                }
        }
    }

    for (const ryml::NodeRef& locationObject : plandoLocations.children())
    {
        if (!locationObject.has_key())
        {
            std::cout << "Plandomizer Error: One of the plando items is missing a location" << std::endl;
            return WorldLoadingError::PLANDOMIZER_ERROR;
        }
        std::string locationName = substrToString(locationObject.key());
        // If the location object has children instead of a value, then parse
        // the item name and potential world id from those children.
        // If no world id is given, then the current world's id will be used.
        int plandoWorldId = worldId;
        std::string itemName;
        if (!locationObject.has_val())
        {
            if (locationObject.has_child("item"))
            {
                itemName = substrToString(locationObject["item"].val());
            }
            else
            {
                std::cout << "Plandomizer Error: Missing key \"item\" in location \"" << locationName << "\"" << std::endl;
                return WorldLoadingError::PLANDOMIZER_ERROR;
            }
            if (locationObject.has_child("world"))
            {
                plandoWorldId = std::stoi(substrToString(locationObject["world"].val()));
                if ((size_t) plandoWorldId > numWorlds || plandoWorldId < 1)
                {
                    std::cout << "Plandomizer Error: Bad World ID \"" << std::to_string(plandoWorldId) << "\"" << std::endl;
                    std::cout << "Only " << std::to_string(numWorlds) << " worlds are being generated" << std::endl;
                    std::cout << "Valid World IDs: 1-" << std::to_string(numWorlds) << "" << std::endl;
                    return WorldLoadingError::PLANDOMIZER_ERROR;
                }
                plandoWorldId--;
            }
        }
        // Otherwise treat the value as an item for the same world as the location
        else
        {
            itemName = substrToString(locationObject.val());
        }


        // Allow pretty names for the locations since that's what people see in the
        // spoiler log and might be inclined to use
        auto locationId = prettyNameToLocationId(locationName);
        auto locationNameNoSpaces = locationName;
        if (locationId == LocationId::INVALID)
        {
            // Remove spaces from location name if it's not the pretty name
            locationNameNoSpaces.erase(std::remove_if(locationNameNoSpaces.begin(), locationNameNoSpaces.end(), [](char& c){return c == ' ';}), locationNameNoSpaces.end());
            locationId = nameToLocationId(locationNameNoSpaces);
            LOCATION_VALID_CHECK(locationId, "Plandomizer Error: Unknown location name \"" << locationName << "\" in plandomizer file");
        }

        // Remove spaces from item name
        auto itemNameNoSpaces = itemName;
        itemNameNoSpaces.erase(std::remove_if(itemNameNoSpaces.begin(), itemNameNoSpaces.end(), [](char& c){return c == ' ';}), itemNameNoSpaces.end());
        auto itemId = nameToGameItem(itemNameNoSpaces);
        ITEM_VALID_CHECK(itemId, "Plandomizer Error: Unknown item name \"" << itemName << "\" in plandomizer file");

        debugLog("Plandomizer Location for world " + std::to_string(worldId + 1) + " - " + locationName + ": " + itemName + "[W" + std::to_string(plandoWorldId) + "]");
        Location* location = &locationEntries[locationIdAsIndex(locationId)];
        Item item = {itemId, plandoWorldId};
        plandomizerLocations.insert({location, item});
    }

    return WorldLoadingError::NONE;
}

// Short function for getting the string data from a file
int World::getFileContents(const std::string& filename, std::string& fileContents)
{
    std::ifstream file(filename);
    if (!file.is_open())
    {
        std::cout << "unable to open file \"" << filename << "\" for world " << std::to_string(worldId) << std::endl;
        return 1;
    }

    // Read and load file contents
    auto ss = std::ostringstream{};
    ss << file.rdbuf();
    fileContents = ss.str();

    return 0;
}

// Load the world based on the given world graph file, macros file, and loation data file
int World::loadWorld(const std::string& worldFilePath, const std::string& macrosFilePath, const std::string& locationDataPath)
{

    std::string macrosStr;
    if (getFileContents(macrosFilePath, macrosStr) != 0)
    {
        return 1;
    }
    std::string worldStr;
    if (getFileContents(worldFilePath, worldStr) != 0)
    {
        return 1;
    }
    std::string locationDataStr;
    if (getFileContents(locationDataPath, locationDataStr) != 0)
    {
        return 1;
    }

    // Read and parse macros
    const ryml::Tree macroListTree = ryml::parse_in_place(ryml::to_substr(macrosStr));
    auto err = loadMacros(macroListTree);
    if (err != World::WorldLoadingError::NONE)
    {
        std::cout << "Got error loading macros for world " << std::to_string(worldId) << ": " << World::errorToName(err) << std::endl;
        std::cout << getLastErrorDetails() << std::endl;
        return 1;
    }

    // Read and parse location data
    const ryml::Tree locationDataTree = ryml::parse_in_place(ryml::to_substr(locationDataStr));
    for (const ryml::NodeRef& locationObject : locationDataTree.rootref().children())
    {
        LocationId locOut;
        err = loadLocation(locationObject, locOut);
        if (err != World::WorldLoadingError::NONE)
        {
            std::cout << "Got error loading location: " << World::errorToName(err) << std::endl;
            std::cout << getLastErrorDetails() << std::endl;
            return 1;
        }
    }

    // Read and load world graph
    const ryml::Tree worldDataTree = ryml::parse_in_place(ryml::to_substr(worldStr));
    for (const ryml::NodeRef& area : worldDataTree.rootref().children())
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

    // Once all areas have been loaded, create the entrance lists. This lets us
    // find assigned islands later
    for (auto& areaEntry : areaEntries)
    {
        for (auto& exit : areaEntry.exits)
        {
            exit.connect(exit.getConnectedArea());
            exit.setOriginalName();
        }
    }



    if (settings.plandomizer)
    {
        if (loadPlandomizerLocations() != WorldLoadingError::NONE)
        {
            std::cout << getLastErrorDetails() << std::endl;
            return 1;
        }
    }
    return 0;
}

Entrance& World::getEntrance(const Area& parentArea, const Area& connectedArea)
{
    auto& parentAreaEntry = areaEntries[areaAsIndex(parentArea)];

    for (auto& exit : parentAreaEntry.exits)
    {
        if (exit.getConnectedArea() == connectedArea)
        {
            return exit;
        }
    }

    std::cout << "WARNING: " << areaToName(parentArea) << " -> " << areaToName(connectedArea) << " is not a connection" << std::endl;
    return areaEntries[areaAsIndex(Area::INVALID)].exits.front();
}

void World::removeEntrance(Entrance* entranceToRemove)
{
    auto& areaExits = areaEntries[areaAsIndex(entranceToRemove->getParentArea())].exits;
    std::erase_if(areaExits, [entranceToRemove](Entrance& entrance)
    {
        return &entrance == entranceToRemove;
    });
}

EntrancePool World::getShuffleableEntrances(const EntranceType& type, const bool& onlyPrimary /*= false*/)
{
    std::vector<Entrance*> shufflableEntrances = {};

    for (auto& areaEntry : areaEntries)
    {
        for (auto& exit : areaEntry.exits)
        {
            if ((exit.getEntranceType() == type || type == EntranceType::ALL) && (!onlyPrimary || exit.isPrimary()))
            {
                shufflableEntrances.push_back(&exit);
            }
        }
    }

    return shufflableEntrances;
}

EntrancePool World::getShuffledEntrances(const EntranceType& type, const bool& onlyPrimary /*= false*/)
{
    auto entrances = getShuffleableEntrances(type, onlyPrimary);
    return filterFromPool(entrances, [](Entrance* e){return e->isShuffled();});
}

// Peforms a breadth first search to find all the islands that lead to the given
// area. In some cases of entrance randomizer, multiple islands can lead to the
// same area
std::unordered_set<HintRegion> World::getIslands(const Area& startArea)
{
    std::unordered_set<HintRegion> islands = {};
    std::unordered_set<Area> alreadyChecked = {};
    std::vector<Area> areaQueue = {startArea};

    while (!areaQueue.empty())
    {
        auto area = areaQueue.back();
        alreadyChecked.insert(area);
        areaQueue.pop_back();

        auto& areaEntry = areaEntries[areaAsIndex(area)];

        if (areaEntry.island != HintRegion::NONE)
        {
            if (area == startArea)
            {
                return {areaEntry.island};
            }
            else
            {
                // Don't search islands we've already put on the list
                islands.insert(areaEntry.island);
                continue;
            }
        }

        // If this area isn't an island, add its entrances to the queue as long
        // as they haven't been checked yet
        for (auto entrance : areaEntry.entrances)
        {
            if (alreadyChecked.count(entrance->getParentArea()) == 0)
            {
                areaQueue.push_back(entrance->getParentArea());
            }
        }
    }

    return islands;
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
    case WorldLoadingError::MACRO_MISSING_VAL:
        return "MACRO_MISSING_VAL";
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
    case WorldLoadingError::LOGIC_SYMBOL_DOES_NOT_EXIST:
        return "LOGIC_SYMBOL_DOES_NOT_EXIST";
    case WorldLoadingError::COULD_NOT_DETERMINE_TYPE:
        return "COULD_NOT_DETERMINE_TYPE";
    case WorldLoadingError::SAME_NESTING_LEVEL:
        return "SAME_NESTING_LEVEL";
    case WorldLoadingError::EXTRA_OR_MISSING_PARENTHESIS:
        return "EXTRA_OR_MISSING_PARENTHESIS";
    case WorldLoadingError::PLANDOMIZER_ERROR:
        return "PLANDOMIZER_ERROR";
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
// Use this command to generate the graph: dot -Tsvg <filename> -o world.svg
// Then, open world.svg in a browser and CTRL + F to find the area of interest
void World::dumpWorldGraph(const std::string& filename, bool onlyRandomizedExits /*= false*/)
{
    std::ofstream worldGraph;
    std::string fullFilename = "dumps/" + filename + ".gv";
    worldGraph.open (fullFilename);
    worldGraph << "digraph {\n\tcenter=true;\n";

    for (const AreaEntry& areaEntry : areaEntries) {

        std::string color = areaEntry.isAccessible ? "\"black\"" : "\"red\"";

        std::string parentName = areaToName(areaEntry.area);
        worldGraph << "\t\"" << parentName << "\"[shape=\"plain\" fontcolor=" << color << "];" << std::endl;

        // Make edge connections defined by exits
        for (const auto& exit : areaEntry.exits) {
            // Only dump shuffled exits if set
            if (!exit.isShuffled() && onlyRandomizedExits)
            {
                continue;
            }
            std::string connectedName = areaToName(exit.getConnectedArea());
            if (parentName != "INVALID" && connectedName != "INVALID"){
                worldGraph << "\t\"" << parentName << "\" -> \"" << connectedName << "\"" << std::endl;
            }
        }

        // Make edge connections between areas and their locations
        for (const auto& locAccess : areaEntry.locations) {
            std::string connectedLocation = locationIdToName(locAccess.location->locationId);
            std::string itemAtLocation = locAccess.location->currentItem.getName();
            color = locAccess.location->hasBeenFound ? "\"black\"" : "\"red\"";
            if (parentName != "INVALID" && connectedLocation != "INVALID"){
                worldGraph << "\t\"" << connectedLocation << "\"[label=<" << connectedLocation << ":<br/>" << itemAtLocation << "> shape=\"plain\" fontcolor=" << color << "];" << std::endl;
                worldGraph << "\t\"" << parentName << "\" -> \"" << connectedLocation << "\"" << "[dir=forward color=" << color << "]" << std::endl;
            }
        }
    }

    worldGraph << "}";
    worldGraph.close();
    std::cout << "Dumped world graph at " << fullFilename << std::endl;
}
