
#include "World.hpp"

#include <memory>
#include <algorithm>
#include <cstdlib>
#include <vector>
#include <random>
#include <iostream>

#include <logic/Requirements.hpp>
#include <logic/PoolFunctions.hpp>
#include <logic/Search.hpp>
#include <logic/flatten/flatten.hpp>
#include <command/Log.hpp>
#include <utility/platform.hpp>
#include <utility/string.hpp>
#include <utility/file.hpp>
#include <seedgen/random.hpp>
#include <options.hpp>

static std::stringstream lastError;

// some error checking macros for brevity and since we can't use exceptions
#define YAML_FIELD_CHECK(ref, key, err) if(!ref[key]) {lastError << "Unable to find key: \"" << key << '"'; return err;}
#define MAPPING_CHECK(str1, str2) if (str1 != str2) {lastError << "\"" << str1 << "\" does not equal" << std::endl << "\"" << str2 << "\""; LOG_ERR_AND_RETURN(WorldLoadingError::MAPPING_MISMATCH);}
#define VALID_CHECK(e, invalid, msg, err) if(e == invalid) {lastError << msg; LOG_ERR_AND_RETURN(err);}
#define EVENT_CHECK(eventName) if (!eventMap.contains(eventName)) {eventMap[eventName] = eventMap.size(); reverseEventMap[eventMap[eventName]] = eventName;}
#define ITEM_VALID_CHECK(item, msg) VALID_CHECK(item, GameItem::INVALID, msg, WorldLoadingError::GAME_ITEM_DOES_NOT_EXIST)
#define AREA_VALID_CHECK(area, msg) VALID_CHECK(0, areaTable.count(area), msg, WorldLoadingError::AREA_DOES_NOT_EXIST)
#define REGION_VALID_CHECK(region, msg) VALID_CHECK(0, hintRegions.count(region), msg, WorldLoadingError::AREA_DOES_NOT_EXIST)
#define LOCATION_VALID_CHECK(loc, msg) VALID_CHECK(0, locationTable.count(loc), msg, WorldLoadingError::LOCATION_DOES_NOT_EXIST)
#define OPTION_VALID_CHECK(opt, msg) VALID_CHECK(opt, Option::INVALID, msg, WorldLoadingError::OPTION_DOES_NOT_EXIST)
#define VALID_DUNGEON_CHECK(dungeon) if (!isValidDungeon(dungeon)) {ErrorLog::getInstance().log("Unrecognized dungeon name: \"" + dungeon + "\""); LOG_ERR_AND_RETURN(WorldLoadingError::INVALID_DUNGEON_NAME)};

int World::eventCounter = 0;

// Potentially set different settings for different worlds
void World::setSettings(const Settings& settings_)
{
    settings = settings_;
    originalSettings = settings;
    addSpoilsToStartingGear();
}

const Settings& World::getSettings() const
{
    return settings;
}

void World::resolveRandomSettings()
{
    if (settings.pig_color == PigColor::Random)
    {
        settings.pig_color = static_cast<PigColor>(Random(0, 3));
        LOG_TO_DEBUG("Random pig color chosen: " + PigColorToName(settings.pig_color));
    }

    if (settings.start_with_random_item)
    {
        // Default random starting item pool
        std::vector<GameItem> randomStartingItemPool = {
            GameItem::BaitBag,
            GameItem::Bombs,
            GameItem::Boomerang,
            GameItem::ProgressiveBow,
            GameItem::DekuLeaf,
            GameItem::DeliveryBag,
            GameItem::GrapplingHook,
            GameItem::Hookshot,
            GameItem::ProgressivePictoBox,
            GameItem::PowerBracelets,
            GameItem::SkullHammer,
        };

        // If the user wants a custom pool with plandomizer, load that instead
        if (!plandomizer.randomStartingItemPool.empty())
        {
            randomStartingItemPool = plandomizer.randomStartingItemPool;
        }

        // Remove items from the pool which we're already starting with
        filterAndEraseFromPool(randomStartingItemPool, [&](const auto& item){
            return elementInPool(item, settings.starting_gear);
        });

        // If we already have all the potential starting items, then don't start with one
        if (randomStartingItemPool.empty()) {
            LOG_TO_DEBUG("No items to choose from for random starting item");
        }
        else {
            GameItem startingItem = RandomElement(randomStartingItemPool);
            LOG_TO_DEBUG("Random starting item chosen: " + gameItemToName(startingItem));
            settings.starting_gear.push_back(startingItem);
        }
    }
    

    if (settings.random_item_slide_item)
    {
        std::vector<GameItem> randomItemSlidingItemPool = {
            GameItem::Boomerang,
            GameItem::ProgressiveBow,
            GameItem::GrapplingHook,
            GameItem::Hookshot
        };
        
        // Remove items from the pool which we're already starting with
        const std::vector<GameItem>& removed = filterAndEraseFromPool(randomItemSlidingItemPool, [&](const auto& item){
            return elementInPool(item, settings.starting_gear);
        });
        if(!removed.empty()) {
            LOG_TO_DEBUG("Already starting with an item sliding item, an additional one will not be chosen.");
        }
        else {
            GameItem startingItem = RandomElement(randomItemSlidingItemPool);
            LOG_TO_DEBUG("Random item sliding item chosen: " + gameItemToName(startingItem));
            settings.starting_gear.push_back(startingItem);
        }
    }
}

void World::addSpoilsToStartingGear()
{
    // Short helper function for adding the spoils to the starting_gear
    auto addSpoils = [&](const uint16_t& setting, const GameItem& gameItem){
        for (uint16_t i = 0; i < setting; i++)
        {
            settings.starting_gear.push_back(gameItem);
        }
    };

    addSpoils(settings.starting_joy_pendants, GameItem::JoyPendant);
    addSpoils(settings.starting_skull_necklaces, GameItem::SkullNecklace);
    addSpoils(settings.starting_boko_baba_seeds, GameItem::BokoBabaSeed);
    addSpoils(settings.starting_golden_feathers, GameItem::GoldenFeather);
    addSpoils(settings.starting_knights_crests, GameItem::KnightsCrest);
    addSpoils(settings.starting_red_chu_jellys, GameItem::RedChuJelly);
    addSpoils(settings.starting_green_chu_jellys, GameItem::GreenChuJelly);
    addSpoils(settings.starting_blue_chu_jellys, GameItem::BlueChuJelly);
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
        startingItems.push_back(getItem(item));
        // If a starting item is in the main item pool, replace it with some junk
        auto itemPoolItr = std::ranges::find(gameItemPool, item);
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
        itemPool.push_back(getItem(item));
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

ItemPool& World::getItemPoolReference()
{
    return itemPool;
}

ItemPool World::getStartingItems() const
{
    return startingItems;
}

ItemPool& World::getStartingItemsReference()
{
    return startingItems;
}

int World::getStartingHeartCount() const
{
    return settings.starting_hcs + (settings.starting_pohs / 4);
}

LocationPool World::getLocations(bool onlyProgression /*= false*/)
{
    LocationPool locations = {};
    for (auto& [name, location] : locationTable) {
        if (!onlyProgression || location->progression)
        {
            locations.push_back(location.get());
        }
    }
    return locations;
}

LocationPool World::getProgressionLocations()
{
    return getLocations(true);
}

size_t World::getNumOverworldProgressionLocations()
{
    return std::ranges::count_if(locationTable, [](auto& pair){return pair.second->progression && !pair.second->categories.contains(LocationCategory::Dungeon);});
}

Area* World::getArea(const std::string& area)
{
    if (!areaTable.contains(area))
    {
        auto message = "ERROR: Area \"" + area + "\" is not defined!";
        LOG_TO_DEBUG(message);
        ErrorLog::getInstance().log(message);
        return nullptr;
    }
    return areaTable[area].get();
}

void World::determineChartMappings()
{
    LOG_TO_DEBUG("Determining Chart Mappings");

    // Create a vector of charts in the vanilla order
    std::vector<GameItem> charts = {};
    for (auto i = 1; i <= 49; i++) // first island index is 1 (room 0 is sea floor)
    {
        charts.push_back(roomIndexToChart(i));
    }

    // Only shuffle around the charts if we're randomizing them
    if (settings.randomize_charts)
    {
        shufflePool(charts);
    }
    LOG_TO_DEBUG("[");
    for (auto i = 0; i < charts.size(); i++)
    {
        auto chart = charts[i];
        size_t sector = i + 1;
        chartMappings[sector] = chart;

        // Change the macro for this island's chart to the one at this index in the array.
        // "Chart For Island <sector number>" macros are type "HAS_ITEM" and have
        // one argument which is the chart Item.
        auto chartName = gameItemToName(chart);
        LOG_TO_DEBUG("\tChart for Island " + std::to_string(sector) + " is now " + chartName);
        macros[macroNameMap.at("Chart For Island " + std::to_string(sector))].args[0] = itemTable[chartName];
    }
    LOG_TO_DEBUG("]");
}

// Returns whether or not the sunken treasure location has a treasure/triforce chart leading to it
bool World::chartLeadsToSunkenTreasure(Location* location, const std::string& itemPrefix)
{
    // If this isn't a sunken treasure location, then a chart won't lead to it
    if (location->getName().find("Sunken Treasure") == std::string::npos)
    {
        LOG_TO_DEBUG("Non-sunken treasure location passed into sunken treasure check: " + location->getName());
        return false;
    }
    auto islandName = location->getName().substr(0, location->getName().find(" - Sunken Treasure"));
    size_t islandNumber = islandNameToRoomIndex(islandName);
    return gameItemToName(chartMappings[islandNumber]).find(itemPrefix) != std::string::npos;
}

World::WorldLoadingError World::determineProgressionLocations()
{

    LOG_TO_DEBUG("Progression Locations: [");
    for (auto& [name, location] : locationTable)
    {
        // If all of the location categories for a location are set as progression, then this is a location which
        // is allowed to contain progression items (but it won't necessarily get one)
        if (std::ranges::all_of(location->categories, [&location = location, this](LocationCategory category)
        {
            if (category == LocationCategory::AlwaysProgression) return true;
            return ( category == LocationCategory::Dungeon           && this->settings.progression_dungeons != ProgressionDungeons::Disabled)  ||
                   ( category == LocationCategory::GreatFairy        && this->settings.progression_great_fairies)                              ||
                   ( category == LocationCategory::PuzzleSecretCave  && this->settings.progression_puzzle_secret_caves)                        ||
                   ( category == LocationCategory::CombatSecretCave  && this->settings.progression_combat_secret_caves)                        ||
                   ( category == LocationCategory::ShortSideQuest    && this->settings.progression_short_sidequests)                           ||
                   ( category == LocationCategory::LongSideQuest     && this->settings.progression_long_sidequests)                            ||
                   ( category == LocationCategory::SpoilsTrading     && this->settings.progression_spoils_trading)                             ||
                   ( category == LocationCategory::Minigame          && this->settings.progression_minigames)                                  ||
                   ( category == LocationCategory::FreeGift          && this->settings.progression_free_gifts)                                 ||
                   ( category == LocationCategory::Mail              && this->settings.progression_mail)                                       ||
                   ( category == LocationCategory::Submarine         && this->settings.progression_submarines)                                 ||
                   ( category == LocationCategory::EyeReefChests     && this->settings.progression_eye_reef_chests)                            ||
                   ( category == LocationCategory::SunkenTreasure    && this->settings.progression_triforce_charts && chartLeadsToSunkenTreasure(location.get(), "Triforce Chart")) ||
                   ( category == LocationCategory::SunkenTreasure    && this->settings.progression_treasure_charts && chartLeadsToSunkenTreasure(location.get(), "Treasure Chart")) ||
                   ( category == LocationCategory::ExpensivePurchase && this->settings.progression_expensive_purchases)                        ||
                   ( category == LocationCategory::Misc              && this->settings.progression_misc)                                       ||
                   ( category == LocationCategory::TingleChest       && this->settings.progression_tingle_chests)                              ||
                   ( category == LocationCategory::BattleSquid       && this->settings.progression_battlesquid)                                ||
                   ( category == LocationCategory::SavageLabyrinth   && this->settings.progression_savage_labyrinth)                           ||
                   ( category == LocationCategory::IslandPuzzle      && this->settings.progression_island_puzzles)                             ||
                   ( category == LocationCategory::DungeonSecret     && this->settings.progression_dungeon_secrets)                            ||
                   ( category == LocationCategory::Obscure           && this->settings.progression_obscure)                                    ||
                   ((category == LocationCategory::Platform || category == LocationCategory::Raft)    && settings.progression_platforms_rafts) ||
                   ((category == LocationCategory::BigOcto  || category == LocationCategory::Gunboat) && settings.progression_big_octos_gunboats);
        }) && (!location->hasDungeonDependency || settings.progression_dungeons != ProgressionDungeons::Disabled)
           && (!this->settings.excluded_locations.contains(name)))
        {
            LOG_TO_DEBUG("\t" + name);
            location->progression = true;
        }
    }
    LOG_TO_DEBUG("]");
    LOG_TO_DEBUG("Manually Excluded Locations: [")
    for (const auto& locName : this->settings.excluded_locations)
    {
        LOG_TO_DEBUG("\t" + locName);
    }
    LOG_TO_DEBUG("]")

    return WorldLoadingError::NONE;
}

// Properly set the dungeons for boss room locations
// for race mode incase boss/miniboss entrances are randomized
World::WorldLoadingError World::setDungeonLocations(WorldPool& worlds)
{
    // Keep track of any unassigned race mode locations
    LocationPool unassignedRaceModeLocations = {};

    for (auto& [areaName, area] : areaTable)
    {
        for (auto& locAcc : area->locations)
        {
            auto loc = locAcc.location;
            if (loc->hintRegions.empty())
            {
                auto connectedDungeons = area->findDungeons();
                auto hintRegions = area->findHintRegions();
                if (!connectedDungeons.empty() && connectedDungeons.size() == hintRegions.size())
                {
                    auto& dungeonName = connectedDungeons.front();
                    auto& dungeon = getDungeon(dungeonName);

                    LOG_TO_DEBUG(loc->getName() + " has been assigned to dungeon " + dungeonName);
                    if (loc->isRaceModeLocation)
                    {
                        if (dungeon.raceModeLocation == nullptr) 
                        {
                            dungeon.raceModeLocation = loc;
                            dungeon.hasNaturalRaceModeLocation = true;
                        }
                        else
                        {
                            unassignedRaceModeLocations.push_back(loc);
                        }
                    }
                    dungeon.locations.emplace_back(loc);
                    loc->hintRegions.push_back(dungeonName);
                    area->dungeon = dungeonName;
                } 
                // If the race mode location doesn't have a dungeon, it's unassigned
                else if (loc->isRaceModeLocation)
                {
                    unassignedRaceModeLocations.push_back(loc);
                }
            }
        }
    }

    // For any unassigned race mode locations, randomly 
    // assign them to dungeons that don't have one,
    // but don't list them in the dungeon's locations
    for (auto& [dungeonName, dungeon] : dungeons)
    {

        if (dungeon.raceModeLocation == nullptr)
        {
            dungeon.raceModeLocation = popRandomElement(unassignedRaceModeLocations);
            LOG_TO_DEBUG("Unconnected race mode location " + dungeon.raceModeLocation->getName() + " has been assigned to dungeon " + dungeonName);
        }
    }

    // Also set each dungeon's outsideDependentLocations.
    // To do this, we'll disconnect all entrances into
    // each dungeon and test to see which locations are
    // not available without entering the dungeon
    for (auto& [dungeonName, dungeon] : dungeons)
    {
        // Disconnect any entrances into this dungeon
        std::unordered_map<Entrance*, Area*> disconnectedEntrances = {};
        for (auto& [name, area] : areaTable)
        {
            for (auto& entrance : area->exits)
            {
                if (entrance.getConnectedArea() && entrance.getConnectedArea()->dungeon == dungeonName)
                {
                    disconnectedEntrances[&entrance] = entrance.disconnect();
                }
            }
        }

        // Get locations which are now accessible
        ItemPool itemPool;
        GET_COMPLETE_ITEM_POOL(itemPool, worlds)

        // If the race mode location already has an item at it because of plandomizer, collect the item
        if (dungeon.raceModeLocation->currentItem.getGameItemId() != GameItem::INVALID)
        {
            itemPool.push_back(dungeon.raceModeLocation->currentItem);
        }

        auto accessibleLocations = search(SearchMode::AccessibleLocations, worlds, itemPool);

        // Set unaccessible progression locations as outside dependent
        for (auto& [locName, location] : locationTable)
        {
            if (!elementInPool(location.get(), accessibleLocations) && !elementInPool(location.get(), dungeon.locations) && location->progression)
            {
                LOG_TO_DEBUG(locName + " is now an outside dependent location of dungeon " + dungeonName);
                dungeon.outsideDependentLocations.push_back(location.get());
            }
        }

        // Reconnect disconnected entrances
        for (auto& [entrance, area] : disconnectedEntrances)
        {
            entrance->connect(area);
        }
    }

    return WorldLoadingError::NONE;
}

World::WorldLoadingError World::determineRaceModeDungeons(WorldPool& worlds)
{
    if (settings.progression_dungeons != ProgressionDungeons::Disabled)
    {
        std::vector<Dungeon> dungeonPool = {};
        for (auto& [name, dungeon] : dungeons)
        {
            // Set names now (should probably do this somewhere else, but fine for now)
            dungeon.name = name;
            // Verify that each dungeon has a race mode location
            if (dungeon.raceModeLocation == nullptr)
            {
                ErrorLog::getInstance().log("Dungeon \"" + dungeon.name + "\" has no set race mode location");
                LOG_ERR_AND_RETURN(WorldLoadingError::DUNGEON_HAS_NO_RACE_MODE_LOCATION);
            }
            // If the race mode location is excluded, then don't choose this dungeon
            if (!dungeon.raceModeLocation->progression)
            {
                LOG_TO_DEBUG("Dungeon \"" + dungeon.name + "\" won't be chosen as a required dungeon due to a non-progression race mode location");
                continue;
            }
            dungeonPool.push_back(dungeon);
        }

        LocationPool nonProgressRollbacks = {};
        bool successfullyChoseRaceModeDungeons = false;
        // If the user only selects dungeons as their progression category, then
        // we might need to reselect race mode dungeons a few times until we select
        // a combination that has sphere 0 locations open.
        do
        {
            shufflePool(dungeonPool);

            int setRaceModeDungeons = 0;
            // Loop through all the dungeons and see if any of them have items plandomized
            // within them (or within their dependent locations). If they have major items
            // plandomized, then select those dungeons as race mode dungeons
            if (settings.plandomizer && settings.progression_dungeons == ProgressionDungeons::RaceMode)
            {
                for (const auto& dungeon : dungeonPool)
                {
                    auto allDungeonLocations = dungeon.locations;
                    addElementsToPool(allDungeonLocations, dungeon.outsideDependentLocations);
                    for (auto dungeonLocation : allDungeonLocations)
                    {
                        if (plandomizer.locations.contains(dungeonLocation) && !plandomizer.locations[dungeonLocation].isJunkItem())
                        {
                            // However, if the dungeon's naturally assigned race mode location is junk then
                            // that's an error on the user's part.
                            Location* raceModeLocation = dungeon.raceModeLocation;
                            bool raceModeLocationIsAcceptable = !plandomizer.locations.contains(raceModeLocation) || !plandomizer.locations[dungeonLocation].isJunkItem();
                            if (dungeon.hasNaturalRaceModeLocation && !raceModeLocationIsAcceptable)
                            {
                                ErrorLog::getInstance().log("Plandomizer Error: Junk item placed at race mode location in dungeon \"" + dungeon.name + "\" with potentially major item");
                                LOG_ERR_AND_RETURN(WorldLoadingError::PLANDOMIZER_ERROR);
                            }
                            LOG_TO_DEBUG("Chose race mode dungeon : " + dungeon.name);
                            dungeons[dungeon.name].isRequiredDungeon = true;
                            setRaceModeDungeons++;
                            break;
                        }
                    }
                }
            }

            // If too many are set, return an error
            if (setRaceModeDungeons > settings.num_required_dungeons)
            {
                ErrorLog::getInstance().log("Plandomizer Error: Too many race mode locations set with potentially major items");
                ErrorLog::getInstance().log("Set race mode locations: " + std::to_string(setRaceModeDungeons));
                ErrorLog::getInstance().log("Set number of race mode dungeons: " + std::to_string(settings.num_required_dungeons));
                LOG_ERR_AND_RETURN(WorldLoadingError::PLANDOMIZER_ERROR);
            }

            // Now check again and fill in any more dungeons that may be necessary
            // Also set non-race mode dungeons locations as non-progress
            for (const auto& dungeon : dungeonPool)
            {
                // If this dungeon was already selected, then skip it
                if (dungeons[dungeon.name].isRequiredDungeon)
                {
                    continue;
                }
                // If this dungeon has a junk item placed as its race mode
                // location, then skip it
                auto raceModeLocation = dungeon.raceModeLocation;
                bool raceModeLocationIsAcceptable = !plandomizer.locations.contains(raceModeLocation) ? false : plandomizer.locations[raceModeLocation].isJunkItem() || dungeon.hasNaturalRaceModeLocation;
                if (!raceModeLocationIsAcceptable && setRaceModeDungeons < settings.num_required_dungeons)
                {
                    LOG_TO_DEBUG("Chose race mode dungeon : " + dungeon.name);
                    dungeons[dungeon.name].isRequiredDungeon = true;
                    setRaceModeDungeons++;
                }
                else if (settings.progression_dungeons == ProgressionDungeons::RaceMode)
                {
                    // If we've already chosen our race mode dungeons, then set all
                    // the other dungeons' locations as non-progression. If dungeons
                    // are set as progression locations, we already set them all as
                    // progression previously, so here we unset those which aren't
                    // progression dungeons.
                    for (auto location : dungeon.locations)
                    {
                        location->progression = false;
                        nonProgressRollbacks.push_back(location);
                    }

                    // Dungeons without a naturally assigned race mode location won't
                    // have their race mode location in their list of locations, so
                    // manually add it to the non-progress checks
                    if (!dungeon.hasNaturalRaceModeLocation)
                    {
                        nonProgressRollbacks.push_back(dungeon.raceModeLocation);
                    }

                    // Also set any progress locations outside the dungeon which
                    // are dependent on accessing it as non-progression locations
                    for (auto location : dungeon.outsideDependentLocations)
                    {
                        if (location->progression)
                        {
                            location->progression = false;
                            nonProgressRollbacks.push_back(location);
                        }
                    }
                }
            }

            ItemPool noItems;
            LocationPool allowedLocations;
            GET_COMPLETE_PROGRESSION_LOCATION_POOL(allowedLocations, worlds);
            // If we have locations available, flag a successful choosing
            if (!getAccessibleLocations(worlds, noItems, allowedLocations).empty())
            {
                successfullyChoseRaceModeDungeons = true;

                if (setRaceModeDungeons < settings.num_required_dungeons)
                {
                    ErrorLog::getInstance().log("Plandomizer Error: Not enough race mode locations for set number of race mode dungeons");
                    ErrorLog::getInstance().log("Possible race mode locations: " + std::to_string(setRaceModeDungeons));
                    ErrorLog::getInstance().log("Set number of race mode dungeons: " + std::to_string(settings.num_required_dungeons));
                    LOG_ERR_AND_RETURN(WorldLoadingError::PLANDOMIZER_ERROR);
                }
            }
            // Otherwise, set all dungeon locations as progression and try again
            else
            {
                LOG_TO_DEBUG("No sphere 0 progression locations with chosen race mode dungeon set");
                for (auto location : nonProgressRollbacks)
                {
                    location->progression = true;
                }
                for (auto& [name, dungeon] : dungeons)
                {
                    dungeon.isRequiredDungeon = false;
                }

                nonProgressRollbacks.clear();
            }
        } while (!successfullyChoseRaceModeDungeons);
    }
    return WorldLoadingError::NONE;
}

RequirementError World::parseMacro(const std::string& macroLogicExpression, Requirement& reqOut)
{
    RequirementError err = RequirementError::NONE;
    // readd prechecks?
    if ((err = parseRequirementString(macroLogicExpression, reqOut, this)) != RequirementError::NONE) return err;
    return RequirementError::NONE;
}

World::WorldLoadingError World::loadMacros(const YAML::Node& macroListTree)
{
    RequirementError err = RequirementError::NONE;
    uint32_t macroCount = 0;

    // first pass to get all macro names
    for (const auto& macro : macroListTree)
    {
        macroStrings.emplace(macro.first.as<std::string>(), macro.second.as<std::string>());
        macroNameMap.emplace(macro.first.as<std::string>(), macroCount);
        macroNames.emplace(macroCount, macro.first.as<std::string>());
        macroCount++;
    }
    for (const auto& macro : macroListTree)
    {
        macros.emplace_back();

        if ((err = parseMacro(macro.second.as<std::string>(), macros.back())) != RequirementError::NONE)
        {
            lastError << " | Encountered parsing macro of name " << macro.first.as<std::string>();
            return WorldLoadingError::BAD_REQUIREMENT;
        }
    }
    return WorldLoadingError::NONE;
}

World::WorldLoadingError World::loadLocation(const YAML::Node& locationObject)
{
    YAML_FIELD_CHECK(locationObject, "Names", WorldLoadingError::ITEM_MISSING_KEY);
    const auto locationName = locationObject["Names"]["English"].as<std::string>();

    addLocation(locationName);
    Location* location = locationTable[locationName].get();
    // Sort locations by order of processing
    static int sortPriority = 0;
    location->sortPriority = sortPriority++;
    location->world = this;
    location->plandomized = false;
    location->categories.clear();
    for (const auto& language : Text::supported_languages)
    {
        location->names[language] = locationObject["Names"][language].as<std::string>();
    }

    // failure indicated by INVALID type for category
    // maybe change to Optional later if thats determined to work
    // on wii u
    for (const auto& category : locationObject["Category"])
    {
        const std::string& categoryNameStr = category.as<std::string>();
        const auto& cat = nameToLocationCategory(categoryNameStr);
        if (cat == LocationCategory::INVALID)
        {
            lastError << "Encountered unknown location category \"" << categoryNameStr << "\"";
            lastError << " while parsing location " << locationName << " in world " << std::to_string(worldId + 1);
            return WorldLoadingError::INVALID_LOCATION_CATEGORY;
        }
        location->categories.insert(cat);
    }

    const std::string& modificationTypeStr = locationObject["Type"].as<std::string>();
    const LocationModificationType modificationType = nameToModificationType(modificationTypeStr);
    if (modificationType == LocationModificationType::INVALID)
    {
        lastError << "Error processing location " << locationName << " in world " << std::to_string(worldId + 1) << ": ";
        lastError << "Modificaiton Type \"" << modificationTypeStr << "\" Does Not Exist";
        return WorldLoadingError::INVALID_MODIFICATION_TYPE;
    }
    switch(modificationType) {
        case LocationModificationType::Chest:
            location->method = std::make_unique<ModifyChest>();
            break;
        case LocationModificationType::Actor:
            location->method = std::make_unique<ModifyActor>();
            break;
        case LocationModificationType::SCOB:
            location->method = std::make_unique<ModifySCOB>();
            break;
        case LocationModificationType::Event:
            location->method = std::make_unique<ModifyEvent>();
            break;
        case LocationModificationType::RPX:
            location->method = std::make_unique<ModifyRPX>();
            break;
        case LocationModificationType::Custom_Symbol:
            location->method = std::make_unique<ModifySymbol>();
            break;
        case LocationModificationType::Boss:
            location->method = std::make_unique<ModifyBoss>();
            break;
        default:
            //Should have this from the constructor
            //location.method = std::make_unique<LocationModification>();
            break;
    }

    if(ModificationError err = location->method->parseArgs(locationObject); err != ModificationError::NONE) {
        switch(err) {
            case ModificationError::MISSING_KEY:
                lastError << "Error processing location " << locationName << " in world " << std::to_string(worldId + 1) << ": ";
                lastError << "Location Key Does Not Exist";
                return WorldLoadingError::LOCATION_MISSING_KEY;
            case ModificationError::MISSING_VALUE:
                lastError << "Error processing location " << locationName << " in world " << std::to_string(worldId + 1) << ": ";
                lastError << "Location Value Does Not Exist";
                return WorldLoadingError::LOCATION_MISSING_VAL;
            case ModificationError::INVALID_OFFSET:
                lastError << "Error processing location " << locationName << " in world " << std::to_string(worldId + 1) << ": ";
                lastError << "Invalid Location Offset";
                return WorldLoadingError::INVALID_OFFSET_VALUE;
            default:
                lastError << "Error processing location " << locationName << " in world " << std::to_string(worldId + 1) << ": ";
                lastError << "Encountered Unknown Error";
                return WorldLoadingError::UNKNOWN;
        }
    }

    const std::string& itemName = locationObject["Original Item"].as<std::string>();
    location->originalItem = itemTable[itemName];
    ITEM_VALID_CHECK(
        location->originalItem.getGameItemId(),
        "Error processing location " << locationName << " in world " << std::to_string(worldId + 1) << ": Item of name " << itemName << " Does Not Exist."
    )

    const std::string& hintPriority = locationObject["Hint Priority"].as<std::string>();
    location->hintPriority = hintPriority;

    if (locationObject["Race Mode Location"])
    {
        location->isRaceModeLocation = true;
        raceModeLocations.push_back(location);
    }

    if (locationObject["Goal Names"])
    {
        for (const auto& language : Text::supported_languages)
        {
            location->goalNames[language] = locationObject["Goal Names"][language].as<std::string>();
        }
    }

    // Get the message label for hint locations
    if (locationObject["Message Label"])
    {
        location->messageLabel = locationObject["Message Label"].as<std::string>();
    }

    // Get the tracker note and note areas for this location if it has them
    if (locationObject["Tracker Data"])
    {
        if (locationObject["Tracker Data"]["Note"])
        {
            location->trackerNote = locationObject["Tracker Data"]["Note"].as<std::string>("");
        }
        if (locationObject["Tracker Data"]["Note Areas"])
        {
            for (const auto& area : locationObject["Tracker Data"]["Note Areas"])
            {
                auto areaStr = area.as<std::string>("");
                location->trackerNoteAreas.insert(areaStr);
            }
        }
    }

    return WorldLoadingError::NONE;
}

World::WorldLoadingError World::loadEventRequirement(const std::string& eventName, const std::string& logicExpression, EventAccess& eventAccess)
{
    addEvent(eventName);
    RequirementError err = RequirementError::NONE;
    if((err = parseRequirementString(logicExpression, eventAccess.requirement, this)) != RequirementError::NONE)
    {
        lastError << "| Encountered parsing event " << eventName;
        return WorldLoadingError::BAD_REQUIREMENT;
    }
    eventAccess.event = eventMap[eventName];
    eventAccess.world = this;
    return WorldLoadingError::NONE;
}

World::WorldLoadingError World::loadLocationRequirement(const std::string& locationName, const std::string& logicExpression, LocationAccess& locAccess)
{
    // failure indicated by INVALID type for category
    // maybe change to Optional later if thats determined to work
    // on wii u
    LOCATION_VALID_CHECK(locationName, "Location of name \"" << locationName << "\" is not defined!");
    locAccess.location = locationTable[locationName].get();
    RequirementError err = RequirementError::NONE;
    if((err = parseRequirementString(logicExpression, locAccess.requirement, this)) != RequirementError::NONE)
    {
        lastError << "| Encountered parsing location " << locationName;
        return WorldLoadingError::BAD_REQUIREMENT;
    }
    return WorldLoadingError::NONE;
}

World::WorldLoadingError World::loadExit(const std::string& connectedArea, const std::string& logicExpression, Entrance& loadedExit, const std::string& parentArea)
{
    // failure indicated by INVALID type for category
    // maybe change to Optional later if thats determined to work
    // on wii u
    AREA_VALID_CHECK(connectedArea, "Connected area of name \"" << connectedArea << "\" does not exist!");
    loadedExit.setParentArea(getArea(parentArea));
    loadedExit.setConnectedArea(getArea(connectedArea));
    loadedExit.setWorldId(worldId);
    loadedExit.setWorld(this);
    RequirementError err = RequirementError::NONE;
    // load exit requirements
    if((err = parseRequirementString(logicExpression, loadedExit.getRequirement(), this)) != RequirementError::NONE)
    {
        lastError << "| Encountered parsing exit \"" << parentArea << " -> " << connectedArea << "\"" << std::endl;
        return WorldLoadingError::BAD_REQUIREMENT;
    }
    return WorldLoadingError::NONE;
}

World::WorldLoadingError World::loadArea(const YAML::Node& areaObject)
{
    // failure indicated by INVALID type for category
    // maybe change to Optional later if thats determined to work
    // on wii u
    auto loadedArea = areaObject["Name"].as<std::string>();
    LOG_TO_DEBUG("Now Loading Area " + loadedArea);
    AREA_VALID_CHECK(loadedArea, "Area of name \"" << loadedArea << "\" is not defined!");

    auto area = getArea(loadedArea);
    area->name = loadedArea;
    area->world = this;
    WorldLoadingError err = WorldLoadingError::NONE;

    // Check to see if this area is assigned to an island
    if (areaObject["Island"])
    {
        const auto island = areaObject["Island"].as<std::string>();
        area->island = island;
        hintRegions[island] = {};
    }

    // Check to see if this area is assigned to a dungeon
    if (areaObject["Dungeon"])
    {
        const auto dungeon = areaObject["Dungeon"].as<std::string>();
        VALID_DUNGEON_CHECK(dungeon)
        area->dungeon = dungeon;
        hintRegions[dungeon] = {};
    }

    // Check to see if this area is the first one in a dungeon. This is important
    // for later finding which island leads to this dungeon in race mode
    if (areaObject["Dungeon Starting Room"])
    {
        const auto dungeon = areaObject["Dungeon Starting Room"].as<std::string>();
        VALID_DUNGEON_CHECK(dungeon)
        dungeons[dungeon].startingArea = area;
    }
    // If this is a dungeon area, but not the starting entrance, add a logical
    // exit to the dungeon entrance to mimic savewarping
    else if (areaObject["Dungeon"])
    {
        auto startingArea = this->getDungeon(area->dungeon).startingArea;
        Entrance exitOut;
        err = loadExit(startingArea->name, "Nothing", exitOut, loadedArea);
        if (err != WorldLoadingError::NONE)
        {
            ErrorLog::getInstance().log(std::string("Got error loading exit: ") + errorToName(err));
            return err;
        }
        LOG_TO_DEBUG("\tAdding exit -> " + exitOut.getConnectedArea()->name + " (Savewarp)");
        area->exits.push_back(exitOut);
    }

    // Check to see if this area is assigned to a hint region
    if (areaObject["Hint Region"])
    {
        const auto region = areaObject["Hint Region"].as<std::string>();
        area->hintRegion = region;
        hintRegions[region] = {};
    }

    // load events and their requirements in this area if there are any
    if (areaObject["Events"])
    {
        for (const auto& event : areaObject["Events"]) {
            EventAccess eventOut;
            const auto eventName = event.first.as<std::string>();
            const auto logicExpression = event.second.as<std::string>();
            err = loadEventRequirement(eventName, logicExpression, eventOut);
            if (err != WorldLoadingError::NONE)
            {
                ErrorLog::getInstance().log(std::string("Got error loading event: ") + errorToName(err));
                return err;
            }
            LOG_TO_DEBUG("\tAdding event " + eventName);
            eventOut.area = area;
            area->events.push_back(eventOut);
        }
    }

    // load locations and their requirements in this area if there are any
    if (areaObject["Locations"])
    {
        for (const auto& locationNode : areaObject["Locations"]) {
            LocationAccess locOut;
            locOut.area = area;
            const auto locationName = locationNode.first.as<std::string>();
            LOCATION_VALID_CHECK(locationName, "Unknown location name \"" + locationName + "\" when parsing area \"" + loadedArea + "\"");
            err = loadLocationRequirement(locationName, locationNode.second.as<std::string>(), locOut);
            if (err != WorldLoadingError::NONE)
            {
                ErrorLog::getInstance().log(std::string("Got error loading location: ") + errorToName(err));
                return err;
            }
            LOG_TO_DEBUG("\tAdding location " + locationName);
            area->locations.push_back(locOut);
            auto location = locationTable[locationName].get();
            location->accessPoints.push_back(&area->locations.back());
            // If this area is part of a dungeon, then add any locations to that dungeon
            if (area->dungeon != "")
            {
                dungeons[area->dungeon].locations.push_back(location);
                LOG_TO_DEBUG("\t\tAdding location to dungeon " + area->dungeon);
                // Also set the location's hint region if the area has any defined
                // dungeon, island, or general hint region
                location->hintRegions = {area->dungeon};
            }
            else if (area->island != "")
            {
                location->hintRegions = {area->island};
            }
            else if (area->hintRegion != "")
            {
                location->hintRegions = {area->hintRegion};
            }
        }
    }


    // load exits in this area if there are any
    if (areaObject["Exits"])
    {
        for (const auto& exit : areaObject["Exits"]) {
            Entrance exitOut;
            err = loadExit(exit.first.as<std::string>(), exit.second.as<std::string>(), exitOut, loadedArea);
            if (err != WorldLoadingError::NONE)
            {
                ErrorLog::getInstance().log(std::string("Got error loading exit: ") + errorToName(err));
                return err;
            }
            LOG_TO_DEBUG("\tAdding exit -> " + exitOut.getConnectedArea()->name);
            area->exits.push_back(exitOut);
        }
    }

    return WorldLoadingError::NONE;
}

World::WorldLoadingError World::loadItem(const YAML::Node& itemObject)
{
    YAML_FIELD_CHECK(itemObject, "Names", WorldLoadingError::ITEM_MISSING_KEY);
    YAML_FIELD_CHECK(itemObject, "Game Item Id", WorldLoadingError::ITEM_MISSING_KEY);

    auto names = itemObject["Names"];
    const auto itemName = names["English"].as<std::string>();
    const GameItem gameItemId = idToGameItem(itemObject["Game Item Id"].as<uint8_t>());

    LOG_TO_DEBUG("Loading item \"" + itemName + "\". Game Item ID to name: " + gameItemToName(gameItemId));

    itemTable[itemName] = Item(gameItemId, this);
    auto& item = itemTable[itemName];

    // Load item names for all languages
    for (const auto& language : Text::supported_languages)
    {
        item.setName(language, Text::Type::STANDARD, itemObject["Names"][language].as<std::string>());

        if (itemObject["Pretty Names"])
        {
            item.setName(language, Text::Type::PRETTY, itemObject["Pretty Names"][language].as<std::string>());
        }
        else
        {
            item.setName(language, Text::Type::PRETTY, item.getUTF8Name(language, Text::Type::STANDARD));
        }

        if (itemObject["Cryptic Names"])
        {
            item.setName(language, Text::Type::CRYPTIC, itemObject["Cryptic Names"][language].as<std::string>());
        }
        else
        {
            item.setName(language, Text::Type::CRYPTIC, item.getUTF8Name(language, Text::Type::PRETTY));
        }

        if (itemObject["Gender"])
        {
            itemTranslations[gameItemId][language].gender = Text::string_to_gender(itemObject["Gender"][language].as<std::string>());
        }

        if (itemObject["Plurality"])
        {
            itemTranslations[gameItemId][language].plurality = Text::string_to_plurality(itemObject["Plurality"][language].as<std::string>());
        }
    }

    if (itemObject["Small Key Dungeon"])
    {
        const auto dungeon = itemObject["Small Key Dungeon"].as<std::string>();
        VALID_DUNGEON_CHECK(dungeon);
        LOG_TO_DEBUG(itemName + " is small key for " + dungeon);
        dungeons[dungeon].smallKey = getItem(itemName);
        // If this is a small key, it also should have a count
        YAML_FIELD_CHECK(itemObject, "Small Key Count", WorldLoadingError::ITEM_MISSING_KEY);
        dungeons[dungeon].keyCount = itemObject["Small Key Count"].as<int>();
        LOG_TO_DEBUG("Key count: " + std::to_string(dungeons[dungeon].keyCount));
    }

    if (itemObject["Big Key Dungeon"])
    {
        const auto dungeon = itemObject["Big Key Dungeon"].as<std::string>();
        VALID_DUNGEON_CHECK(dungeon);
        LOG_TO_DEBUG(itemName + " is big key for " + dungeon);
        dungeons[dungeon].bigKey = getItem(itemName);
    }

    if (itemObject["Map Dungeon"])
    {
        const auto dungeon = itemObject["Map Dungeon"].as<std::string>();
        VALID_DUNGEON_CHECK(dungeon);
        dungeons[dungeon].map = getItem(itemName);
    }

    if (itemObject["Compass Dungeon"])
    {
        const auto dungeon = itemObject["Compass Dungeon"].as<std::string>();
        VALID_DUNGEON_CHECK(dungeon);
        dungeons[dungeon].compass = getItem(itemName);
    }

    return WorldLoadingError::NONE;
}

World::WorldLoadingError World::loadAreaTranslations(const YAML::Node& areaObject)
{
    YAML_FIELD_CHECK(areaObject, "Names", WorldLoadingError::AREA_MISSING_KEY);

    auto areaName = areaObject["Names"]["English"].as<std::string>();
    REGION_VALID_CHECK(areaName, "Area \"" << areaName << "\" is not a defined hint region");

    // Load area names for all languages
    for (const auto& language : Text::supported_languages)
    {
        hintRegions[areaName][language].types[Text::Type::STANDARD] = areaObject["Names"][language].as<std::string>();

        if (areaObject["Pretty Names"])
        {
            hintRegions[areaName][language].types[Text::Type::PRETTY] = areaObject["Pretty Names"][language].as<std::string>();
        }
        else
        {
            hintRegions[areaName][language].types[Text::Type::PRETTY] = hintRegions[areaName][language].types[Text::Type::STANDARD];
        }

        if (areaObject["Cryptic Names"])
        {
            hintRegions[areaName][language].types[Text::Type::CRYPTIC] = areaObject["Cryptic Names"][language].as<std::string>();
        }
        else
        {
            hintRegions[areaName][language].types[Text::Type::CRYPTIC] = hintRegions[areaName][language].types[Text::Type::PRETTY];
        }
    }

    return WorldLoadingError::NONE;
}

World::WorldLoadingError World::loadDungeonExitInfo()
{
    std::string dungeonExitData;
    Utility::getFileContents(Utility::get_data_path() / "logic/dungeon_entrance_info.yaml", dungeonExitData, true);
    YAML::Node dungeonExitTree = YAML::Load(dungeonExitData);

    for (const auto& dungeonExitData : dungeonExitTree)
    {
        auto dungeonName = dungeonExitData.first.as<std::string>();
        auto& dungeon = getDungeon(dungeonName);
        auto& exitData = dungeonExitData.second;
        YAML_FIELD_CHECK(exitData, "Savewarp", WorldLoadingError::DUNGEON_MISSING_KEY);
        YAML_FIELD_CHECK(exitData, "Wind Warp Exit",  WorldLoadingError::DUNGEON_MISSING_KEY);

        auto& savewarpData = exitData["Savewarp"];
        auto& windWarpData = exitData["Wind Warp Exit"];

        YAML_FIELD_CHECK(savewarpData, "stage", WorldLoadingError::DUNGEON_MISSING_KEY);
        YAML_FIELD_CHECK(savewarpData, "room", WorldLoadingError::DUNGEON_MISSING_KEY);
        YAML_FIELD_CHECK(savewarpData, "spawn", WorldLoadingError::DUNGEON_MISSING_KEY);
        YAML_FIELD_CHECK(windWarpData, "stage", WorldLoadingError::DUNGEON_MISSING_KEY);
        YAML_FIELD_CHECK(windWarpData, "room", WorldLoadingError::DUNGEON_MISSING_KEY);
        YAML_FIELD_CHECK(windWarpData, "spawn", WorldLoadingError::DUNGEON_MISSING_KEY);

        dungeon.windWarpExitStage = windWarpData["stage"].as<std::string>();
        dungeon.windWarpExitRoom = windWarpData["room"].as<uint8_t>();
        dungeon.windWarpExitSpawn = windWarpData["spawn"].as<uint8_t>();
        dungeon.savewarpStage = savewarpData["stage"].as<std::string>();
        dungeon.savewarpRoom = savewarpData["room"].as<uint8_t>();
        dungeon.savewarpSpawn = savewarpData["spawn"].as<uint8_t>();
    }

    return WorldLoadingError::NONE;
}

World::WorldLoadingError World::processPlandomizerLocations(WorldPool& worlds)
{
    // Process Locations
    for (auto& [locationName, plandoItem] : plandomizer.locationsStr)
    {
        LOCATION_VALID_CHECK(locationName, "Plandomizer Error: Unknown location name \"" << locationName << "\" in plandomizer file.");

        auto itemName = gameItemToName(plandoItem.gameItem);
        auto& plandoWorldId = plandoItem.world;
        World& itemsWorld = worlds[plandoWorldId];

        LOG_TO_DEBUG("Plandomizer Location for world " + std::to_string(worldId + 1) + " - " + locationName + ": " + itemName + " [W" + std::to_string(plandoWorldId + 1) + "]");
        Location* location = locationTable[locationName].get();

        if (location->hasKnownVanillaItem)
        {
            ErrorLog::getInstance().log("Plandomizer Error: Attempted to plandomize item \"" + itemName + "\" at a location \"" + locationName + "\" which already has vanilla item \"" + location->currentItem.getName() + "\"");
            return WorldLoadingError::PLANDOMIZER_ERROR;
        }

        location->plandomized = true;
        Item item = itemsWorld.getItem(itemName);
        plandomizer.locations.insert({location, item});
        // Place progression locations' items now to make sure that if entrance
        // randomizer is on, it creates a suitable world graph for the pre-decided
        // major item layout. Place non-progress plandomized locations later
        // so that the entrance randomizer doesn't consider potential out
        // of logic items such as extra bottles.
        if (location->progression)
        {
            location->currentItem = item;
            LOG_TO_DEBUG("Plandomized " + itemName + " at " + locationName);
            // Remove placed items from the item's world's item pool
            removeElementFromPool(itemsWorld.getItemPoolReference(), item);
        }
    }

    return WorldLoadingError::NONE;
}

// Load the world based on the given world graph file, macros file, loation data file, item data file, and area data file
int World::loadWorld(const fspath& worldFilePath, const fspath& macrosFilePath, const fspath& locationDataPath, const fspath& itemDataPath, const fspath& areaDataPath)
{
    LOG_TO_DEBUG("Loading world");
    // load and parse items
    std::string itemData;
    Utility::getFileContents(itemDataPath, itemData, true);
    YAML::Node itemDataTree = YAML::Load(itemData);
    for (const auto& item : itemDataTree)
    {
        auto err = loadItem(item);
        if (err != World::WorldLoadingError::NONE)
        {
            ErrorLog::getInstance().log(std::string("Got error loading item: ") + errorToName(err));
            ErrorLog::getInstance().log(getLastErrorDetails());
            return 1;
        }
    }

    // load world graph
    std::string worldData;
    Utility::getFileContents(worldFilePath, worldData, true);
    YAML::Node worldDataTree = YAML::Load(worldData);
    // First pass to get area names
    for (const auto& area : worldDataTree)
    {
        auto areaName = area["Name"].as<std::string>();
        // Construct Area object (struct) with just the name for now
        areaTable[areaName] = std::make_unique<Area>();
        areaTable[areaName]->name = areaName;
    }

    // Read and parse macros
    std::string macroListData;
    Utility::getFileContents(macrosFilePath, macroListData, true);
    YAML::Node macroListTree = YAML::Load(macroListData);
    auto err = loadMacros(macroListTree);
    if (err != World::WorldLoadingError::NONE)
    {
        ErrorLog::getInstance().log("Got error loading macros for world " + std::to_string(worldId) + ": " + errorToName(err));
        ErrorLog::getInstance().log(getLastErrorDetails());
        return 1;
    }

    // Read and parse location data
    std::string locationData;
    Utility::getFileContents(locationDataPath, locationData, true);
    YAML::Node locationDataTree = YAML::Load(locationData);
    for (const auto& locationObject : locationDataTree)
    {
        err = loadLocation(locationObject);
        if (err != World::WorldLoadingError::NONE)
        {
            ErrorLog::getInstance().log(std::string("Got error loading location: ") + errorToName(err));
            ErrorLog::getInstance().log(getLastErrorDetails());
            return 1;
        }
    }

    // Second pass of world graph to load each area's data
    for (const auto& area : worldDataTree)
    {
        err = loadArea(area);
        if (err != World::WorldLoadingError::NONE)
        {
            ErrorLog::getInstance().log("Got error loading area for world " + std::to_string(worldId) + ": " + errorToName(err));
            ErrorLog::getInstance().log(getLastErrorDetails());
            return 1;
        }
    }

    // Read and parse area translations for hints/spoiler logs in other languages
    std::string areaData;
    Utility::getFileContents(areaDataPath, areaData, true);
    YAML::Node areaDataTree = YAML::Load(areaData);
    for (const auto& areaObject : areaDataTree)
    {
        err = loadAreaTranslations(areaObject);
        if (err != World::WorldLoadingError::NONE)
        {
            ErrorLog::getInstance().log(std::string("Got error loading area translations: ") + errorToName(err));
            ErrorLog::getInstance().log(getLastErrorDetails());
            return 1;
        }
    }

    // Once all areas have been loaded, create the entrance lists. This lets us
    // find assigned islands/dungeons later
    for (auto& [name, area] : areaTable)
    {
        for (auto& exit : area->exits)
        {
            exit.connect(exit.getConnectedArea());
            exit.setOriginalName();

            // Set each dungeon's associated starting entrance
            auto connectedDungeon = exit.getConnectedArea()->dungeon;
            auto connectedArea = exit.getConnectedArea();
            if (area->dungeon == "" && connectedDungeon != "")
            {
                auto& dungeon = dungeons[connectedDungeon];
                if (dungeon.startingArea == connectedArea)
                {
                    dungeon.startingEntrance = &exit;
                }
            }
        }
    }

    // Load dungeon wind warp exit info
    err = loadDungeonExitInfo();
    if (err != World::WorldLoadingError::NONE)
    {
        ErrorLog::getInstance().log(std::string("Got error loading dungeon exit info: ") + errorToName(err));
        ErrorLog::getInstance().log(getLastErrorDetails());
        return 1;
    }

    return 0;
}

Entrance* World::getEntrance(const std::string& parentAreaName, const std::string& connectedAreaName)
{
    // sanity check that the areas exist
    if (!areaTable.contains(parentAreaName))
    {
        ErrorLog::getInstance().log("ERROR: \"" + parentAreaName + "\" is not a defined area!");
        return nullptr;
    }
    if (!areaTable.contains(connectedAreaName))
    {
        ErrorLog::getInstance().log("ERROR: \"" + connectedAreaName + "\" is not a defined area!");
        return nullptr;
    }

    auto parentArea = getArea(parentAreaName);
    auto connectedArea = getArea(connectedAreaName);

    return getEntrance(parentArea, connectedArea);
}

Entrance* World::getEntrance(Area* parentArea, Area* connectedArea)
{
    for (auto& exit : parentArea->exits)
    {
        if (exit.getOriginalConnectedArea() == connectedArea)
        {
            return &exit;
        }
    }

    ErrorLog::getInstance().log("ERROR: " + parentArea->name + " -> " + connectedArea->name + " is not a connection!");
    return nullptr;
}

void World::removeEntrance(Entrance* entranceToRemove)
{
    std::list<Entrance>& areaExits = getArea(entranceToRemove->getParentArea()->name)->exits;
    std::erase_if(areaExits, [entranceToRemove](const Entrance& entrance)
    {
        return &entrance == entranceToRemove;
    });
}

EntrancePool World::getShuffleableEntrances(const EntranceType& type, const bool& onlyPrimary /*= false*/)
{
    std::vector<Entrance*> shufflableEntrances = {};

    for (auto& [name, area] : areaTable)
    {
        for (auto& exit : area->exits)
        {
            if ((isAnyOf(type, exit.getEntranceType(), EntranceType::ALL)) && (!onlyPrimary || exit.isPrimary()) && exit.getEntranceType() != EntranceType::NONE)
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
    return filterFromPool(entrances, [](const Entrance* e){return e->isShuffled();});
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
    return Text::apply_name_color(str, color);
}

// Add an event with the passed in name to the event map
// if it doesn't already exist
void World::addEvent(const std::string& eventName)
{
    if (!eventMap.contains(eventName))
    {
        eventMap[eventName] = eventCounter;
        reverseEventMap[eventCounter] = eventName;
        eventCounter += 1;
    }
}

// Add a location with the passed in name to the location table
// if it doesn't already exist
void World::addLocation(const std::string& locationName)
{
    if (!locationTable.contains(locationName))
    {
        locationTable[locationName] = std::make_unique<Location>();
    }
}

Item World::getItem(const std::string& itemName)
{
    auto sanitizedName = gameItemToName(nameToGameItem(itemName));
    if (!itemTable.contains(sanitizedName))
    {
        ErrorLog::getInstance().log("ERROR: Item \"" + itemName + "\" is not defined for World " + std::to_string(worldId + 1));
        return itemTable["Nothing"];
    }
    return itemTable[sanitizedName];
}

// Perform a flattening search for this world.
// This will set a simplified single requirement statement
// for each location. This will then be used to calculate
// each item's chain locations as well as the set of items
// that could potentially be requiredto access any given location.
// This info is useful for calculating intuitive hint and ctmc data.
// TODO: It can also be used for faster filling in the future if desired
// although the filling is pretty fast currently so it's probably not
// necessary.
void World::flattenLogicRequirements()
{       
    // Run the flattening search. The search
    // will set the simplified requirement for
    // each location
    auto flatten = FlattenSearch(this);
    flatten.doSearch();

    // For each location, note down any item
    // that appears in it's simplified requirement
    // and store that as a potential item requirement
    // for this location.
    for (auto& [name, loc] : locationTable)
    {
        loc->itemsInComputedRequirement = loc->computedRequirement.getItems();

        // For each item listed, set this location as a chain
        // location of the item
        for (auto& gameItem : loc->itemsInComputedRequirement)
        {
            // Make an exception for hearts, which will only appear
            // in spiky chests if the player starts with less than 3
            if (getStartingHeartCount() >= 3 && (gameItem == GameItem::HeartContainer || gameItem == GameItem::PieceOfHeart))
            {
                continue;
            }
            itemTable[gameItemToName(gameItem)].addChainLocation(loc.get());
        }
    }

    // Properly set the chain locations for each item in the item pools as well
    // as any already placed items, and dungeon's associated items
    for (auto pool : {&itemPool, &startingItems})
    {
        for (auto& item : *pool)
        {
            for (auto loc : itemTable[gameItemToName(item.getGameItemId())].getChainLocations())
            {
                item.addChainLocation(loc);
            }
        }
    }
    for (auto& [name, loc] : locationTable)
    {
        auto& item = loc->currentItem;
        if (item.getGameItemId() != GameItem::INVALID)
        {
            for (auto loc : itemTable[gameItemToName(item.getGameItemId())].getChainLocations())
            {
                item.addChainLocation(loc);
            }
        }
    }
    for (auto& [name, dungeon] : dungeons)
    {
        if (dungeon.smallKey.getGameItemId() != GameItem::INVALID)
        {
            for (auto loc : itemTable[gameItemToName(dungeon.smallKey.getGameItemId())].getChainLocations())
            {
                dungeon.smallKey.addChainLocation(loc);
            }
        }
        if (dungeon.bigKey.getGameItemId() != GameItem::INVALID)
        {
            for (auto loc : itemTable[gameItemToName(dungeon.bigKey.getGameItemId())].getChainLocations())
            {
                dungeon.bigKey.addChainLocation(loc);
            }
        }
    }
}

std::string World::errorToName(WorldLoadingError err)
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
    case WorldLoadingError::DUNGEON_MISSING_KEY:
        return "DUNGEON_MISSING_KEY";
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
    case WorldLoadingError::BAD_REQUIREMENT:
        return "BAD_REQUIREMENT";
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

    for (auto& [name, area] : areaTable) {

        std::string color = area->isAccessible ? "\"black\"" : "\"red\"";

        auto& parentName = area->name;
        worldGraph << "\t\"" << parentName << "\"[shape=\"plain\" fontcolor=" << color << "];" << std::endl;

        // Make edge connections defined by exits
        for (const auto& exit : area->exits) {
            // Only dump shuffled exits if set
            if ((!exit.isShuffled() && onlyRandomizedExits) || !exit.getConnectedArea())
            {
                continue;
            }
            auto connectedName = exit.getConnectedArea()->name;
            if (parentName != "INVALID" && connectedName != "INVALID"){
                worldGraph << "\t\"" << parentName << "\" -> \"" << connectedName << "\"" << std::endl;
            }
        }

        // Make edge connections between areas and their locations
        for (const auto& locAccess : area->locations) {
            std::string connectedLocation = locAccess.location->getName();
            std::ranges::replace(connectedLocation, '&', 'A');
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
    Utility::platformLog("Dumped world graph at " + fullFilename);
}
