
#include "Fill.hpp"
#include "Search.hpp"
#include "PoolFunctions.hpp"
#include "Random.hpp"

static void printLocation(LocationEntry* loc)
{
    std::cout << "\t" << locationToName(loc->locationId) << " in world " << std::to_string(loc->worldId) << std::endl;
}

static void printItemAtLocation(LocationEntry* loc)
{
    std::cout << loc->currentItem.getName() << " place at " << locationToName(loc->locationId) << " in world " << std::to_string(loc->worldId) << std::endl;
}

// Place the given items completely randomly among the given locations
// The given locations are assumed to not be filled yet
static FillError fastFill(ItemPool& items, LocationPool& locations)
{
    if (items.size() > locations.size())
    {
        #ifdef ENABLE_DEBUG
            std::cout << "num items: " << std::to_string(items.size()) << " num locations: " << std::to_string(locations.size()) << std::endl;
        #endif
        return FillError::MORE_ITEMS_THAN_LOCATIONS;
    }
    while (!items.empty() && !locations.empty())
    {
        auto item = popRandomElement(items);
        auto location = popRandomElement(locations);
        location->currentItem = item;
        #ifdef ENABLE_DEBUG
            std::cout << "Placed " << item.getName() << " at ";
            printLocation(location);
        #endif
    }

    return FillError::NONE;
}

// Place the given items within the given locations using the assumed fill algorithm.
// The given locations are assumed to not be filled yet
static FillError assumedFill(WorldPool& worlds, ItemPool& itemsToPlace, const ItemPool& itemsNotYetPlaced, LocationPool& allowedLocations)
{

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
            AddElementsToPool(assumedItems, itemsToPlace);

            const auto& accessibleLocations = getAccessibleLocations(worlds, assumedItems, allowedLocations);

            // If there aren't any accessible locations, rollback any previously
            // placed items in this group.
            if (accessibleLocations.empty())
            {
                std::cout << "No Accessible Locations" << std::endl;
                AddElementsToPool(itemsToPlace, rollbacks);
                rollbacks.clear();
                // Break out of the item placement loop and flag an unsuccessful
                // placement attempt to try again.
                unsuccessfulPlacement = true;
                break;
            }

            // Place the item within one of the allowed locations
            auto location = RandomElement(accessibleLocations);
            location->currentItem = std::move(item);
            printItemAtLocation(location);
        }
    }
    while(unsuccessfulPlacement);
    return FillError::NONE;
}

FillError fill(std::vector<World>& worlds)
{
    FillError err;
    // Combine all worlds' item pools
    ItemPool itemPool;
    LocationPool allLocations;
    for (auto& world : worlds)
    {
        AddElementsToPool(itemPool, world.getItemPool());
        AddElementsToPool(allLocations, world.getLocations());
    }

    std::cout << "All items:" << std::endl;
    for (auto& item : itemPool)
    {
        std::cout << "\t" << item.getName() << std::endl;
    }

    auto majorItems = filterFromPool(itemPool, [](const Item& i){return i.isMajorItem();});
    std::cout << std::to_string(majorItems.size()) << std::endl;

    std::cout << "Major items:" << std::endl;
    for (auto& item : majorItems)
    {
        std::cout << "Item";
        std::cout << "\t" << item.getName() << std::endl;
    }
    std::cout << "End of Major items:" << std::endl << std::endl;

    // Place all major items in the Item Pool using assumed fill
    err = assumedFill(worlds, majorItems, itemPool, allLocations);
    if (err != FillError::NONE)
    {
        return err;
    }

    // Then fast fill for the rest


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
    default:
        return "UNKNOWN";
    }
}
