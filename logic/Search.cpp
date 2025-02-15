
#include "Search.hpp"

#include <list>
#include <ranges>
#include <unordered_set>
#include <algorithm>

#include <logic/PoolFunctions.hpp>
#include <command/Log.hpp>

// Recursively explore new areas based on the given areaEntry
void explore(const SearchMode& searchMode, WorldPool& worlds, const ItemMultiSet& ownedItems, const EventSet& ownedEvents, Area* area, std::list<EventAccess*>& eventsToTry, std::list<Entrance*>& exitsToTry, std::list<LocationAccess*>& locationsToTry)
{
    for (auto& eventAccess : area->events)
    {
        eventsToTry.push_back(&eventAccess);
    }
    for (auto& exit : area->exits)
    {
        // If the exit is disconnected, then ignore it
        if (exit.getConnectedArea() == nullptr)
        {
            // Evaluate the exit still for tracker purposes
            if (!exit.hasBeenFound() && evaluateRequirement(exit.getWorld(), exit.getRequirement(), &ownedItems, &ownedEvents))
            {
                exit.setFound(true);
            }
            continue;
        }
        // If we're generating the playthrough, evaluate the entrance requirement
        // if it's shuffled to potentially add it to the entrance spheres
        if (searchMode == SearchMode::GeneratePlaythrough && exit.isShuffled())
        {
            // If entrances are not decoupled we only want to add the first entrance
            // of a bound two way entrance to the entrance playthrough for
            // spoiler log simplicity
            bool reverseInPlaythrough = false;
            if (!exit.isDecoupled() && exit.getReplaces()->getReverse() != nullptr)
            {
                for (auto& sphere : worlds[0].entranceSpheres)
                {
                    if (std::ranges::find(sphere, exit.getReplaces()->getReverse()) != sphere.end())
                    {
                        reverseInPlaythrough = true;
                        break;
                    }
                }
            }

            if (!reverseInPlaythrough && evaluateRequirement(exit.getWorld(), exit.getRequirement(), &ownedItems, &ownedEvents))
            {
                worlds[0].entranceSpheres.back().push_back(&exit);
            }
        }

        auto connectedArea = exit.getConnectedArea();
        // If the connected area is already reachable, then the current exit
        // is ignored since it won't matter for logical access
        if (!connectedArea->isAccessible)
        {
            if (evaluateRequirement(exit.getWorld(), exit.getRequirement(), &ownedItems, &ownedEvents))
            {
                exit.setFound(true);
                connectedArea->isAccessible = true;
                explore(searchMode, worlds, ownedItems, ownedEvents, connectedArea, eventsToTry, exitsToTry, locationsToTry);
            }
            else
            {
                // Push failed exits to the front so the searching loop doesn't
                // consider them until the next sphere of iteration
                exitsToTry.push_front(&exit);
            }
        }
        // If this exit hasn't been found, but is now found, mark it as such (for the tracker)
        else if (!exit.hasBeenFound() && evaluateRequirement(exit.getWorld(), exit.getRequirement(), &ownedItems, &ownedEvents))
        {
            exit.setFound(true);
        }
    }
    for (auto& locAccess : area->locations)
    {
        // Add new locations we come across to try them and potentially account
        // for any items on the next iteration.
        locationsToTry.push_back(&locAccess);
    }
}

// Argument 2 is a copy of the passed in ItemPool since we want to modify
// it locally. If worldToSearch is not -1 then only the world with that worldId
// will be searched.
LocationPool search(const SearchMode& searchMode, WorldPool& worlds, ItemPool items, int worldToSearch /* = -1 */)
{
    // Add starting inventory items to the pool of items
    for (auto& world : worlds)
    {
        if (worldToSearch == -1 || worldToSearch == world.getWorldId())
        {
            addElementsToPool(items, world.getStartingItems());
        }
    }

    ItemMultiSet ownedItems (items.begin(), items.end());
    EventSet ownedEvents = {};

    LocationPool accessibleLocations = {};
    // Lists of events, exits and locations whose requirement returned false.
    // These will be tried and explored on the next iteration. Start
    // by putting the root exit of each world into the list of exits
    // to try (or only the exit of the single world to explore).
    std::list<EventAccess*> eventsToTry = {};
    std::list<Entrance*> exitsToTry = {};
    std::list<LocationAccess*> locationsToTry = {};
    for (auto& world : worlds)
    {
        if (worldToSearch == -1 || worldToSearch == world.getWorldId())
        {
            for (auto& exit : world.getArea("Root")->exits)
            {
                exitsToTry.push_back(&exit);
            }
        }

        // Reset search variables for all areas and exits
        for (auto& [name, area] : world.areaTable)
        {
            area->isAccessible = false;
            for (auto& exit : area->exits)
            {
                exit.setFound(false);
            }
        }

        for (auto& [name, location] : world.locationTable)
        {
            location->hasBeenFound = false;
        }
    }

    // Variables for general searching
    bool newThingsFound = false;

    do
    {

        // push_back an empty sphere if we're generating the playthroughs
        if (searchMode == SearchMode::GeneratePlaythrough)
        {
            worlds[0].playthroughSpheres.emplace_back();
            worlds[0].entranceSpheres.emplace_back();
        }

        // Variable to keep track of making logical progress. We want to keep
        // looping as long as we're finding new things on each iteration
        newThingsFound = false;
        // Loop through and see if there are any events that we are now accessible.
        // Add them to the ownedEvents list if they are.
        for (auto eventItr = eventsToTry.begin(); eventItr != eventsToTry.end(); )
        {
            auto eventAccess = *eventItr;
            if (ownedEvents.contains(eventAccess->event))
            {
                eventItr = eventsToTry.erase(eventItr);
                continue;
            }
            if (evaluateRequirement(eventAccess->world, eventAccess->requirement, &ownedItems, &ownedEvents))
            {
                newThingsFound = true;
                eventItr = eventsToTry.erase(eventItr);
                ownedEvents.insert(eventAccess->event);
            }
            else
            {
                eventItr++; // Only increment if we don't erase
            }
        }

        // Search each exit in the exitsToTry list and explore any new areas found as well.
        // For any exits which we try and don't meet the requirements for, put them
        // into exitsToTry for the next iteration. Any locations we come across will
        // be added to locationsToTry.
        for (auto exitItr = exitsToTry.begin(); exitItr != exitsToTry.end(); )
        {
            auto exit = *exitItr;
            if (evaluateRequirement(exit->getWorld(), exit->getRequirement(), &ownedItems, &ownedEvents)) {
                exit->setFound(true);
                // Erase the exit from the list of exits if we've met its requirement
                exitItr = exitsToTry.erase(exitItr);
                if (exit->getConnectedArea() == nullptr)
                {
                    continue;
                }
                // If we're generating the playthrough, add it to the entranceSpheres if it's randomized
                if (searchMode == SearchMode::GeneratePlaythrough && exit->isShuffled())
                {
                    worlds[0].entranceSpheres.back().push_back(exit);
                }
                // If this exit's connected region has not been explored yet, then explore it
                auto connectedArea = exit->getConnectedArea();
                if (!connectedArea->isAccessible)
                {
                    newThingsFound = true;
                    connectedArea->isAccessible = true;
                    explore(searchMode, worlds, ownedItems, ownedEvents, connectedArea, eventsToTry, exitsToTry, locationsToTry);
                }
            }
            else
            {
                exitItr++; // Only increment if we don't erase
            }
        }
        // Note which locations are now accessible on this iteration
        LocationPool accessibleThisIteration = {};
        for (auto locItr = locationsToTry.begin(); locItr != locationsToTry.end(); )
        {
            auto locAccess = *locItr;
            auto location = locAccess->location;
            // Erase locations which have already been found. Some item locations
            // can be obtained from multiple areas, so this check is necessary
            // in those circumstances
            if (location->hasBeenFound)
            {
                locItr = locationsToTry.erase(locItr);
                continue;
            }
            if (evaluateRequirement(location->world, locAccess->requirement, &ownedItems, &ownedEvents))
            {
                newThingsFound = true;
                location->hasBeenFound = true;
                // Delete newly accessible locations from the list
                accessibleThisIteration.push_back(location);
                locItr = locationsToTry.erase(locItr);
            }
            else
            {
                locItr++; // Only increment if we don't erase
            }
        }

        // Now apply any effects of newly accessible locations for the next iteration.
        // This lets us properly keep track of spheres for playthrough generation
        for (auto location : accessibleThisIteration)
        {
            accessibleLocations.push_back(location);
            auto& item = location->currentItem; 
            if (item.getGameItemId() != GameItem::INVALID && !item.isJunkItem())
            {
                ownedItems.emplace(item.getGameItemId(), item.getWorld());
                // Only add progression locations to the playthrough if they don't have known vanilla items
                // Also add in dungeon locations which have small/big keys if mixed bosses is on
                if (searchMode == SearchMode::GeneratePlaythrough && ((location->progression && (!location->hasKnownVanillaItem || item.getGameItemId() == GameItem::GameBeatable)) ||
                                                                      (location->world->getSettings().mix_bosses && (item.isBigKey() || item.isSmallKey()))))
                {
                    worlds[0].playthroughSpheres.back().push_back(location);
                }
            }
        }
    }
    while (newThingsFound);

    return accessibleLocations;
}

LocationPool getAccessibleLocations(WorldPool& worlds, ItemPool& items, LocationPool& allowedLocations, int worldToSearch /*= -1*/)
{
    auto accessibleLocations = search(SearchMode::AccessibleLocations, worlds, items, worldToSearch);
    // Filter to only those locations which are allowed
    return filterFromPool(accessibleLocations, [allowedLocations](Location* loc){return elementInPool(loc, allowedLocations) && loc->currentItem.getGameItemId() == GameItem::INVALID;});
}

void runGeneralSearch(WorldPool& worlds, int worldToSearch /*= -1*/)
{
    ItemPool emptyItems = {};
    search(SearchMode::AccessibleLocations, worlds, emptyItems, worldToSearch);
}

bool gameBeatable(WorldPool& worlds)
{
    ItemPool emptyItems = {};
    auto accessibleLocations = search(SearchMode::GameBeatable, worlds, emptyItems);
    auto worldsBeatable = filterFromPool(accessibleLocations, [](Location* loc){return loc->currentItem.getGameItemId() == GameItem::GameBeatable;});
    return worldsBeatable.size() == worlds.size();
}

// Whittle down the playthrough to only the items which are absolutely necessary
// for beating the game
static void pareDownPlaythrough(WorldPool& worlds)
{

    auto& playthroughSpheres = worlds[0].playthroughSpheres;
    auto& entranceSpheres = worlds[0].entranceSpheres;
    // Keep track of all locations we temporarily take items away from to give them back
    // after the playthrough calculation
    std::unordered_map<Location*, Item> nonRequiredLocations = {};
    // None of the items in non progress locations should be considered in the playthrough
    // Temporarily take these items away, unless they're small/big keys when boss entrances
    // are mixed
    for (auto& world : worlds)
    {
        for (auto& [name, location] : world.locationTable)
        {
            if (!location->progression && 
                !(world.getSettings().mix_bosses && (location->currentItem.isBigKey() || location->currentItem.isSmallKey())) && 
                !location->categories.contains(LocationCategory::BlueChuChu))
            {
                nonRequiredLocations.insert({location.get(), location->currentItem});
                location->currentItem = {GameItem::INVALID, &world};
            }
        }
    }

    for (auto& sphere : playthroughSpheres)
    {
        for (auto loc = sphere.begin(); loc != sphere.end(); )
        {
            // Remove the item at the current location and check if the game is still beatable
            auto location = *loc;
            auto itemAtLocation = location->currentItem;
            location->currentItem = {GameItem::INVALID, location->world};
            if (gameBeatable(worlds))
            {
                // If the game is still beatable, then this location is not required
                // and we can erase it from the playthrough
                loc = sphere.erase(loc);
                nonRequiredLocations.insert({location, itemAtLocation});
            }
            else
            {
                location->currentItem = itemAtLocation;
                loc++; // Only increment if we don't erase
            }
        }
    }

    // Now regenerate the playthrough with only the required locations incase
    // some spheres were flattened by non-required locations having progress items
    playthroughSpheres.clear();
    entranceSpheres.clear();
    ItemPool emptyItems = {};
    search(SearchMode::GeneratePlaythrough, worlds, emptyItems);

    // Give back nonrequired items
    for (auto& [location, item] : nonRequiredLocations)
    {
        location->currentItem = item;
    }

    // Now do the same process for the entrances to pare down the entrance playthrough
    std::unordered_map<Entrance*, Area*> nonRequiredEntrances = {};
    for (std::list<Entrance*>& entranceSphere : std::ranges::reverse_view(entranceSpheres))
    {
        for (auto entranceItr = entranceSphere.begin(); entranceItr != entranceSphere.end(); )
        {
            Entrance* entrance = *entranceItr;
            // Disconnect the entrance and then see if the world is still beatable
            auto connectedArea = entrance->disconnect();
            if (gameBeatable(worlds))
            {
                // If the game is still beatable, then this entrance is not required
                // and we can erase it from the playthrough
                entranceItr = entranceSphere.erase(entranceItr);
                nonRequiredEntrances[entrance] = connectedArea;
            }
            else
            {
                // If the entrance is required, then reconnect it
                entrance->connect(connectedArea);
                entranceItr++; // Only increment if we don't erase
            }
        }
    }

    for (auto& [entrance, area] : nonRequiredEntrances)
    {
        entrance->connect(area);
    }

    // Get rid of any empty spheres in both the item playthrough and entrance playthrough
    // based only on if the item playthrough has empty spheres. Both the playthroughs
    // will have the same number of spheres, so we only need to conditionally
    // check one of them.
    auto itemItr = playthroughSpheres.begin();
    auto entranceItr = entranceSpheres.begin();
    while (itemItr != playthroughSpheres.end())
    {
        if (itemItr->empty() && entranceItr->empty())
        {
            itemItr = playthroughSpheres.erase(itemItr);
            entranceItr = entranceSpheres.erase(entranceItr);
        }
        else
        {
            itemItr++;     // Only incremement if we don't erase
            entranceItr++;
        }
    }
}

void generatePlaythrough(WorldPool& worlds)
{
    ItemPool emptyItems = {};
    search(SearchMode::GeneratePlaythrough, worlds, emptyItems);
    LOG_TO_DEBUG("Pare Down");
    pareDownPlaythrough(worlds);
}

// Checks to see if the specific locations from the passed in location pool are all accessible
bool locationsReachable(WorldPool& worlds, ItemPool& items, LocationPool& locationsToCheck, int worldToSearch /*= -1*/)
{
    auto accessibleLocations = search(SearchMode::AccessibleLocations, worlds, items, worldToSearch);
    // Check if every location in locationsToCheck is in accessibleLocations
    return std::ranges::all_of(locationsToCheck, [&](const Location* loc){
        bool inPool = elementInPool(loc, accessibleLocations);
        if (!inPool)
        {
            LOG_TO_DEBUG("Missing location " + loc->getName());
        }
        return inPool;
    });
}

// Checks to see if ALL locations are accessible
bool allLocationsReachable(WorldPool& worlds, ItemPool& items, int worldToSearch /*= -1*/)
{
    size_t totalWorldsLocations = 0;
    for (auto& world : worlds)
    {
        totalWorldsLocations += world.locationTable.size();
    }
    auto accessibleLocations = search(SearchMode::AllLocationsReachable, worlds, items, worldToSearch);
    return totalWorldsLocations == accessibleLocations.size();
}
