
#include "Fill.hpp"
#include "Search.hpp"
#include "PoolFunctions.hpp"
#include "Dungeon.hpp"
#include "../seedgen/random.hpp"
#include "../server/command/Log.hpp"
#include "../server/utility/platform.hpp"
#include <chrono>

#define FILL_ERROR_CHECK(err) if (err != FillError::NONE) {return err;}

static void logItemsAndLocations(ItemPool& items, LocationPool& locations)
{
    DebugLog::getInstance().log("num items: " + std::to_string(items.size()) + " num locations: " + std::to_string(locations.size()));
    DebugLog::getInstance().log("Items:");
    for (auto& item : items)
    {
        DebugLog::getInstance().log("\t" + item.getName());
    }
    for (auto location : locations)
    {
        DebugLog::getInstance().log("\t" + locationIdToName(location->locationId) + " in world " + std::to_string(location->worldId));
    }
}

#define ENOUGH_SPACE_CHECK(items, locations) if (items.size() > locations.size()) {logItemsAndLocations(items, locations);return FillError::MORE_ITEMS_THAN_LOCATIONS;}

// Place the given items completely randomly among the given locations.
static FillError fastFill(ItemPool& items, LocationPool& locations)
{

    // Get rid of locations which already have items
    filterAndEraseFromPool(locations, [](const Location* loc){return loc->currentItem.getGameItemId() != GameItem::INVALID;});
    ENOUGH_SPACE_CHECK(items, locations);

    while (!items.empty() && !locations.empty())
    {
        auto item = popRandomElement(items);
        auto location = popRandomElement(locations);
        location->currentItem = item;
        #ifdef ENABLE_DEBUG
            DebugLog::getInstance().log("Placed " + item.getName() + " at " + locationIdToName(location->locationId) + " in world " + std::to_string(location->worldId + 1));
        #endif
    }

    return FillError::NONE;
}

static FillError fillTheRest(ItemPool& items, LocationPool& locations)
{
    FillError err;
    // First place the junk already in the pool
    err = fastFill(items, locations);
    FILL_ERROR_CHECK(err);

    // When the item pool is empty, get random junk for the remaining locations
    for (auto location : locations)
    {
        if (location->currentItem.getGameItemId() == GameItem::INVALID)
        {
            location->currentItem = Item(getRandomJunk(), location->worldId);
        }
    }
    return FillError::NONE;
}

// Sometimes assumed fill will fail a lot if there's only one or two initial locations open.
// In this case we'll switch to forward fill for some time until there are more free places to
// place items. The 3rd parameter allowedLocations is a copy of the passed in LocationPool since
// we want to locally modify it in this function, but not outside of it
static FillError forwardFillUntilMoreFreeSpace(WorldPool& worlds, ItemPool& itemsToPlace, LocationPool allowedLocations, int openLocations = 2)
{
    ENOUGH_SPACE_CHECK(itemsToPlace, allowedLocations);

    ItemPool forwardPlacedItems;
    auto accessibleLocations = getAccessibleLocations(worlds, forwardPlacedItems, allowedLocations);

    if (accessibleLocations.empty())
    {
        #ifdef ENABLE_DEBUG
            DebugLog::getInstance().log("No reachable locations during forward fill attempt");
        #endif
        return FillError::NO_REACHABLE_LOCATIONS;
    }

    while (accessibleLocations.size() < openLocations * worlds.size())
    {
        #ifdef ENABLE_DEBUG
            DebugLog::getInstance().log("Not enough locations available. Need: " + std::to_string(openLocations * worlds.size()) + " Have: " + std::to_string(accessibleLocations.size()));
        #endif
        // Filter out already accessible locations
        filterAndEraseFromPool(allowedLocations, [accessibleLocations](const Location* loc){return elementInPool(loc, accessibleLocations);});
        shufflePool(itemsToPlace);

        auto sizeBefore = forwardPlacedItems.size();
        for (auto& item : itemsToPlace)
        {

            forwardPlacedItems.push_back(item);

            if (getAccessibleLocations(worlds, forwardPlacedItems, allowedLocations).size() > 0)
            {

                auto location = RandomElement(accessibleLocations);
                #ifdef ENABLE_DEBUG
                    DebugLog::getInstance().log("Item " + item.getName() + " opened up more space");
                    DebugLog::getInstance().log("Placing item at " + locationName(location));
                #endif
                location->currentItem = item;
                break;
            }
            else
            {
                forwardPlacedItems.pop_back();
            }
        }
        // If no new items were placed, then we can't progress
        if (forwardPlacedItems.size() == sizeBefore)
        {
            #ifdef ENABLE_DEBUG
                DebugLog::getInstance().log("No items opened up progression during forward fill attempt");
            #endif
            return FillError::RAN_OUT_OF_RETRIES;
        }

        accessibleLocations = getAccessibleLocations(worlds, forwardPlacedItems, allowedLocations);
    }

    // Remove items placed during forward fill from the item pool
    for (auto& item : forwardPlacedItems)
    {
        removeElementFromPool(itemsToPlace, item, 1);
    }

    return FillError::NONE;
}

// Place the given items within the given locations using the assumed fill algorithm. If a world to fill is specified
// then only that world's graph will be explored to find accessible locations
static FillError assumedFill(WorldPool& worlds, ItemPool& itemsToPlace, const ItemPool& itemsNotYetPlaced, LocationPool& allowedLocations, int worldToFill = -1)
{
    ENOUGH_SPACE_CHECK(itemsToPlace, allowedLocations);

    int retries = 5;
    bool unsuccessfulPlacement = false;
    do
    {
        if (retries <= 0)
        {
            #ifdef ENABLE_DEBUG
                DebugLog::getInstance().log("Ran out of retries, attempting forward fill as last resort");
            #endif
            if (forwardFillUntilMoreFreeSpace(worlds, itemsToPlace, allowedLocations) != FillError::NONE)
            {
                return FillError::RAN_OUT_OF_RETRIES;
            }
            else
            {
                retries = 5;
                continue;
            }
        }
        retries--;
        unsuccessfulPlacement = false;
        shufflePool(itemsToPlace);
        LocationPool rollbacks;

        while (!itemsToPlace.empty())
        {
            // Get a random item to place
            auto item = popRandomElement(itemsToPlace);

            // Assume we have all items which haven't been placed yet
            // (except for the one we're about to place).
            ItemPool assumedItems (itemsNotYetPlaced);
            addElementsToPool(assumedItems, itemsToPlace);

            // Get a list of accessible locations
            auto accessibleLocations = getAccessibleLocations(worlds, assumedItems, allowedLocations, worldToFill);

            // If there aren't any accessible locations, rollback any previously
            // used locations in this group and add their items back to the
            // itemsToPlace vector
            if (accessibleLocations.empty())
            {
                #ifdef ENABLE_DEBUG
                    DebugLog::getInstance().log("No Accessible Locations to place " + item.getName() + ". Retrying " + std::to_string(retries) + " more times.");
                #endif
                for (auto location : rollbacks)
                {
                    #ifdef ENABLE_DEBUG
                        DebugLog::getInstance().log("Rolling back " + locationIdToName(location->locationId) + ": " + location->currentItem.getName());
                    #endif
                    itemsToPlace.push_back(location->currentItem);
                    location->currentItem = Item(GameItem::INVALID, -1);
                }
                // Also add back the randomly selected item
                itemsToPlace.push_back(item);
                rollbacks.clear();
                // Break out of the item placement loop and flag an unsuccessful
                // placement attempt to try again.
                unsuccessfulPlacement = true;
                break;
            }

            // Attempt to not place treasure and triforce charts at sunken treasure locations
            // to avoid chart chains. Only allow it if sunken treasure locations
            // are all that are accessible.
            if (item.isChartForSunkenTreasure())
            {
                auto sunkenTreasureLocations = filterAndEraseFromPool(accessibleLocations, [](const Location* loc){return loc->categories.count(LocationCategory::SunkenTreasure) > 0;});
                if (accessibleLocations.empty())
                {
                    accessibleLocations = std::move(sunkenTreasureLocations);
                }
            }

            // Place the item within one of the allowed locations and add the
            // location to the list of rollbacks
            auto location = RandomElement(accessibleLocations);
            location->currentItem = std::move(item);
            rollbacks.push_back(location);
            #ifdef ENABLE_DEBUG
            DebugLog::getInstance().log("Placed " + item.getName() + " at " + locationIdToName(location->locationId));
        #endif
        }

    }
    while(unsuccessfulPlacement);
    return FillError::NONE;
}

static void placeHardcodedItems(WorldPool& worlds)
{
    for (auto& world : worlds)
    {
        // Place the game beatable item at Ganondorf
        world.locationEntries[locationIdAsIndex(LocationId::DefeatGanondorf)].currentItem = {GameItem::GameBeatable, world.getWorldId()};
    }
}

// Determine which items are major items. A major item is any item required
// for access to any progression location and/or game beatability. This function
// is called multiple times during the fill algorithm as the items required for
// beatability may change depending on which items are placed in race mode locations
// and/or plandomized
void determineMajorItems(WorldPool& worlds, ItemPool& itemPool, LocationPool& allLocations)
{
    #ifdef ENABLE_DEBUG
        DebugLog::getInstance().log("Determining Major Items");
        DebugLog::getInstance().log("New Major Items: [");
    #endif
    auto progressionLocations = filterFromPool(allLocations, [](const Location* location){return location->progression;});
    shufflePool(itemPool);
    for (auto& item : itemPool)
    {
        // Don't check junk items
        if (!item.isJunkItem())
        {
            // Temporarily take this item out of the pool
            auto gameItemId = item.getGameItemId();
            item.setGameItemId(GameItem::NOTHING);

            auto thisWorldsProgressionLocations = filterFromPool(progressionLocations, [item](const Location* location){return location->worldId == item.getWorldId();});

            // If all progress locations in the item's world are not reachable,
            // set it as a major item and give back it's gameitemId
            if (!locationsReachable(worlds, itemPool, thisWorldsProgressionLocations, item.getWorldId()))
            {
                item.setAsMajorItem();
                item.setGameItemId(gameItemId);
                #ifdef ENABLE_DEBUG
                DebugLog::getInstance().log("\t" + item.getName());
            #endif
            }
            // Otherwise save the gameItemId to re-apply later once all major items
            // have been determined
            else
            {
                item.setDelayedItemId(gameItemId);
            }
        }
    }
    #ifdef ENABLE_DEBUG
        DebugLog::getInstance().log("]");
    #endif
    // Re-apply all items which had delayed items set
    for (auto& item : itemPool)
    {
        if (item.getGameItemId() == GameItem::NOTHING)
        {
            item.saveDelayedItemId();
        }
    }
}

static void handleDungeonItems(WorldPool& worlds, ItemPool& itemPool)
{
    // For each world, either add the dungeon items to the main item pool
    // or place them within their dungeon if the world has keylunacy disabled
    for (auto& world : worlds)
    {
        auto worldId = world.getWorldId();

        for (auto& dungeonId : getDungeonList())
        {
            auto dungeon = dungeonIdToDungeon(dungeonId);

            // If keylunacy is disabled, then add dungeon items to the world's
            // item pool
            if (!world.getSettings().keylunacy)
            {
                // Filter to only the dungeons locations
                auto worldLocations = world.getLocations();
                auto dungeonLocations = filterFromPool(worldLocations, [dungeon](const Location* loc){return elementInPool(loc->locationId, dungeon.locations);});
                // Filter out tingle chests if they're not progression locations
                if (!world.getSettings().progression_tingle_chests)
                {
                    filterAndEraseFromPool(dungeonLocations, [](const Location* loc){return elementInPool(LocationCategory::TingleChest, loc->categories);});
                }
                // Filter out obscure locations if they're not progression locations
                if (!world.getSettings().progression_obscure)
                {
                    filterAndEraseFromPool(dungeonLocations, [](const Location* loc){return elementInPool(LocationCategory::Obscure, loc->categories);});
                }

                // Place small keys and the big key using only items and locations
                // from this world
                ItemPool dungeonPool;
                if (dungeon.smallKey != GameItem::INVALID && dungeon.bigKey != GameItem::INVALID)
                {
                    // Remove the boss key and small keys from the itemPool
                    removeElementFromPool(itemPool, Item(dungeon.smallKey, worldId), dungeon.keyCount);
                    removeElementFromPool(itemPool, Item(dungeon.bigKey, worldId));

                    for (int i = 0; i < dungeon.keyCount; i++)
                    {
                        dungeonPool.emplace_back(dungeon.smallKey, worldId);
                    }
                    dungeonPool.emplace_back(dungeon.bigKey, worldId);
                    assumedFill(worlds, dungeonPool, itemPool, dungeonLocations, worldId);
                }

                // Place maps and compasses after since they aren't progressive items
                dungeonPool.clear();
                dungeonPool.emplace_back(dungeon.map, worldId);
                dungeonPool.emplace_back(dungeon.compass, worldId);
                fastFill(dungeonPool, dungeonLocations);
                removeElementFromPool(itemPool, Item(dungeon.map, worldId));
                removeElementFromPool(itemPool, Item(dungeon.compass, worldId));
            }
        }
    }
}

static void generateRaceModeItems(LocationPool& raceModeLocations, ItemPool& raceModeItems, ItemPool& itemsToChooseFrom, ItemPool& mainItemPool)
{
    shufflePool(itemsToChooseFrom);
    while (!itemsToChooseFrom.empty() && raceModeItems.size() < raceModeLocations.size())
    {
        raceModeItems.push_back(popRandomElement(itemsToChooseFrom));
    }
    // Add back any unused elements
    addElementsToPool(mainItemPool, itemsToChooseFrom);
}

// Place progression items in specific locations at the end of dungeons to require the player
// to beat those dungeons.
static FillError placeRaceModeItems(WorldPool& worlds, ItemPool& itemPool, LocationPool& allLocations)
{
    LocationPool raceModeLocations;
    ItemPool raceModeItems;
    for (auto& world : worlds)
    {
        for (auto& [dungeonId, hintRegion] : world.raceModeDungeons)
        {
            const auto& dungeon = dungeonIdToDungeon(dungeonId);
            // The race mode location for each dungeon is the last one listed
            Location* raceModeLocation = &world.locationEntries[locationIdAsIndex(dungeon.locations.back())];
            // If this location already has an item placed at it, then skip it
            if (raceModeLocation->currentItem.getGameItemId() != GameItem::INVALID)
            {
                continue;
            }
            raceModeLocations.push_back(raceModeLocation);
        }
    }

    // Build up the list of race mode items starting with triforce shards...
    auto triforceShards = filterAndEraseFromPool(itemPool, [](const Item& item){return item.getGameItemId() >= GameItem::TriforceShard1 && item.getGameItemId() <= GameItem::TriforceShard8;});
    generateRaceModeItems(raceModeLocations, raceModeItems,  triforceShards, itemPool);

    // Then swords...
    auto swords = filterAndEraseFromPool(itemPool, [](const Item& item){return item.getGameItemId() == GameItem::ProgressiveSword;});
    generateRaceModeItems(raceModeLocations, raceModeItems, swords, itemPool);

    // Then bows...
    auto bows = filterAndEraseFromPool(itemPool, [](const Item& item){return item.getGameItemId() == GameItem::ProgressiveBow;});
    generateRaceModeItems(raceModeLocations, raceModeItems, bows, itemPool);

    // Then the rest of the major items if necessary.
    auto majorItems = filterAndEraseFromPool(itemPool, [](const Item& item){return item.isMajorItem();});
    generateRaceModeItems(raceModeLocations, raceModeItems, majorItems, itemPool);

    // logItemPool("raceModeItems", raceModeItems);

    if (raceModeItems.size() < raceModeLocations.size())
    {
        Utility::platformLog("WARNING: Not enough major items to place at race mode locations.");
    }

    // Then place the items in the race mode locations
    return assumedFill(worlds, raceModeItems, itemPool, raceModeLocations);
}

static FillError placePlandomizerItems(WorldPool& worlds, ItemPool& itemPool)
{
    #ifdef ENABLE_DEBUG
        DebugLog::getInstance().log("Placing Plandomizer Items");
    #endif
    std::unordered_map<Location*, Item> allPlandoLocations = {};
    for (auto& world : worlds)
    {
        allPlandoLocations.insert(world.plandomizerLocations.begin(), world.plandomizerLocations.end());
    }
    for (auto& [location, item] : allPlandoLocations)
    {
        if (!item.isJunkItem())
        {
            item = removeElementFromPool(itemPool, item);
            // Don't accept trying to place major items in non-progress locations
            if (item.isMajorItem() && !location->progression)
            {
                ErrorLog::getInstance().log("Error: Attempted to plandomize major item " + item.getName() + " in non-progress location " + locationName(location));
                ErrorLog::getInstance().log("Plandomizing major items in non-progress locations is not allowed.");
                return FillError::PLANDOMIZER_ERROR;
            }
        }
        location->currentItem = item;
    }
    return FillError::NONE;
}

FillError fill(WorldPool& worlds)
{
    // Time how long the fill takes
    #ifdef ENABLE_TIMING
        auto start = std::chrono::high_resolution_clock::now();
    #endif
    FillError err;
    ItemPool itemPool;
    LocationPool allLocations;

    placeHardcodedItems(worlds);

    // Combine all worlds' item pools and location pools
    for (auto& world : worlds)
    {
        addElementsToPool(itemPool, world.getItemPool());
        addElementsToPool(allLocations, world.getLocations());
    }

    determineMajorItems(worlds, itemPool, allLocations);
    err = placePlandomizerItems(worlds, itemPool);
    FILL_ERROR_CHECK(err);
    // Handle dungeon items and race mode dungeons first if necessary. Generally
    // we need to place items that go into more restrictive location pools first before
    // we can place other items.
    placeRaceModeItems(worlds, itemPool, allLocations);
    handleDungeonItems(worlds, itemPool);

    // Recalculate major items since new items may now be required depending on
    // what items were placed at plandomizer and race mode locations
    determineMajorItems(worlds, itemPool, allLocations);

    auto majorItems = filterAndEraseFromPool(itemPool, [](const Item& i){return i.isMajorItem();});
    auto progressionLocations = filterFromPool(allLocations, [](const Location* loc){return loc->progression && loc->locationId != LocationId::DefeatGanondorf && loc->currentItem.getGameItemId() == GameItem::INVALID;});

    if (majorItems.size() > progressionLocations.size())
    {
        Utility::platformLog(std::string("Major Items: ") + std::to_string(majorItems.size()));
        Utility::platformLog(std::string("Available Progression Locations: ") + std::to_string(progressionLocations.size()));
        Utility::platformLog("Please enable more spots for major items.");

        #ifdef ENABLE_DEBUG
            logItemPool("Major Items:", majorItems);

            DebugLog::getInstance().log("Progression Locations:");
            for (auto location : progressionLocations)
            {
                DebugLog::getInstance().log("\t" + locationName(location));
            }
        #endif
        return FillError::NOT_ENOUGH_PROGRESSION_LOCATIONS;
    }

    // Place all major items in the Item Pool using assumed fill.
    // Don't assume we have any non-major items.
    ItemPool noAssumedItems = {};
    err = assumedFill(worlds, majorItems, noAssumedItems, progressionLocations);
    FILL_ERROR_CHECK(err);

    // Then place the rest of the non-major progression items using assumed fill.
    auto remainingProgressionItems = filterAndEraseFromPool(itemPool, [](const Item& i){return !i.isJunkItem();});

    // TODO: It should be possible to speed this next fill up by adding back the major items
    // to the item pool resulting in less searching iterations, but I'm not confident
    // it's logically sound so I'll research it later.
    err = assumedFill(worlds, remainingProgressionItems, itemPool, allLocations);
    FILL_ERROR_CHECK(err);

    // Fill the remaining locations with junk
    err = fillTheRest(itemPool, allLocations);
    FILL_ERROR_CHECK(err);

    if (!gameBeatable(worlds))
    {
        return FillError::GAME_NOT_BEATABLE;
    }

    // Calculate time difference
    #ifdef ENABLE_TIMING
        auto stop = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
        auto seconds = static_cast<double>(duration.count()) / 1000000.0;
        Utility::platformLog(std::string("Fill took ") + std::to_string(seconds) + " seconds\n");
    #endif

    return FillError::NONE;
}

void clearWorlds(WorldPool& worlds)
{
    for (auto& world : worlds)
    {
        for (auto& location : world.locationEntries)
        {
            location.currentItem = {GameItem::INVALID, -1};
        }
    }
}

const char* errorToName(FillError err)
{
    switch(err)
    {
    case FillError::NONE:
        return "NONE";
    case FillError::RAN_OUT_OF_RETRIES:
        return "RAN_OUT_OF_RETRIES";
    case FillError::MORE_ITEMS_THAN_LOCATIONS:
        return "MORE_ITEMS_THAN_LOCATIONS";
    case FillError::NO_REACHABLE_LOCATIONS:
        return "NO_REACHABLE_LOCATIONS";
    case FillError::GAME_NOT_BEATABLE:
        return "GAME_NOT_BEATABLE";
    case FillError::NOT_ENOUGH_PROGRESSION_LOCATIONS:
        return "NOT_ENOUGH_PROGRESSION_LOCATIONS";
    case FillError::PLANDOMIZER_ERROR:
        return "PLANDOMIZER_ERROR";
    default:
        return "UNKNOWN";
    }
}
