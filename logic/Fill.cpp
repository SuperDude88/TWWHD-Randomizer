#include "Fill.hpp"

#include <chrono>
#include <numeric>

#include <logic/Search.hpp>
#include <logic/PoolFunctions.hpp>
#include <logic/Dungeon.hpp>
#include <seedgen/random.hpp>
#include <command/Log.hpp>
#include <utility/platform.hpp>
#include <utility/time.hpp>

#define FILL_ERROR_CHECK(func) err = func; if (err != FillError::NONE) {return err;}

static void logItemsAndLocations(ItemPool& items, LocationPool& locations)
{
    #ifdef ENABLE_DEBUG
        LOG_TO_DEBUG("num items: " + std::to_string(items.size()) + " num locations: " + std::to_string(locations.size()));
        LOG_TO_DEBUG("Items:");
        for (auto& item : items)
        {
            LOG_TO_DEBUG("\t" + item.getName());
        }
        LOG_TO_DEBUG("Locations:");
        for (auto location : locations)
        {
            LOG_TO_DEBUG("\t" + location->getName() + " in world " + std::to_string(location->world->getWorldId()));
        }
    #endif
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
        LOG_TO_DEBUG("Placed " + item.getName() + " at " + location->getName() + " in world " + std::to_string(location->world->getWorldId() + 1));
    }

    return FillError::NONE;
}

static FillError fillTheRest(WorldPool& worlds, ItemPool& items, LocationPool& locations)
{
    FillError err;
    // First place the non consumable junk already in the pool
    // Filter out the consumable junk to place afterwards
    auto consumableJunk = filterAndEraseFromPool(items, [](const Item& i){return i.isConsumableJunkItem();});
    FILL_ERROR_CHECK(fastFill(items, locations));

    // For the remaining locations, get items from the consumable junk. If the consumable junk runs out, just get more random
    // consumable junk
    for (auto location : locations)
    {
        if (location->currentItem.getGameItemId() == GameItem::INVALID)
        {
            Item item;
            if (!consumableJunk.empty())
            {
                item = popRandomElement(consumableJunk);
            }
            else
            {
                item = location->world->getItem(getRandomJunk());
            }
            location->currentItem = item;
            LOG_TO_DEBUG("Placed " + item.getName() + " at " + location->getName() + " in world " + std::to_string(location->world->getWorldId() + 1));
        }
    }
    return FillError::NONE;
}

// Sometimes assumed fill will fail a lot if there's only one or two initial locations open.
// In this case we'll switch to forward fill for some time until there are more free places to
// place items. The 3rd parameter allowedLocations is a copy of the passed in LocationPool since
// we want to locally modify it in this function, but not outside of it
FillError forwardFillUntilMoreFreeSpace(WorldPool& worlds, ItemPool& itemsToPlace, LocationPool allowedLocations, size_t openLocations /*= 3*/)
{
    ItemPool forwardPlacedItems;
    ItemPool noItems;
    auto accessibleLocations = getAccessibleLocations(worlds, forwardPlacedItems, allowedLocations);

    if (accessibleLocations.empty())
    {
        LOG_TO_DEBUG("No reachable locations during forward fill attempt");
        return FillError::NO_REACHABLE_LOCATIONS;
    }

    if (accessibleLocations.size() >= openLocations)
    {
        return FillError::NONE;
    }

    bool successfullyPlacedItems = false;
    LOG_TO_DEBUG("Number of open locations: " + std::to_string(accessibleLocations.size()));
    while (accessibleLocations.size() < openLocations * worlds.size() || !successfullyPlacedItems)
    {
        successfullyPlacedItems = false;
        #if ENABLE_DEBUG
            for (const Location* loc : accessibleLocations)
            {
                LOG_TO_DEBUG("  " + loc->getName());
            }
        #endif
        // Filter out already accessible locations
        filterAndEraseFromPool(allowedLocations, [accessibleLocations](const Location* loc){return elementInPool(loc, accessibleLocations);});
        shufflePool(itemsToPlace);

        auto sizeBefore = forwardPlacedItems.size();

        // The idea here is to try every combination of 1..n items where n is the number of available
        // places to place items. First try all items individually, then every set of 2, then 3, etc.
        // until we find a combination of items that opens up more space
        for (size_t itemsInSet = 1; itemsInSet <= accessibleLocations.size(); itemsInSet++)
        {
            std::vector<size_t> indices (itemsInSet, 0);
            std::iota(indices.begin(), indices.end(), 0);

            do
            {
                // increment the indices, starting with an attempt at the last one
                // and moving back to increment previous indices if necessary.
                for (size_t i = indices.size() - 1; i >= 0; i--) // loop is infinite due to uints
                {
                    size_t max = itemsToPlace.size() - (indices.size() - i);

                    if (indices[i] < max)
                    {
                        size_t counter = 1;
                        for (; i < indices.size(); i++)
                        {
                            indices[i] += counter++;
                        }
                        break;
                    }
                    else
                    {
                        indices[i] = indices[i-1];
                    }
                }

                ItemPool newForwardItems = {};
                LOG_TO_DEBUG("Trying forward items: ");
                for (auto& index : indices)
                {
                    newForwardItems.push_back(itemsToPlace[index]);
                    LOG_TO_DEBUG(std::string("  ") + itemsToPlace[index].getName());
                }


                if (!getAccessibleLocations(worlds, newForwardItems, allowedLocations).empty())
                {
                    addElementsToPool(forwardPlacedItems, newForwardItems);
                    fastFill(newForwardItems, accessibleLocations);
                    successfullyPlacedItems = true;
                    break;
                }

            } while (indices[0] < itemsToPlace.size() - itemsInSet);

            if (successfullyPlacedItems)
            {
                break;
            }
        }

        // If no new items were placed, then we can't progress
        if (forwardPlacedItems.size() == sizeBefore)
        {
            LOG_TO_DEBUG("No item combinations opened up progression during forward fill attempt");
            return FillError::RAN_OUT_OF_RETRIES;
        }
        sizeBefore = forwardPlacedItems.size();
        accessibleLocations = getAccessibleLocations(worlds, noItems, allowedLocations);
        LOG_TO_DEBUG("Number of open locations: " + std::to_string(accessibleLocations.size()));
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
            LOG_TO_DEBUG("Ran out of retries, attempting forward fill as last resort");
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
                LOG_TO_DEBUG("No Accessible Locations to place " + item.getName() + ". Retrying " + std::to_string(retries) + " more times.");
                for (auto location : rollbacks)
                {
                    LOG_TO_DEBUG("Rolling back " + location->getName() + ": " + location->currentItem.getName());
                    itemsToPlace.push_back(location->currentItem);
                    location->currentItem = Item(GameItem::INVALID, nullptr);
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
                auto sunkenTreasureLocations = filterAndEraseFromPool(accessibleLocations, [](const Location* loc){return loc->categories.contains(LocationCategory::SunkenTreasure);});
                if (accessibleLocations.empty())
                {
                    accessibleLocations = std::move(sunkenTreasureLocations);
                }
            }

            // Place the item within one of the allowed locations and add the
            // location to the list of rollbacks
            auto location = RandomElement(accessibleLocations);
            location->currentItem = item;
            rollbacks.push_back(location);
            LOG_TO_DEBUG("Placed " + item.getName() + " at " + location->getName());
        }

    }
    while(unsuccessfulPlacement);
    return FillError::NONE;
}

void placeVanillaItems(WorldPool& worlds)
{
    for (auto& world : worlds)
    {
        auto& settings = world.getSettings();

        // Place the game beatable item at Ganondorf
        world.locationTable["Ganon's Tower - Defeat Ganondorf"]->currentItem = world.getItem("Game Beatable");
        world.locationTable["Ganon's Tower - Defeat Ganondorf"]->hasKnownVanillaItem = true;

        // Place vanilla items depending on settings and remove placed vanilla items from the item pool
        for (auto location : world.getLocations())
        {
            auto vanillaItem = location->originalItem.getName();
            auto locationName = location->getName();
            std::string dungeonItemName = locationName.substr(0, locationName.find('-')) + vanillaItem;

            if ((settings.dungeon_small_keys     == PlacementOption::Vanilla &&  vanillaItem == "Small Key")  ||
                (settings.dungeon_big_keys       == PlacementOption::Vanilla &&  vanillaItem == "Big Key")    ||
                (settings.dungeon_maps_compasses == PlacementOption::Vanilla && (vanillaItem == "Dungeon Map" || vanillaItem == "Compass")))
                {
                    location->currentItem = world.getItem(dungeonItemName);
                    location->hasKnownVanillaItem = true;
                    removeElementFromPool(world.getItemPoolReference(), location->currentItem);
                    LOG_TO_DEBUG("Placed item " + dungeonItemName + " at vanilla location " + locationName);
                }

            if (vanillaItem == "Blue Chu Jelly" /*&& settings.progression_blue_chus*/)
            {
                location->currentItem = world.getItem(vanillaItem);
                location->hasKnownVanillaItem = true;
                removeElementFromPool(world.getItemPoolReference(), location->currentItem);
                LOG_TO_DEBUG("Placed item " + vanillaItem + " at vanilla location " + locationName);
            }
        }
    }
}

// Determine which items are major items. A major item is any item required
// for access to any progression location and/or game beatability. This function
// is called multiple times during the fill algorithm as the items required for
// beatability may change depending on which items are placed in race mode locations
// and/or plandomized
void determineMajorItems(WorldPool& worlds, ItemPool& itemPool, LocationPool& allLocations)
{
    LOG_TO_DEBUG("Determining Major Items");
    LOG_TO_DEBUG("New Major Items: [");
    auto progressionLocations = filterFromPool(allLocations, [](const Location* location){return location->progression;});

    // Combine the item pool as well as all items that have already been placed to test
    std::vector<Item*> totalItemPool = {};
    for (auto& item : itemPool)
    {
        totalItemPool.push_back(&item);
    }
    for (auto& world : worlds)
    {
        for (auto& location : world.getLocations(true))
        {
            if (location->currentItem.getGameItemId() != GameItem::INVALID)
            {
                totalItemPool.push_back(&location->currentItem);
            }
        }
    }

    shufflePool(totalItemPool);
    for (auto item : totalItemPool)
    {
        // Don't check junk items
        if (!item->isJunkItem())
        {
            // Temporarily take this item out of the pool
            auto gameItemId = item->getGameItemId();
            item->setGameItemId(GameItem::NOTHING);

            // If all progress locations are not reachable,
            // set it as a major item and give back it's gameitemId
            
            if (!locationsReachable(worlds, itemPool, progressionLocations))
            {
                item->setAsMajorItem();
                item->setGameItemId(gameItemId);
                LOG_TO_DEBUG("\t" + item->getName());
            }
            // Otherwise save the gameItemId to re-apply later once all major items
            // have been determined
            else
            {
                item->setDelayedItemId(gameItemId);
            }
        }
    }
    LOG_TO_DEBUG("]");
    // Re-apply all items which had delayed items set
    for (auto item : totalItemPool)
    {
        if (item->getGameItemId() == GameItem::NOTHING)
        {
            item->saveDelayedItemId();
        }
    }
}

// Randomize any approrpiate dungeon items into their own dungeons
static FillError randomizeOwnDungeon(WorldPool& worlds, ItemPool& itemPool)
{
    FillError err;
    for (auto& world : worlds)
    {
        auto& settings = world.getSettings();
        // Continue if none of these settings are enabled
        if (settings.dungeon_small_keys     != PlacementOption::OwnDungeon &&
            settings.dungeon_big_keys       != PlacementOption::OwnDungeon &&
            settings.dungeon_maps_compasses != PlacementOption::OwnDungeon)
            {
                continue;
            }
        auto worldId = world.getWorldId();
        for (auto& [name, dungeon] : world.dungeons)
        {
            // Filter to only the dungeons locations which are progression locations
            // If dungeons are not progression locations, or if race mode is on
            // and this isn't a race mode dungeon, then take all locations in
            // the dungeon since none of them are progression anyway
            auto worldLocations = world.getLocations();
            auto dungeonLocations = filterFromPool(worldLocations, [&dungeon = dungeon, &settings = settings](const Location* loc){
                return elementInPool(loc, dungeon.locations) &&
                          (loc->progression || settings.progression_dungeons == ProgressionDungeons::Disabled ||
                          (settings.progression_dungeons == ProgressionDungeons::RaceMode && !dungeon.isRequiredDungeon));});

            // Place small keys and the big key using only items and locations
            // from this world (even in multiworld)
            ItemPool dungeonPool;
            if (settings.dungeon_small_keys == PlacementOption::OwnDungeon && dungeon.smallKey.isValidItem())
            {
                auto smallKey = dungeon.smallKey;
                for (size_t i = 0; i < elementCountInPool(smallKey, itemPool); i++)
                {
                    dungeonPool.emplace_back(smallKey);
                }
                removeElementFromPool(itemPool, smallKey, dungeon.keyCount);
            }
            if (settings.dungeon_big_keys == PlacementOption::OwnDungeon && dungeon.bigKey.isValidItem())
            {
                auto bigKey = dungeon.bigKey;
                for (size_t i = 0; i < elementCountInPool(bigKey, itemPool); i++)
                {
                    dungeonPool.emplace_back(bigKey);
                }
                removeElementFromPool(itemPool, bigKey);
            }
            // Place small keys and big keys first since they're progression items
            FILL_ERROR_CHECK(assumedFill(worlds, dungeonPool, itemPool, dungeonLocations, worldId));

            // Set any locations which got the keys as having expected items
            for (auto loc : dungeonLocations)
            {
                if ((dungeon.smallKey.isValidItem() && loc->currentItem == dungeon.smallKey && settings.dungeon_small_keys == PlacementOption::OwnDungeon) || 
                    (dungeon.bigKey.isValidItem() && loc->currentItem == dungeon.bigKey && settings.dungeon_big_keys == PlacementOption::OwnDungeon)) 
                {
                    loc->hasExpectedItem = true;
                }
            }

            // Place maps and compasses after since they aren't progressive items
            dungeonPool.clear();
            if (settings.dungeon_maps_compasses == PlacementOption::OwnDungeon)
            {
                auto map = dungeon.map;
                auto compass = dungeon.compass;
                for (size_t i = 0; i < elementCountInPool(map, itemPool); i++)
                {
                    dungeonPool.emplace_back(map);
                }
                for (size_t i = 0; i < elementCountInPool(compass, itemPool); i++)
                {
                    dungeonPool.emplace_back(compass);
                }
                removeElementFromPool(itemPool, map);
                removeElementFromPool(itemPool, compass);
                FILL_ERROR_CHECK(fastFill(dungeonPool, dungeonLocations));
            }
        }
    }

    return FillError::NONE;
}

// Randomize any appropriate dungeon items into any dungeon or overworld
static FillError randomizeRestrictedDungeonItems(WorldPool& worlds, ItemPool& itemPool)
{
    FillError err;
    for (auto& world : worlds)
    {
        auto& settings = world.getSettings();
        auto worldId = world.getWorldId();

        // Continue if none of these settings are enabled
        if (settings.dungeon_small_keys     != PlacementOption::AnyDungeon && settings.dungeon_small_keys     != PlacementOption::Overworld &&
            settings.dungeon_big_keys       != PlacementOption::AnyDungeon && settings.dungeon_big_keys       != PlacementOption::Overworld &&
            settings.dungeon_maps_compasses != PlacementOption::AnyDungeon && settings.dungeon_maps_compasses != PlacementOption::Overworld)
            {
                continue;
            }

        // Create all necessary location pools. We need two pools for each setting to
        // separate progress locations in each pool from the entire pool.
        auto worldLocations = world.getLocations();
        LocationPool anyDungeonLocations = filterFromPool(worldLocations, [](const Location* location){
            return location->categories.contains(LocationCategory::Dungeon);});
        LocationPool anyDungeonProgressionLocations = filterFromPool(anyDungeonLocations, [](const Location* location){
            return location->progression;});
        LocationPool overworldLocations = filterFromPool(worldLocations, [](const Location* location){
            return !location->categories.contains(LocationCategory::Dungeon);});
        LocationPool overworldProgressionLocations = filterFromPool(overworldLocations, [](const Location* location){
            return location->progression;});;

        ItemPool anyDungeonItems;
        ItemPool anyDungeonProgressionItems;
        ItemPool overworldItems;
        ItemPool overworldProgressionItems;

        // Separate Maps and Compasses since they aren't progressive
        ItemPool anyDungeonMapCompassItems;
        ItemPool overworldMapCompassItems;

        // First gather small keys and big keys into their necessary pools
        for (auto& [name, dungeon] : world.dungeons)
        {
            auto smallKey = dungeon.smallKey;
            auto bigKey   = dungeon.bigKey;
            auto map      = dungeon.map;
            auto compass  = dungeon.compass;

            auto smallKeys     = filterFromPool(itemPool, [&](const Item& item){return item == smallKey;});
            auto bigKeys       = filterFromPool(itemPool, [&](const Item& item){return item == bigKey;});
            auto mapsCompasses = filterFromPool(itemPool, [&](const Item& item){return item == map || item == compass;});

            bool addItemsToProgressionPool = settings.progression_dungeons == ProgressionDungeons::Standard ||
                                            (settings.progression_dungeons == ProgressionDungeons::RaceMode && dungeon.isRequiredDungeon);

            if (settings.dungeon_small_keys == PlacementOption::AnyDungeon)
            {
                addElementsToPool((addItemsToProgressionPool ? anyDungeonProgressionItems : anyDungeonItems), smallKeys);
            }
            else if (settings.dungeon_small_keys == PlacementOption::Overworld)
            {
                addElementsToPool((addItemsToProgressionPool ? overworldProgressionItems : overworldItems), smallKeys);
            }

            if (settings.dungeon_big_keys == PlacementOption::AnyDungeon)
            {
                addElementsToPool((addItemsToProgressionPool ? anyDungeonProgressionItems : anyDungeonItems), bigKeys);
            }
            else if (settings.dungeon_big_keys == PlacementOption::Overworld)
            {
                addElementsToPool((addItemsToProgressionPool ? overworldProgressionItems : overworldItems), bigKeys);
            }

            if (settings.dungeon_maps_compasses == PlacementOption::AnyDungeon)
            {
                addElementsToPool(anyDungeonMapCompassItems, mapsCompasses);
            }
            else if (settings.dungeon_maps_compasses == PlacementOption::Overworld)
            {
                addElementsToPool(overworldMapCompassItems, mapsCompasses);
            }
        }

        // Then place the progression dungeon items within their locations
        removeElementsFromPool(itemPool, anyDungeonProgressionItems);
        FILL_ERROR_CHECK(assumedFill(worlds, anyDungeonProgressionItems, itemPool, anyDungeonProgressionLocations, worldId));
        removeElementsFromPool(itemPool, overworldProgressionItems);
        FILL_ERROR_CHECK(assumedFill(worlds, overworldProgressionItems, itemPool, overworldProgressionLocations, worldId));

        // Then place non-required progression dungeon items within any available locations
        removeElementsFromPool(itemPool, anyDungeonItems);
        FILL_ERROR_CHECK(assumedFill(worlds, anyDungeonItems, itemPool, anyDungeonLocations, worldId));
        removeElementsFromPool(itemPool, overworldItems);
        FILL_ERROR_CHECK(assumedFill(worlds, overworldItems, itemPool, overworldLocations, worldId));

        // Then place maps and compasses randomly within their pools
        removeElementsFromPool(itemPool, anyDungeonMapCompassItems);
        FILL_ERROR_CHECK(fastFill(anyDungeonMapCompassItems, anyDungeonLocations));
        removeElementsFromPool(itemPool, overworldMapCompassItems);
        FILL_ERROR_CHECK(fastFill(overworldMapCompassItems, overworldLocations));
    }

    return FillError::NONE;
}

static FillError handleDungeonItems(WorldPool& worlds, ItemPool& itemPool)
{
    // For each world, place dungeon items starting with the most restrictive settings
    // first. Start with the Own Dungeon setting
    randomizeOwnDungeon(worlds, itemPool);

    // Now do the any dungeon and overworld settings
    randomizeRestrictedDungeonItems(worlds, itemPool);

    return FillError::NONE;
}

static FillError placeNonProgressLocationPlandomizerItems(WorldPool& worlds, ItemPool& itemPool)
{
    LOG_TO_DEBUG("Placing Non-Progress Plandomizer Items");
    std::unordered_map<Location*, Item> allPlandoLocations = {};
    for (auto& world : worlds)
    {
        allPlandoLocations.insert(world.plandomizer.locations.begin(), world.plandomizer.locations.end());
    }
    for (auto& [location, item] : allPlandoLocations)
    {
        // Items in progression locations were placed earlier, so skip them now
        if (location->progression)
        {
            continue;
        }
        if (!item.isJunkItem())
        {
            item = removeElementFromPool(itemPool, item);
            // Don't accept trying to place major items in non-progress locations
            if (item.isMajorItem())
            {
                ErrorLog::getInstance().log("Attempted to plandomize major item \"" + gameItemToName(item.getGameItemId()) + "\" in non-progress location \"" + location->getName() + "\"");
                ErrorLog::getInstance().log("Plandomizing major items in non-progress locations is not allowed.");
                return FillError::PLANDOMIZER_ERROR;
            }
        }
        location->currentItem = item;
        LOG_TO_DEBUG("Placed " + item.getName() + " at " + location->getName());
    }
    return FillError::NONE;
}

FillError validateEnoughLocations(WorldPool& worlds)
{
    ItemPool majorItems;
    LocationPool progressionLocations;
    GET_COMPLETE_ITEM_POOL(majorItems, worlds);
    GET_COMPLETE_PROGRESSION_LOCATION_POOL(progressionLocations, worlds);
    determineMajorItems(worlds, majorItems, progressionLocations);
    majorItems = filterFromPool(majorItems, [](const Item& item){return item.isMajorItem();});

    if (majorItems.size() > progressionLocations.size())
    {
        ErrorLog::getInstance().clearLastErrors();
        ErrorLog::getInstance().log("Major Items: " + std::to_string(majorItems.size()));
        ErrorLog::getInstance().log("Available Progression Locations: " + std::to_string(progressionLocations.size()));
        ErrorLog::getInstance().log("Please select more places for progress items to appear.");

        #ifdef ENABLE_DEBUG
            logItemPool("Major Items", majorItems);

            LOG_TO_DEBUG("Progression Locations:");
            for (auto location : progressionLocations)
            {
                LOG_TO_DEBUG("\t" + location->getName());
            }
        #endif
        return FillError::NOT_ENOUGH_PROGRESSION_LOCATIONS;
    }

    return FillError::NONE;
}

FillError fill(WorldPool& worlds)
{
    // Time how long the fill takes
    #ifdef ENABLE_TIMING
        ScopedTimer<"Fill took ", std::chrono::milliseconds> timer;
    #endif
    FillError err;
    ItemPool itemPool;
    LocationPool allLocations;

    // Combine all worlds' item pools and location pools
    for (auto& world : worlds)
    {
        addElementsToPool(itemPool, world.getItemPool());
        addElementsToPool(allLocations, world.getLocations());
    }

    // Filter out hint locations from allLocations
    filterAndEraseFromPool(allLocations, [](const Location* loc){return loc->categories.contains(LocationCategory::HoHoHint);});

    determineMajorItems(worlds, itemPool, allLocations);
    FILL_ERROR_CHECK(placeNonProgressLocationPlandomizerItems(worlds, itemPool));
    // Handle dungeon items first if necessary. Generally
    // we need to place items that go into more restrictive location pools first before
    // we can place other items.
    FILL_ERROR_CHECK(handleDungeonItems(worlds, itemPool));

    // Recalculate major items again since new items may now be required depending on
    // what items were placed when handling dungeon items
    determineMajorItems(worlds, itemPool, allLocations);

    auto majorItems = filterAndEraseFromPool(itemPool, [](const Item& i){return i.isMajorItem();});
    auto progressionLocations = filterFromPool(allLocations, [](const Location* loc){return loc->progression && loc->currentItem.getGameItemId() == GameItem::INVALID;});

    FILL_ERROR_CHECK(validateEnoughLocations(worlds));

    // Place all major items in the Item Pool using assumed fill.
    // Don't assume we have any non-major items.
    ItemPool noAssumedItems = {};
    FILL_ERROR_CHECK(assumedFill(worlds, majorItems, noAssumedItems, progressionLocations));

    // Then place the rest of the non-major progression items using assumed fill.
    auto remainingProgressionItems = filterAndEraseFromPool(itemPool, [](const Item& i){return !i.isJunkItem();});

    // TODO: It should be possible to speed this next fill up by adding back the major items
    // to the item pool resulting in less searching iterations, but I'm not confident
    // it's logically sound so I'll research it later.
    FILL_ERROR_CHECK(assumedFill(worlds, remainingProgressionItems, itemPool, allLocations));

    // Fill the remaining locations with junk
    FILL_ERROR_CHECK(fillTheRest(worlds, itemPool, allLocations));

    if (!gameBeatable(worlds))
    {
        LOG_ERR_AND_RETURN(FillError::GAME_NOT_BEATABLE);
    }

    return FillError::NONE;
}

void clearWorlds(WorldPool& worlds)
{
    for (auto& world : worlds)
    {
        for (auto& [name, location] : world.locationTable)
        {
            if (!location->plandomized && !location->hasKnownVanillaItem)
            {
                location->currentItem = {GameItem::INVALID, nullptr};
                location->hasExpectedItem = false;
            }
        }
    }
}

std::string errorToName(FillError err)
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
