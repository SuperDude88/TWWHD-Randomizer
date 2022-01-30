
#include "Fill.hpp"
#include "Search.hpp"
#include "PoolFunctions.hpp"
#include "Random.hpp"
#include "Debug.hpp"

static void logItemsAndLocations(ItemPool& items, LocationPool& locations)
{
    debugLog("num items: " + std::to_string(items.size()) + " num locations: " + std::to_string(locations.size()));
    debugLog("Items:");
    for (auto& item : items)
    {
        debugLog("\t" + item.getName());
    }
    for (auto location : locations)
    {
        debugLog("\t" + locationIdToName(location->locationId) + " in world " + std::to_string(location->worldId));
    }
}

// Place the given items completely randomly among the given locations.
// The given locations are assumed to not be filled yet.
static FillError fastFill(ItemPool& items, LocationPool& locations)
{
    if (items.size() > locations.size())
    {
        logItemsAndLocations(items, locations);
        return FillError::MORE_ITEMS_THAN_LOCATIONS;
    }
    while (!items.empty() && !locations.empty())
    {
        auto item = popRandomElement(items);
        auto location = popRandomElement(locations);
        location->currentItem = item;
        debugLog("Placed " + item.getName() + " at " + locationIdToName(location->locationId) + " in world " + std::to_string(location->worldId));
    }

    return FillError::NONE;
}

// Place the given items within the given locations using the assumed fill algorithm.
// The given locations are assumed to not be filled yet
static FillError assumedFill(WorldPool& worlds, ItemPool& itemsToPlace, const ItemPool& itemsNotYetPlaced, LocationPool& allowedLocations)
{
    if (itemsToPlace.size() > allowedLocations.size())
    {
        logItemsAndLocations(itemsToPlace, allowedLocations);
        return FillError::MORE_ITEMS_THAN_LOCATIONS;
    }

    int retries = 10;
    bool unsuccessfulPlacement = false;
    do
    {
        retries--;
        if (retries <= 0)
        {
            return FillError::RAN_OUT_OF_RETRIES;
        }
        unsuccessfulPlacement = false;
        shufflePool(itemsToPlace);
        ItemPool rollbacks;

        while (!itemsToPlace.empty())
        {
            // Get a random item to place and add it to the rollbacks just in case
            auto item = popRandomElement(itemsToPlace);
            rollbacks.push_back(item);

            // Assume we have all items which haven't been placed yet
            // (except for the one we're about to place).
            ItemPool assumedItems (itemsNotYetPlaced);
            addElementsToPool(assumedItems, itemsToPlace);

            // Get a list of accessible locations
            const auto& accessibleLocations = getAccessibleLocations(worlds, assumedItems, allowedLocations);

            // If there aren't any accessible locations, rollback any previously
            // placed items in this group.
            if (accessibleLocations.empty())
            {
                // debugLog("No Accessible Locations to place " + item.getName());
                addElementsToPool(itemsToPlace, rollbacks);
                rollbacks.clear();
                // Break out of the item placement loop and flag an unsuccessful
                // placement attempt to try again.
                unsuccessfulPlacement = true;
                break;
            }

            // Place the item within one of the allowed locations
            auto location = RandomElement(accessibleLocations);
            location->currentItem = std::move(item);
            // debugLog("Placed " + item.getName() + " at " + locationIdToName(location->locationId) + " in world " + std::to_string(location->worldId));
        }
    }
    while(unsuccessfulPlacement);
    return FillError::NONE;
}

static void placeHardcodedItems(WorldPool& worlds)
{
    for (auto& world : worlds)
    {
        world.locationEntries[locationIdAsIndex(LocationId::DefeatGanondorf)].currentItem = {GameItem::GameBeatable, world.getWorldId()};
    }
}

FillError fill(WorldPool& worlds)
{
    std::cout << "Filling Worlds" << std::endl;
    placeHardcodedItems(worlds);

    FillError err;
    // Combine all worlds' item pools
    ItemPool itemPool;
    LocationPool allLocations;
    for (auto& world : worlds)
    {
        addElementsToPool(itemPool, world.getItemPool());
        addElementsToPool(allLocations, world.getLocations());
    }

    #ifdef ENABLE_DEBUG
        debugLog("All items:");
        for (auto& item : itemPool)
        {
            debugLog("\t" + item.getName());
        }
    #endif

    auto majorItems = filterAndEraseFromPool(itemPool, [](const Item& i){return i.isMajorItem();});

    #ifdef ENABLE_DEBUG
        debugLog("Major items:");
        for (auto& item : majorItems)
        {
            debugLog("\t" + item.getName());
            std::cout <<  << std::endl;
        }
        debugLog("End of Major items");
        debugLog();
    #endif

    // Place all major items in the Item Pool using assumed fill
    err = assumedFill(worlds, majorItems, itemPool, allLocations);
    if (err != FillError::NONE)
    {
        return err;
    }

    // Then fast fill for the rest
    err = fastFill(itemPool, allLocations);
    if (err != FillError::NONE)
    {
        return err;
    }

    if (!gameBeatable(worlds))
    {
        return FillError::GAME_NOT_BEATABLE;
    }

    return FillError::NONE;
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
    case FillError::GAME_NOT_BEATABLE:
        return "GAME_NOT_BEATABLE";
    default:
        return "UNKNOWN";
    }
}
