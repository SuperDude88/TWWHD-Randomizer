
#include "World.hpp"
#include "Requirements.hpp"
#include "PoolFunctions.hpp"
#include "Search.hpp"
#include "../server/command/Log.hpp"
#include "../server/utility/platform.hpp"
#include "../server/utility/stringUtil.hpp"
#include "../server/utility/file.hpp"
#include "../seedgen/random.hpp"
#include "../options.hpp"
#include <memory>
#include <algorithm>
#include <cstdlib>
#include <vector>
#include <random>

// some error checking macros for brevity and since we can't use exceptions
#define YAML_FIELD_CHECK(ref, key, err) if(ref[key].IsNone()) {lastError << "Unable to find key: \"" << key << '"'; return err;}
#define MAPPING_CHECK(str1, str2) if (str1 != str2) {lastError << "\"" << str1 << "\" does not equal" << std::endl << "\"" << str2 << "\""; LOG_ERR_AND_RETURN(WorldLoadingError::MAPPING_MISMATCH);}
#define VALID_CHECK(e, invalid, msg, err) if(e == invalid) {lastError << msg; LOG_ERR_AND_RETURN(err);}
#define EVENT_CHECK(eventName) if (!eventMap.contains(eventName)) {eventMap[eventName] = eventMap.size(); reverseEventMap[eventMap[eventName]] = eventName;}
#define ITEM_VALID_CHECK(item, msg) VALID_CHECK(item, GameItem::INVALID, msg, WorldLoadingError::GAME_ITEM_DOES_NOT_EXIST)
#define AREA_VALID_CHECK(area, msg) VALID_CHECK(0, areaEntries.count(area), msg, WorldLoadingError::AREA_DOES_NOT_EXIST)
#define REGION_VALID_CHECK(region, msg) VALID_CHECK(0, hintRegions.count(region), msg, WorldLoadingError::AREA_DOES_NOT_EXIST)
#define LOCATION_VALID_CHECK(loc, msg) VALID_CHECK(0, locationEntries.count(loc), msg, WorldLoadingError::LOCATION_DOES_NOT_EXIST)
#define OPTION_VALID_CHECK(opt, msg) VALID_CHECK(opt, Option::INVALID, msg, WorldLoadingError::OPTION_DOES_NOT_EXIST)
#define VALID_DUNGEON_CHECK(dungeon) if (!isValidDungeon(dungeon)) {ErrorLog::getInstance().log("Unrecognized dungeon name: \"" + dungeon + "\""); LOG_ERR_AND_RETURN(WorldLoadingError::INVALID_DUNGEON_NAME)};



World::World()
{

}

World::World(size_t numWorlds_)
{
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

void World::resolveRandomSettings()
{
    if (settings.pig_color == PigColor::RANDOM) {
        settings.pig_color = PigColor(Random(0, 3));
        LOG_TO_DEBUG("Random pig color chosen: " + std::to_string(static_cast<int>(settings.pig_color)));
    }
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
World::WorldLoadingError World::setItemPools()
{
    itemPool.clear();
    startingItems.clear();
    auto gameItemPool = generateGameItemPool(settings, this);
    auto startingGameItemPool = generateStartingGameItemPool(settings);

    for (const auto& item : startingGameItemPool)
    {
        ITEM_VALID_CHECK(nameToGameItem(item), item + " is not defined");
        startingItems.push_back(itemEntries[item]);
        // If a starting item is in the main item pool, replace it with some junk
        auto itemPoolItr = std::find(gameItemPool.begin(), gameItemPool.end(), item);
        if (itemPoolItr != gameItemPool.end())
        {
            auto junk = getRandomJunk();
            ITEM_VALID_CHECK(nameToGameItem(junk), junk + " is not defined");
            *itemPoolItr = junk;
        }
    }

    for (const auto& item : gameItemPool)
    {
        ITEM_VALID_CHECK(nameToGameItem(item), item + " is not defined");
        itemPool.push_back(itemEntries[item]);
    }
    #ifdef ENABLE_DEBUG
        logItemPool("Items for world " + std::to_string(worldId), itemPool);
    #endif
    return WorldLoadingError::NONE;
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
    for (auto& [name, location] : locationEntries) {
        if (!onlyProgression || location.progression)
        {
            locations.push_back(&location);
        }
    }
    return locations;
}

AreaEntry& World::getArea(const std::string& area)
{
    if (!areaEntries.contains(area))
    {
        auto message = "ERROR: Area \"" + area + "\" is not defined!";
        LOG_TO_DEBUG(message);
        ErrorLog::getInstance().log(message);
    }
    return areaEntries[area];
}

void World::determineChartMappings()
{
    LOG_TO_DEBUG("Determining Chart Mappings");
    // The ordering of this array corresponds each treasure/triforce chart with
    // the island sector it's located in and the name of the sunken treasure location
    // in that room
    std::array<GameItem, 49> charts = {
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
        GameItem::TreasureChart33 // Sector 49 Five Star Isles
    };

    // Only shuffle around the charts if we're randomizing them
    if (settings.randomize_charts)
    {
        shufflePool(charts);
    }
    LOG_TO_DEBUG("[");
    for (size_t i = 0; i < charts.size(); i++)
    {
        auto chart = charts[i];
        chartMappings[i] = chart;
        size_t sector = i + 1;

        // Set the sunken treasure location as the chain location for each treasure/triforce chart in the itemEntries
        auto locationName = roomIndexToIslandName(sector) + " - Sunken Treasure";
        auto chartName = gameItemToName(chart);
        if (!locationEntries.contains(locationName))
        {
            ErrorLog::getInstance().log("\"" + locationName + "\" is not a known sunken treasure location");
        }
        if (!itemEntries.contains(chartName))
        {
            ErrorLog::getInstance().log("\"" + chartName + "\" is not a known treasure chart item");
        }
        auto location = &locationEntries[locationName];
        itemEntries[chartName].addChainLocation(location);
        // Change the macro for this island's chart to the one at this index in the array.
        // "Chart For Island <sector number>" macros are type "HAS_ITEM" and have
        // one argument which is the chart Item.
        LOG_TO_DEBUG("\tChart for Island " + std::to_string(sector) + " is now " + gameItemToName(chart));
        macros[macroNameMap.at("Chart For Island " + std::to_string(sector))].args[0] = itemEntries[chartName];
    }
    LOG_TO_DEBUG("]");
}

// Returns whether or not the sunken treasure location has a treasure/triforce chart leading to it
bool World::chartLeadsToSunkenTreasure(const Location& location, const std::string& itemPrefix)
{
    // If this isn't a sunken treasure location, then a chart won't lead to it
    if (location.getName().find("Sunken Treasure") == std::string::npos)
    {
        LOG_TO_DEBUG("Non-sunken treasure location passed into sunken treasure check: " + location.getName());
        return false;
    }
    auto islandName = location.getName().substr(0, location.getName().find(" - Sunken Treasure"));
    size_t islandNumber = islandNameToRoomIndex(islandName);
    return gameItemToName(chartMappings[islandNumber]).find(itemPrefix) != std::string::npos;
}

void World::determineProgressionLocations()
{
    LOG_TO_DEBUG("Progression Locations: [");
    for (auto& [name, location] : locationEntries)
    {
        // If all of the location categories are set as progression, then this is a location which
        // is allowed to contain progression items (but it won't necessarily get one)
        if (std::all_of(location.categories.begin(), location.categories.end(), [this, &location = location](LocationCategory category)
        {

            if (category == LocationCategory::Junk) return false;
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
                   ( category == LocationCategory::SunkenTreasure    && this->settings.progression_triforce_charts && chartLeadsToSunkenTreasure(location, "Triforce Chart")) ||
                   ( category == LocationCategory::SunkenTreasure    && this->settings.progression_treasure_charts && chartLeadsToSunkenTreasure(location, "Treasure Chart")) ||
                   ( category == LocationCategory::ExpensivePurchase && this->settings.progression_expensive_purchases) ||
                   ( category == LocationCategory::Misc              && this->settings.progression_misc)                ||
                   ( category == LocationCategory::TingleChest       && this->settings.progression_tingle_chests)       ||
                   ( category == LocationCategory::BattleSquid       && this->settings.progression_battlesquid)         ||
                   ( category == LocationCategory::SavageLabyrinth   && this->settings.progression_savage_labyrinth)    ||
                   ( category == LocationCategory::IslandPuzzle      && this->settings.progression_island_puzzles)      ||
                   ( category == LocationCategory::Obscure           && this->settings.progression_obscure)             ||
                   ((category == LocationCategory::Platform || category == LocationCategory::Raft)    && settings.progression_platforms_rafts) ||
                   ((category == LocationCategory::BigOcto  || category == LocationCategory::Gunboat) && settings.progression_big_octos_gunboats);
        }) || location.categories.contains(LocationCategory::AlwaysProgression))
        {
            LOG_TO_DEBUG("\t" + name);
            location.progression = true;
        }
    }
    LOG_TO_DEBUG("]");
}

World::WorldLoadingError World::determineRaceModeDungeons()
{
    if (settings.race_mode)
    {
        std::vector<Dungeon> dungeonPool = {};
        for (auto& [name, dungeon] : dungeons)
        {
            // Set names now (should probably do this somewhere else, but fine for now)
            dungeon.name = name;
            // Verify that each dungeon has a race mode location
            if (dungeon.raceModeLocation == "")
            {
                ErrorLog::getInstance().log("Dungeon \"" + dungeon.name + "\" has no set race mode location");
                LOG_ERR_AND_RETURN(WorldLoadingError::DUNGEON_HAS_NO_RACE_MODE_LOCATION);
            }
            dungeonPool.push_back(dungeon);
        }

        shufflePool(dungeonPool);

        int setRaceModeDungeons = 0;
        // Loop through all the dungeons and see if any of them have items plandomized
        // within them (or within their dependent locations). If they have major items
        // plandomized, then select those dungeons as race mode dungeons
        if (settings.plandomizer)
        {
            for (const auto& dungeon : dungeonPool)
            {
                auto allDungeonLocations = dungeon.locations;
                addElementsToPool(allDungeonLocations, dungeon.outsideDependentLocations);
                for (auto& location : allDungeonLocations)
                {
                    auto dungeonLocation = &locationEntries[location];
                    bool dungeonLocationForcesRaceMode = !plandomizer.locations.contains(dungeonLocation) ? false : !plandomizer.locations[dungeonLocation].isJunkItem();
                    if (dungeonLocationForcesRaceMode)
                    {
                        // However, if the dungeon's race mode location is junk then
                        // that's an error on the user's part.
                        Location* raceModeLocation = &locationEntries[dungeon.raceModeLocation];
                        bool raceModeLocationIsAcceptable = !plandomizer.locations.contains(raceModeLocation) ? true : !plandomizer.locations[dungeonLocation].isJunkItem();
                        if (!raceModeLocationIsAcceptable)
                        {
                            ErrorLog::getInstance().log("Plandomizer Error: Junk item placed at race mode location in dungeon \"" + dungeon.name + "\" with potentially major item");
                            LOG_ERR_AND_RETURN(WorldLoadingError::PLANDOMIZER_ERROR);
                        }
                        LOG_TO_DEBUG("Chose race mode dungeon : " + dungeon.name);
                        dungeons[dungeon.name].isRaceModeDungeon = true;
                        setRaceModeDungeons++;
                        break;
                    }
                }
            }
        }

        // If too many are set, return an error
        if (setRaceModeDungeons > settings.num_race_mode_dungeons)
        {
            ErrorLog::getInstance().log("Plandomizer Error: Too many race mode locations set with potentially major items");
            ErrorLog::getInstance().log("Set race mode locations: " + std::to_string(setRaceModeDungeons));
            ErrorLog::getInstance().log("Set number of race mode dungeons: " + std::to_string(settings.num_race_mode_dungeons));
            LOG_ERR_AND_RETURN(WorldLoadingError::PLANDOMIZER_ERROR);
        }

        // Now check again and fill in any more dungeons that may be necessary
        // Also set non-race mode dungeons locations as non-progress
        for (const auto& dungeon : dungeonPool)
        {
            // If this dungeon was already selected, then skip it
            if (dungeons[dungeon.name].isRaceModeDungeon)
            {
                continue;
            }
            // If this dungeon has a junk item placed as its race mode
            // location, then skip it
            auto raceModeLocation = &locationEntries[dungeon.raceModeLocation];
            bool raceModeLocationIsAcceptable = !plandomizer.locations.contains(raceModeLocation) ? false : plandomizer.locations[raceModeLocation].isJunkItem();
            if (!raceModeLocationIsAcceptable && setRaceModeDungeons < settings.num_race_mode_dungeons)
            {
                LOG_TO_DEBUG("Chose race mode dungeon : " + dungeon.name);
                dungeons[dungeon.name].isRaceModeDungeon = true;
                setRaceModeDungeons++;
            }
            else
            {
                // If we've already chosen our race mode dungeons, then set all
                // the other dungeons' locations as non-progression. If dungeons
                // are set as progression locations, we already set them all as
                // progression previously, so here we unset those which aren't
                // progression dungeons.
                for (auto& location : dungeon.locations)
                {
                    locationEntries[location].progression = false;
                }
                // Also set any locations outside the dungeon which are dependent
                // on beating it as non-progression locations
                for (auto& location : dungeon.outsideDependentLocations)
                {
                    locationEntries[location].progression = false;
                }
            }
        }
        if (setRaceModeDungeons < settings.num_race_mode_dungeons)
        {
            ErrorLog::getInstance().log("Plandomizer Error: Not enough race mode locations for set number of race mode dungeons");
            ErrorLog::getInstance().log("Possible race mode locations: " + std::to_string(setRaceModeDungeons));
            ErrorLog::getInstance().log("Set number of race mode dungeons: " + std::to_string(settings.num_race_mode_dungeons));
            LOG_ERR_AND_RETURN(WorldLoadingError::PLANDOMIZER_ERROR);
        }
    }
    return WorldLoadingError::NONE;
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
        ErrorLog::getInstance().log("Extra or missing parenthesis within expression: \"" + str + "\"");
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
        std::replace(argStr.begin(), argStr.end(), '_', ' ');
        // First, see if we have nothing
        if (argStr == "Nothing")
        {
            req.type = RequirementType::NOTHING;
            return WorldLoadingError::NONE;
        }

        // Then an event...
        if (argStr[0] == '\'')
        {
            req.type = RequirementType::EVENT;
            std::string eventName (argStr.begin() + 1, argStr.end() - 1); // Remove quotes
            EVENT_CHECK(eventName);

            EventId eventId = eventMap[eventName] + (worldId * 10000);

            req.args.push_back(eventId);
            return WorldLoadingError::NONE;
        }

        // Then a macro...
        if (macroNameMap.contains(argStr))
        {
            req.type = RequirementType::MACRO;
            req.args.push_back(macroNameMap.at(argStr));
            return WorldLoadingError::NONE;
        }
        // Then an item...
        else if (nameToGameItem(argStr) != GameItem::INVALID)
        {
            req.type = RequirementType::HAS_ITEM;
            req.args.push_back(Item(nameToGameItem(argStr), this));
            return WorldLoadingError::NONE;
        }
        // Then a can_access check...
        else if (argStr.find("can access") != std::string::npos)
        {
            req.type = RequirementType::CAN_ACCESS;
            std::string areaName (argStr.begin() + argStr.find('(') + 1, argStr.end() - 1);
            AREA_VALID_CHECK(areaName, "Area \"" << areaName << "\" is not defined");
            req.args.push_back(areaName);
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
            auto argItem = nameToGameItem(itemName);
            ITEM_VALID_CHECK(argItem, "Game Item of name \"" << itemName << " Does Not Exist");
            req.args.push_back(count);
            req.args.push_back(itemEntries[itemName]);
            return WorldLoadingError::NONE;
        }

        ErrorLog::getInstance().log("Unrecognized logic symbol: \"" + argStr + "\"");
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
            ErrorLog::getInstance().log("Unrecognized 2 part expression: " + str);
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
        ErrorLog::getInstance().log("\"and\" & \"or\" in same nesting level when parsing \"" + str + "\"");
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
        ErrorLog::getInstance().log("Could not determine logical operator type from expression: \"" + str + "\"");
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

World::WorldLoadingError World::loadMacros(Yaml::Node& macroListTree)
{
    WorldLoadingError err = WorldLoadingError::NONE;
    uint32_t macroCount = 0;

    // first pass to get all macro names
    for (auto macroIt = macroListTree.Begin(); macroIt != macroListTree.End(); macroIt++)
    {
        auto macro = *macroIt;
        macroNameMap.emplace(macro.first, macroCount++);
    }
    for (auto macroIt = macroListTree.Begin(); macroIt != macroListTree.End(); macroIt++)
    {
        auto macro = *macroIt;
        macros.emplace_back();

        std::string logicExpression = macro.second.As<std::string>();
        if ((err = parseMacro(logicExpression, macros.back())) != WorldLoadingError::NONE)
        {
            lastError << " | Encountered parsing macro of name " << macro.first;
            return err;
        }
    }
    return WorldLoadingError::NONE;
}

World::WorldLoadingError World::loadLocation(Yaml::Node& locationObject)
{
    YAML_FIELD_CHECK(locationObject, "Names", WorldLoadingError::ITEM_MISSING_KEY);
    std::string location = locationObject["Names"]["English"].As<std::string>();

    Location& newEntry = locationEntries[location];
    // Sort locations by order of processing
    static int sortPriority = 0;
    newEntry.sortPriority = sortPriority++;
    newEntry.world = this;
    newEntry.plandomized = false;
    newEntry.categories.clear();
    for (const auto& language : Text::supported_languages)
    {
        newEntry.names[language] = Utility::Str::RemoveUnicodeReplacements(locationObject["Names"][language].As<std::string>());
    }

    // failure indicated by INVALID type for category
    // maybe change to Optional later if thats determined to work
    // on wii u
    for (auto categoryIt = locationObject["Category"].Begin(); categoryIt != locationObject["Category"].End(); categoryIt++)
    {
        Yaml::Node category = (*categoryIt).second;
        const std::string& categoryNameStr = category.As<std::string>();
        const auto& cat = nameToLocationCategory(categoryNameStr);
        if (cat == LocationCategory::INVALID)
        {
            lastError << "Encountered unknown location category \"" << categoryNameStr << "\"";
            lastError << " while parsing location " << location << " in world " << std::to_string(worldId + 1);
            return WorldLoadingError::INVALID_LOCATION_CATEGORY;
        }
        newEntry.categories.insert(cat);
    }

    const std::string& modificationTypeStr = locationObject["Type"].As<std::string>();
    const LocationModificationType modificationType = nameToModificationType(modificationTypeStr);
    if (modificationType == LocationModificationType::INVALID)
    {
        lastError << "Error processing location " << location << " in world " << std::to_string(worldId + 1) << ": ";
        lastError << "Modificaiton Type \"" << modificationTypeStr << "\" Does Not Exist";
        return WorldLoadingError::INVALID_MODIFICATION_TYPE;
    }
    switch(modificationType) {
        case LocationModificationType::Chest:
            newEntry.method = std::make_unique<ModifyChest>();
            break;
        case LocationModificationType::Actor:
            newEntry.method = std::make_unique<ModifyActor>();
            break;
        case LocationModificationType::SCOB:
            newEntry.method = std::make_unique<ModifySCOB>();
            break;
        case LocationModificationType::Event:
            newEntry.method = std::make_unique<ModifyEvent>();
            break;
        case LocationModificationType::RPX:
            newEntry.method = std::make_unique<ModifyRPX>();
            break;
        case LocationModificationType::Custom_Symbol:
            newEntry.method = std::make_unique<ModifySymbol>();
            break;
        case LocationModificationType::Boss:
            newEntry.method = std::make_unique<ModifyBoss>();
            break;
        default:
            //Should have this from the constructor
            //newEntry.method = std::make_unique<LocationModification>();
            break;
    }

    if(ModificationError err = newEntry.method->parseArgs(locationObject); err != ModificationError::NONE) {
        switch(err) {
            case ModificationError::MISSING_KEY:
                lastError << "Error processing location " << location << " in world " << std::to_string(worldId + 1) << ": ";
                lastError << "Location Key Does Not Exist";
                return WorldLoadingError::LOCATION_MISSING_KEY;
            case ModificationError::MISSING_VALUE:
                lastError << "Error processing location " << location << " in world " << std::to_string(worldId + 1) << ": ";
                lastError << "Location Value Does Not Exist";
                return WorldLoadingError::LOCATION_MISSING_VAL;
            case ModificationError::INVALID_OFFSET:
                lastError << "Error processing location " << location << " in world " << std::to_string(worldId + 1) << ": ";
                lastError << "Invalid Location Offset";
                return WorldLoadingError::INVALID_OFFSET_VALUE;
            default:
                lastError << "Error processing location " << location << " in world " << std::to_string(worldId + 1) << ": ";
                lastError << "Encountered Unknown Error";
                return WorldLoadingError::UNKNOWN;
        }
    }

    const std::string& itemName = locationObject["Original Item"].As<std::string>();
    newEntry.originalItem = {nameToGameItem(itemName), this};
    ITEM_VALID_CHECK(
        newEntry.originalItem.getGameItemId(),
        "Error processing location " << location << " in world " << std::to_string(worldId + 1) << ": Item of name " << itemName << " Does Not Exist."
    )

    const std::string& hintPriority = locationObject["Hint Priority"].As<std::string>();
    newEntry.hintPriority = hintPriority;

    // If this location is dependent on beating a dungeon, then add it to the dungeon's
    // list of outside dependent locations
    if (!locationObject["Dungeon Dependency"].IsNone())
    {
        const std::string dungeonName = locationObject["Dungeon Dependency"].As<std::string>();
        VALID_DUNGEON_CHECK(dungeonName);
        Dungeon& dungeon = dungeons[dungeonName];
        dungeon.outsideDependentLocations.push_back(newEntry.getName());
    }

    if (!locationObject["Race Mode Location"].IsNone())
    {
        newEntry.isRaceModeLocation = true;
    }

    if (!locationObject["Goal Names"].IsNone())
    {
        for (const auto& language : Text::supported_languages)
        {
            newEntry.goalNames[language] = Utility::Str::RemoveUnicodeReplacements(locationObject["Goal Names"][language].As<std::string>());
        }
    }

    // Get the message label for hint locations
    if (!locationObject["Message Label"].IsNone())
    {
        newEntry.messageLabel = locationObject["Message Label"].As<std::string>();
    }

    return WorldLoadingError::NONE;
}

World::WorldLoadingError World::loadEventRequirement(const std::string& eventName, const std::string& logicExpression, EventAccess& eventAccess)
{
    EVENT_CHECK(eventName);
    WorldLoadingError err = WorldLoadingError::NONE;
    if((err = parseRequirementString(logicExpression, eventAccess.requirement)) != WorldLoadingError::NONE)
    {
        lastError << "| Encountered parsing event " << eventName;
        return err;
    }
    eventAccess.event = eventMap[eventName] + (worldId * 10000);
    eventAccess.worldId = worldId;
    return WorldLoadingError::NONE;
}

World::WorldLoadingError World::loadLocationRequirement(const std::string& locationName, const std::string& logicExpression, LocationAccess& locAccess)
{
    // failure indicated by INVALID type for category
    // maybe change to Optional later if thats determined to work
    // on wii u
    LOCATION_VALID_CHECK(locationName, "Location of name \"" << locationName << "\" is not defined!");
    locAccess.location = &locationEntries[locationName];
    WorldLoadingError err = WorldLoadingError::NONE;
    if((err = parseRequirementString(logicExpression, locAccess.requirement)) != WorldLoadingError::NONE)
    {
        lastError << "| Encountered parsing location " << locationName;
        return err;
    }
    return WorldLoadingError::NONE;
}

World::WorldLoadingError World::loadExit(const std::string& connectedArea, const std::string& logicExpression, Entrance& loadedExit, const std::string& parentArea)
{
    // failure indicated by INVALID type for category
    // maybe change to Optional later if thats determined to work
    // on wii u
    AREA_VALID_CHECK(connectedArea, "Connected area of name \"" << connectedArea << "\" does not exist!");
    loadedExit.setParentArea(parentArea);
    loadedExit.setConnectedArea(connectedArea);
    loadedExit.setWorldId(worldId);
    loadedExit.setWorld(this);
    WorldLoadingError err = WorldLoadingError::NONE;
    // load exit requirements
    if((err = parseRequirementString(logicExpression, loadedExit.getRequirement())) != WorldLoadingError::NONE)
    {
        lastError << "| Encountered parsing exit \"" << parentArea << " -> " << connectedArea << "\"" << std::endl;
        return err;
    }
    return WorldLoadingError::NONE;
}

World::WorldLoadingError World::loadArea(Yaml::Node& areaObject)
{
    // failure indicated by INVALID type for category
    // maybe change to Optional later if thats determined to work
    // on wii u
    auto loadedArea = areaObject["Name"].As<std::string>();
    LOG_TO_DEBUG("Now Loading Area " + loadedArea);
    AREA_VALID_CHECK(loadedArea, "Area of name \"" << loadedArea << "\" is not defined!");

    AreaEntry& newEntry = getArea(loadedArea);
    newEntry.worldId = worldId;
    WorldLoadingError err = WorldLoadingError::NONE;

    // Check to see if this area is assigned to an island
    if (!areaObject["Island"].IsNone())
    {
        const std::string island = areaObject["Island"].As<std::string>();
        newEntry.island = island;
        hintRegions[island] = {};
    }

    // Check to see if this area is assigned to a dungeon
    if (!areaObject["Dungeon"].IsNone())
    {
        const std::string dungeon = areaObject["Dungeon"].As<std::string>();
        VALID_DUNGEON_CHECK(dungeon)
        newEntry.dungeon = dungeon;
        hintRegions[dungeon] = {};
    }

    // Check to see if this area is the first one in a dungeon. This is important
    // for later finding which island leads to this dungeon in race mode
    if (!areaObject["Dungeon Entrance Room"].IsNone())
    {
        const std::string dungeon = areaObject["Dungeon Entrance Room"].As<std::string>();
        VALID_DUNGEON_CHECK(dungeon)
        dungeons[dungeon].entranceRoom = loadedArea;
    }

    // Check to see if this area is assigned to a hint region
    if (!areaObject["Hint Region"].IsNone())
    {
        const std::string region = areaObject["Hint Region"].As<std::string>();
        newEntry.hintRegion = region;
        hintRegions[region] = {};
    }

    // load events and their requirements in this area if there are any
    if (areaObject["Events"].IsMap())
    {
        for (auto locationIt = areaObject["Events"].Begin(); locationIt != areaObject["Events"].End(); locationIt++) {
            auto location = *locationIt;
            EventAccess eventOut;
            const std::string eventName = location.first;
            const std::string logicExpression = location.second.As<std::string>();
            err = loadEventRequirement(eventName, logicExpression, eventOut);
            if (err != World::WorldLoadingError::NONE)
            {
                ErrorLog::getInstance().log(std::string("Got error loading event: ") + World::errorToName(err));
                return err;
            }
            LOG_TO_DEBUG("\tAdding event " + eventName);
            newEntry.events.push_back(eventOut);
        }
    }

    // load locations and their requirements in this area if there are any
    if (areaObject["Locations"].IsMap())
    {
        for (auto locationIt = areaObject["Locations"].Begin(); locationIt != areaObject["Locations"].End(); locationIt++) {
            auto location = *locationIt;
            LocationAccess locOut;
            const std::string locationName = location.first;
            LOCATION_VALID_CHECK(locationName, "Unknown location name \"" + locationName + "\" when parsing area \"" + loadedArea + "\"");
            const std::string logicExpression = location.second.As<std::string>();
            err = loadLocationRequirement(locationName, logicExpression, locOut);
            if (err != World::WorldLoadingError::NONE)
            {
                ErrorLog::getInstance().log(std::string("Got error loading location: ") + World::errorToName(err));
                return err;
            }
            LOG_TO_DEBUG("\tAdding location " + locationName);
            newEntry.locations.push_back(locOut);
            // If this area is part of a dungeon, then add any locations to that dungeon
            if (newEntry.dungeon != "")
            {
                dungeons[newEntry.dungeon].locations.push_back(locationName);
                LOG_TO_DEBUG("\t\tAdding location to dungeon " + newEntry.dungeon);
                // If it's a race mode location, set it as the dungeon's race mode location
                if (locationEntries[locationName].isRaceModeLocation)
                {
                    dungeons[newEntry.dungeon].raceModeLocation = locationName;
                }
                // Also set the location's hint region if the area has any defined
                // dungeon, island, or general hint region
                locationEntries[locationName].hintRegions = {newEntry.dungeon};
            }
            else if (newEntry.island != "")
            {
                locationEntries[locationName].hintRegions = {newEntry.island};
            }
            else if (newEntry.hintRegion != "")
            {
                locationEntries[locationName].hintRegions = {newEntry.hintRegion};
            }
        }
    }


    // load exits in this area if there are any
    if (areaObject["Exits"].IsMap())
    {
        for (auto exitIt = areaObject["Exits"].Begin(); exitIt != areaObject["Exits"].End(); exitIt++) {
            auto exit = *exitIt;
            Entrance exitOut;
            const std::string connectedAreaName = exit.first;
            const std::string logicExpression = exit.second.As<std::string>();
            err = loadExit(connectedAreaName, logicExpression, exitOut, loadedArea);
            if (err != World::WorldLoadingError::NONE)
            {
                ErrorLog::getInstance().log(std::string("Got error loading exit: ") + World::errorToName(err));
                return err;
            }
            LOG_TO_DEBUG("\tAdding exit -> " + exitOut.getConnectedArea());
            newEntry.exits.push_back(exitOut);
        }
    }

    return WorldLoadingError::NONE;
}

World::WorldLoadingError World::loadItem(Yaml::Node& itemObject)
{
    YAML_FIELD_CHECK(itemObject, "Names", WorldLoadingError::ITEM_MISSING_KEY);
    YAML_FIELD_CHECK(itemObject, "Game Item Id", WorldLoadingError::ITEM_MISSING_KEY);

    auto names = itemObject["Names"];
    const std::string itemName = names["English"].As<std::string>();
    const GameItem gameItemId = idToGameItem(std::stoul(itemObject["Game Item Id"].As<std::string>(), nullptr, 16));

    LOG_TO_DEBUG("Loading item \"" + itemName + "\". Game Item ID to name: " + gameItemToName(gameItemId));

    itemEntries[itemName] = Item(gameItemId, this);
    auto& item = itemEntries[itemName];

    // Load item names for all languages
    for (const auto& language : Text::supported_languages)
    {
        item.setName(language, Text::Type::STANDARD, itemObject["Names"][language].As<std::string>());

        if (!itemObject["Pretty Names"].IsNone())
        {
            item.setName(language, Text::Type::PRETTY, itemObject["Pretty Names"][language].As<std::string>());
        }
        else
        {
            item.setName(language, Text::Type::PRETTY, item.getUTF8Name(language, Text::Type::STANDARD));
        }

        if (!itemObject["Cryptic Names"].IsNone())
        {
            item.setName(language, Text::Type::CRYPTIC, itemObject["Cryptic Names"][language].As<std::string>());
        }
        else
        {
            item.setName(language, Text::Type::CRYPTIC, item.getUTF8Name(language, Text::Type::PRETTY));
        }

        if (!itemObject["Gender"].IsNone())
        {
            itemTranslations[gameItemId][language].gender = Text::string_to_gender(itemObject["Gender"][language].As<std::string>());
        }

        if (!itemObject["Plurality"].IsNone())
        {
            itemTranslations[gameItemId][language].plurality = Text::string_to_plurality(itemObject["Plurality"][language].As<std::string>());
        }
    }

    if (itemObject["Chain Locations"].IsSequence())
    {
        for (auto it = itemObject["Chain Locations"].Begin(); it != itemObject["Chain Locations"].End(); it++)
        {
            const Yaml::Node& locationNode = (*it).second;
            const std::string locationName = locationNode.As<std::string>();
            item.addChainLocation(&locationEntries[locationName]);
            LOG_TO_DEBUG("\"" + locationName + "\" added as chain location for \"" + itemName + "\"");
        }
    }

    if (!itemObject["Small Key Dungeon"].IsNone())
    {
        const std::string dungeon = itemObject["Small Key Dungeon"].As<std::string>();
        VALID_DUNGEON_CHECK(dungeon);
        LOG_TO_DEBUG(itemName + " is small key for " + dungeon);
        dungeons[dungeon].smallKey = itemName;
        // If this is a small key, it also should have a count
        YAML_FIELD_CHECK(itemObject, "Small Key Count", WorldLoadingError::ITEM_MISSING_KEY);
        dungeons[dungeon].keyCount = itemObject["Small Key Count"].As<int>();
        LOG_TO_DEBUG("Key count: " + std::to_string(dungeons[dungeon].keyCount));
    }

    if (!itemObject["Big Key Dungeon"].IsNone())
    {
        const std::string dungeon = itemObject["Big Key Dungeon"].As<std::string>();
        VALID_DUNGEON_CHECK(dungeon);
        LOG_TO_DEBUG(itemName + " is big key for " + dungeon);
        dungeons[dungeon].bigKey = itemName;
    }

    if (!itemObject["Map Dungeon"].IsNone())
    {
        const std::string dungeon = itemObject["Map Dungeon"].As<std::string>();
        VALID_DUNGEON_CHECK(dungeon);
        dungeons[dungeon].map = itemName;
    }

    if (!itemObject["Compass Dungeon"].IsNone())
    {
        const std::string dungeon = itemObject["Compass Dungeon"].As<std::string>();
        VALID_DUNGEON_CHECK(dungeon);
        dungeons[dungeon].compass = itemName;
    }

    return WorldLoadingError::NONE;
}

World::WorldLoadingError World::loadAreaTranslations(Yaml::Node& areaObject)
{
    YAML_FIELD_CHECK(areaObject, "Names", WorldLoadingError::AREA_MISSING_KEY);

    auto areaName = areaObject["Names"]["English"].As<std::string>();
    REGION_VALID_CHECK(areaName, "Area \"" << areaName << "\" is not a defined hint region");

    // Load area names for all languages
    for (const auto& language : Text::supported_languages)
    {
        hintRegions[areaName][language].types[Text::Type::STANDARD] = areaObject["Names"][language].As<std::string>();

        if (!areaObject["Pretty Names"].IsNone())
        {
            hintRegions[areaName][language].types[Text::Type::PRETTY] = areaObject["Pretty Names"][language].As<std::string>();
        }
        else
        {
            hintRegions[areaName][language].types[Text::Type::PRETTY] = hintRegions[areaName][language].types[Text::Type::STANDARD];
        }

        if (!areaObject["Cryptic Names"].IsNone())
        {
            hintRegions[areaName][language].types[Text::Type::CRYPTIC] = areaObject["Cryptic Names"][language].As<std::string>();
        }
        else
        {
            hintRegions[areaName][language].types[Text::Type::CRYPTIC] = hintRegions[areaName][language].types[Text::Type::PRETTY];
        }
    }

    return WorldLoadingError::NONE;
}

World::WorldLoadingError World::loadPlandomizer()
{
    LOG_TO_DEBUG("Loading plandomzier file");
    #ifdef DEVKITPRO
        const std::string plandoFilepath = "./plandomizer.yaml"; //can't bundle in the romfs, put it in the executable directory instead
    #else
        const std::string plandoFilepath = settings.plandomizerFile;
    #endif

    std::string plandoStr;
    if (Utility::getFileContents(plandoFilepath, plandoStr) != 0)
    {
        Utility::platformLog("Will skip using plando file\n");
        return WorldLoadingError::NONE;
    }

    Yaml::Node plandoTree;
    Yaml::Parse(plandoTree, plandoStr);
    std::string worldName = "World " + std::to_string(worldId + 1);
    Yaml::Node plandoLocations;
    Yaml::Node plandoEntrances;
    std::string plandoStartingIsland = "";
    // Grab the YAML object which holds the locations for this world.
    // If there's only one world, then allow the plandomizer file to not
    // have the world specification
    for (auto refIt = plandoTree.Begin(); refIt != plandoTree.End(); refIt++)
    {
        Yaml::Node& ref = (*refIt).second;
        switch (numWorlds)
        {
            // If only 1 world, just look for "locations", "entrances", etc.
            case 1:
                if (ref["locations"].IsMap())
                {
                    plandoLocations = ref["locations"];
                }
                if (ref["entrances"].IsMap())
                {
                    plandoEntrances =  ref["entrances"];
                }
                if (ref["starting island"].IsScalar())
                {
                    plandoStartingIsland = ref["starting island"].As<std::string>();
                }
                [[fallthrough]];
            // If more than one world, look for "World 1", "World 2", etc.
            default:
                if (ref[worldName].IsMap())
                {
                    if (ref[worldName]["locations"].IsMap())
                    {
                        plandoLocations = ref[worldName]["locations"];
                    }

                    if (ref[worldName]["entrances"].IsMap())
                    {
                        plandoEntrances = ref[worldName]["entrances"];
                    }
                    if (ref[worldName]["starting island"].IsScalar())
                    {
                        plandoStartingIsland = ref[worldName]["starting island"].As<std::string>();
                    }
                }
        }
        break;
    }

    // Process starting island
    if (plandoStartingIsland != "")
    {
        plandomizer.startingIslandRoomIndex = islandNameToRoomIndex(plandoStartingIsland);
        if (plandomizer.startingIslandRoomIndex == 0)
        {
            ErrorLog::getInstance().log("Plandomizer Error: Starting island name \"" + plandoStartingIsland + "\" is not recognized");
            return WorldLoadingError::PLANDOMIZER_ERROR;
        }
        else
        {
            // Automatically turn on random starting island if one is specified in
            // the plandomizer file
            settings.randomize_starting_island = true;
        }
    }

    // Process Locations
    if (!plandoLocations.IsNone())
    {
        for (auto locationIt = plandoLocations.Begin(); locationIt != plandoLocations.End(); locationIt++)
        {
            auto locationObject = *locationIt;
            if (locationObject.first.empty())
            {
                ErrorLog::getInstance().log("Plandomizer Error: One of the plando items is missing a location");
                return WorldLoadingError::PLANDOMIZER_ERROR;
            }
            // If the location object has children instead of a value, then parse
            // the item name and potential world id from those children.
            // If no world id is given, then the current world's id will be used.
            int plandoWorldId = worldId;
            std::string itemName;
            if (locationObject.second.IsMap())
            {
                if (!locationObject.second["item"].IsNone())
                {
                    itemName = locationObject.second["item"].As<std::string>();
                }
                else
                {
                    ErrorLog::getInstance().log("Plandomizer Error: Missing key \"item\" in location \"" + locationObject.first + "\"");
                    return WorldLoadingError::PLANDOMIZER_ERROR;
                }
                if (!locationObject.second["world"].IsNone())
                {
                    plandoWorldId = std::stoi(locationObject.second["world"].As<std::string>());
                    if (static_cast<size_t>(plandoWorldId) > numWorlds || plandoWorldId < 1)
                    {
                        ErrorLog::getInstance().log("Plandomizer Error: Bad World ID \"" + std::to_string(plandoWorldId) + "\"");
                        ErrorLog::getInstance().log("Only " + std::to_string(numWorlds) + " worlds are being generated");
                        ErrorLog::getInstance().log("Valid World IDs: 1-" + std::to_string(numWorlds));
                        return WorldLoadingError::PLANDOMIZER_ERROR;
                    }
                    plandoWorldId--;
                }
            }
            // Otherwise treat the value as an item for the same world as the location
            else
            {
                itemName = locationObject.second.As<std::string>();
                if (itemName == "\n" || itemName == "")
                {
                    ErrorLog::getInstance().log("Plandomizer Error: Location \"" + locationObject.first + "\" has no plandomized item.");
                    return WorldLoadingError::PLANDOMIZER_ERROR;
                }
            }

            // Get location name
            std::string locationName = locationObject.first;
            LOCATION_VALID_CHECK(locationName, "Plandomizer Error: Unknown location name \"" << locationName << "\" in plandomizer file.");

            // Get item
            auto itemId = nameToGameItem(itemName);
            ITEM_VALID_CHECK(itemId, "Plandomizer Error: Unknown item name \"" << itemName << "\" in plandomizer file");

            LOG_TO_DEBUG("Plandomizer Location for world " + std::to_string(worldId + 1) + " - " + locationName + ": " + itemName + " [W" + std::to_string(plandoWorldId) + "]");
            Location* location = &locationEntries[locationName];
            location->plandomized = true;
            Item item = itemEntries[itemName];
            plandomizer.locations.insert({location, item});
            // Place progression locations' items now to make sure that if entrance
            // randomizer is on, it creates a suitable world graph for the pre-decided
            // major item layout. Place non-progress plandomized locations later
            // so that the entrance randomizer doesn't consider potential out
            // of logic items such as extra bottles.
            if (location->progression)
            {
                location->currentItem = item;
                // Remove placed items from the world's item pool
                removeElementFromPool(itemPool, item);
            }
        }
    }

    // Process Entrances
    if (!plandoEntrances.IsNone())
    {
        for (auto entranceIt = plandoEntrances.Begin(); entranceIt != plandoEntrances.End(); entranceIt++)
        {
            auto entranceObject = *entranceIt;
            if (entranceObject.first.empty())
            {
                ErrorLog::getInstance().log("Plandomizer Error: One of the plando entrances is missing a parent entrance");
                return WorldLoadingError::PLANDOMIZER_ERROR;
            }
            // Process strings of each plando's entrance into their respective entrance
            // pointers
            const std::string originalEntranceStr = entranceObject.first;
            const std::string replacementEntranceStr = entranceObject.second.As<std::string>();

            const std::string originalTok = " -> ";
            const std::string replacementTok = " from ";

            // Verify that the format of each one is correct
            auto arrowPos = originalEntranceStr.find(originalTok);
            if (arrowPos == std::string::npos)
            {
                ErrorLog::getInstance().log("Plandomizer Error: Entrance plandomizer string \"" + originalEntranceStr + "\" is not properly formatted.");
                return WorldLoadingError::PLANDOMIZER_ERROR;
            }
            auto fromPos = replacementEntranceStr.find(replacementTok);
            if (fromPos == std::string::npos)
            {
                ErrorLog::getInstance().log("Plandomizer Error: Entrance plandomizer string \"" + replacementEntranceStr + "\" is not properly formatted.");
                return WorldLoadingError::PLANDOMIZER_ERROR;
            }

            // Separate out all the area names
            std::string originalEntranceParent = originalEntranceStr.substr(0, arrowPos);
            std::string originalEntranceConnection = originalEntranceStr.substr(arrowPos + originalTok.size());
            std::string replacementEntranceParent = replacementEntranceStr.substr(fromPos + replacementTok.size());
            std::string replacementEntranceConnection = replacementEntranceStr.substr(0, fromPos);

            Entrance* originalEntrance = getEntrance(originalEntranceParent, originalEntranceConnection);
            Entrance* replacementEntrance = getEntrance(replacementEntranceParent, replacementEntranceConnection);
            // Sanity check the entrance pointers
            if (originalEntrance == nullptr)
            {
                ErrorLog::getInstance().log("Plandomizer Error: Entrance plandomizer string \"" + originalEntranceStr + "\" is incorrect.");
                return WorldLoadingError::PLANDOMIZER_ERROR;
            }
            if (replacementEntrance == nullptr)
            {
                ErrorLog::getInstance().log("Plandomizer Error: Entrance plandomizer string \"" + replacementEntranceStr + "\" is incorrect.");
                return WorldLoadingError::PLANDOMIZER_ERROR;
            }

            plandomizer.entrances.insert({originalEntrance, replacementEntrance});
            LOG_TO_DEBUG("Plandomizer Location for world " + std::to_string(worldId + 1) + " - " + entranceObject.first + ": " + replacementEntranceStr);
        }
    }

    return WorldLoadingError::NONE;
}

// Load the world based on the given world graph file, macros file, and loation data file
int World::loadWorld(const std::string& worldFilePath, const std::string& macrosFilePath, const std::string& locationDataPath, const std::string& itemDataPath, const std::string& areaDataPath)
{
    LOG_TO_DEBUG("Loading world");
    // load and parse items
    Yaml::Node itemDataTree;
    std::string itemData;
    Utility::getFileContents(itemDataPath, itemData);
    itemData = Utility::Str::InsertUnicodeReplacements(itemData);
    Yaml::Parse(itemDataTree, itemData);
    for (auto itemIt = itemDataTree.Begin(); itemIt != itemDataTree.End(); itemIt++)
    {
        Yaml::Node& item = (*itemIt).second;
        auto err = loadItem(item);
        if (err != World::WorldLoadingError::NONE)
        {
            ErrorLog::getInstance().log(std::string("Got error loading item: ") + World::errorToName(err));
            ErrorLog::getInstance().log(getLastErrorDetails());
            return 1;
        }
    }

    // load world graph
    Yaml::Node worldDataTree;
    Yaml::Parse(worldDataTree, worldFilePath.c_str());
    // First pass to get area names
    for (auto areaIt = worldDataTree.Begin(); areaIt != worldDataTree.End(); areaIt++)
    {
        Yaml::Node area = (*areaIt).second;
        auto areaName = area["Name"].As<std::string>();
        // Construct AreaEntry object (struct) with just the name for now
        areaEntries[areaName] = {areaName};
    }

    // Read and parse macros
    Yaml::Node macroListTree;
    Yaml::Parse(macroListTree, macrosFilePath.c_str());
    auto err = loadMacros(macroListTree);
    if (err != World::WorldLoadingError::NONE)
    {
        ErrorLog::getInstance().log("Got error loading macros for world " + std::to_string(worldId) + ": " + World::errorToName(err));
        ErrorLog::getInstance().log(getLastErrorDetails());
        return 1;
    }

    // Read and parse location data
    Yaml::Node locationDataTree;
    std::string locationData;
    Utility::getFileContents(locationDataPath, locationData);
    locationData = Utility::Str::InsertUnicodeReplacements(locationData);
    Yaml::Parse(locationDataTree, locationData);
    for (auto locationObjectIt = locationDataTree.Begin(); locationObjectIt != locationDataTree.End(); locationObjectIt++)
    {
        Yaml::Node& locationObject = (*locationObjectIt).second;
        err = loadLocation(locationObject);
        if (err != World::WorldLoadingError::NONE)
        {
            ErrorLog::getInstance().log(std::string("Got error loading location: ") + World::errorToName(err));
            ErrorLog::getInstance().log(getLastErrorDetails());
            return 1;
        }
    }

    // Second pass of world graph to load each area's data
    for (auto areaIt = worldDataTree.Begin(); areaIt != worldDataTree.End(); areaIt++)
    {
        Yaml::Node area = (*areaIt).second;
        err = loadArea(area);
        if (err != World::WorldLoadingError::NONE)
        {
            ErrorLog::getInstance().log("Got error loading area for world " + std::to_string(worldId) + ": " + World::errorToName(err));
            ErrorLog::getInstance().log(getLastErrorDetails());
            return 1;
        }
    }

    // Read and parse area translations for hints/spoiler logs in other languages
    Yaml::Node areaDataTree;
    std::string areaData;
    Utility::getFileContents(areaDataPath, areaData);
    areaData = Utility::Str::InsertUnicodeReplacements(areaData);
    Yaml::Parse(areaDataTree, areaData);
    for (auto areaObjectIt = areaDataTree.Begin(); areaObjectIt != areaDataTree.End(); areaObjectIt++)
    {
        Yaml::Node& areaObject = (*areaObjectIt).second;
        err = loadAreaTranslations(areaObject);
        if (err != World::WorldLoadingError::NONE)
        {
            ErrorLog::getInstance().log(std::string("Got error loading area translations: ") + World::errorToName(err));
            ErrorLog::getInstance().log(getLastErrorDetails());
            return 1;
        }
    }

    // Once all areas have been loaded, create the entrance lists. This lets us
    // find assigned islands later
    for (auto& [name, areaEntry] : areaEntries)
    {
        for (auto& exit : areaEntry.exits)
        {
            exit.connect(exit.getConnectedArea());
            exit.setOriginalName();
        }
    }

    return 0;
}

Entrance* World::getEntrance(const std::string& parentArea, const std::string& connectedArea)
{
    // sanity check that the areas exist
    if (!areaEntries.contains(parentArea))
    {
        ErrorLog::getInstance().log("ERROR: \"" + parentArea + "\" is not a defined area!");
        return nullptr;
    }
    if (!areaEntries.contains(connectedArea))
    {
        ErrorLog::getInstance().log("ERROR: \"" + connectedArea + "\" is not a defined area!");
        return nullptr;
    }

    auto& parentAreaEntry = getArea(parentArea);

    for (auto& exit : parentAreaEntry.exits)
    {
        if (exit.getConnectedArea() == connectedArea)
        {
            return &exit;
        }
    }

    ErrorLog::getInstance().log("ERROR: " + parentArea + " -> " + connectedArea + " is not a connection!");
    return nullptr;
}

void World::removeEntrance(Entrance* entranceToRemove)
{
    std::list<Entrance>& areaExits = areaEntries[entranceToRemove->getParentArea()].exits;
    std::erase_if(areaExits, [entranceToRemove](Entrance& entrance)
    {
        return &entrance == entranceToRemove;
    });
}

EntrancePool World::getShuffleableEntrances(const EntranceType& type, const bool& onlyPrimary /*= false*/)
{
    std::vector<Entrance*> shufflableEntrances = {};

    for (auto& [name, areaEntry] : areaEntries)
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
std::unordered_set<std::string> World::getIslands(const std::string& startArea)
{
    std::unordered_set<std::string> islands = {};
    std::unordered_set<std::string> alreadyChecked = {};
    std::vector<std::string> areaQueue = {startArea};

    while (!areaQueue.empty())
    {
        auto area = areaQueue.back();
        alreadyChecked.insert(area);
        areaQueue.pop_back();

        auto& areaEntry = getArea(area);

        // Don't search through other dungeons or hint areas
        if (area != startArea && (areaEntry.dungeon != "" || areaEntry.hintRegion != ""))
        {
            continue;
        }

        if (areaEntry.island != "")
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
            if (!alreadyChecked.contains(entrance->getParentArea()))
            {
                areaQueue.push_back(entrance->getParentArea());
            }
        }
    }

    return islands;
}

Dungeon& World::getDungeon(const std::string& dungeonName)
{
    if (!dungeons.contains(dungeonName))
    {
        ErrorLog::getInstance().log("ERROR: Unknown dungeon name " + dungeonName);
    }
    return dungeons[dungeonName];
}

std::string World::getUTF8HintRegion(const std::string& hintRegion, const std::string& language /*= "English"*/, const Text::Type& type /*= Text::Type::STANDARD*/, const Text::Color& color /*= Text::Color::RAW*/) const
{
    return Utility::Str::toUTF8(getUTF16HintRegion(hintRegion, language, type, color));
}
std::u16string World::getUTF16HintRegion(const std::string& hintRegion, const std::string& language /*= "English"*/, const Text::Type& type /*= Text::Type::STANDARD*/, const Text::Color& color /*= Text::Color::RED*/) const
{
    std::u16string str = Utility::Str::toUTF16(hintRegions.at(hintRegion).at(language).types.at(type));
    str = Text::apply_name_color(str, color);
    return Utility::Str::RemoveUnicodeReplacements(str);
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
    case WorldLoadingError::ITEM_MISSING_KEY:
        return "ITEM_MISSING_KEY";
    case WorldLoadingError::REQUIREMENT_MISSING_KEY:
        return "REQUIREMENT_MISSING_KEY";
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
    case WorldLoadingError::DUNGEON_HAS_NO_RACE_MODE_LOCATION:
        return "DUNGEON_HAS_NO_RACE_MODE_LOCATION";
    case WorldLoadingError::INVALID_DUNGEON_NAME:
        return "INVALID_DUNGEON_NAME";
    default:
        return "UNKNOWN";
    }
}

std::string World::getLastErrorDetails()
{
    std::string out = lastError.str();
    lastError.str(std::string());
    lastError.clear();
    LOG_TO_DEBUG(out);
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
    std::string fullFilename =  filename + ".gv";
    worldGraph.open (fullFilename);
    worldGraph << "digraph {\n\tcenter=true;\n";

    for (auto& [name, areaEntry] : areaEntries) {

        std::string color = areaEntry.isAccessible ? "\"black\"" : "\"red\"";

        auto& parentName = areaEntry.name;
        worldGraph << "\t\"" << parentName << "\"[shape=\"plain\" fontcolor=" << color << "];" << std::endl;

        // Make edge connections defined by exits
        for (const auto& exit : areaEntry.exits) {
            // Only dump shuffled exits if set
            if (!exit.isShuffled() && onlyRandomizedExits)
            {
                continue;
            }
            auto connectedName = exit.getConnectedArea();
            if (parentName != "INVALID" && connectedName != "INVALID"){
                worldGraph << "\t\"" << parentName << "\" -> \"" << connectedName << "\"" << std::endl;
            }
        }

        // Make edge connections between areas and their locations
        for (const auto& locAccess : areaEntry.locations) {
            std::string connectedLocation = locAccess.location->getName();
            std::replace(connectedLocation.begin(), connectedLocation.end(), '&', 'A');
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
    Utility::platformLog("Dumped world graph at " + fullFilename + '\n');
}
