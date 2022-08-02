
#include "Search.hpp"
#include "PoolFunctions.hpp"
#include "../server/command/Log.hpp"
#include <list>
#include <unordered_set>
#include <algorithm>

using ItemMultiSet = std::unordered_multiset<Item>;
using EventSet = std::unordered_set<EventId>;

static bool evaluateRequirement(World& world, const Requirement& req, const ItemMultiSet& ownedItems, const EventSet& ownedEvents)
{
    uint32_t expectedCount = 0;
    Item item;
    EventId event;
    std::string eventName;
    switch(req.type)
    {
    case RequirementType::NOTHING:
        return true;

    case RequirementType::IMPOSSIBLE:
        return false;

    case RequirementType::OR:
        return std::any_of(
            req.args.begin(),
            req.args.end(),
            [&](const Requirement::Argument& arg){
                return evaluateRequirement(world, std::get<Requirement>(arg), ownedItems, ownedEvents);
            }
        );

    case RequirementType::AND:
        return std::all_of(
            req.args.begin(),
            req.args.end(),
            [&](const Requirement::Argument& arg){
                return evaluateRequirement(world, std::get<Requirement>(arg), ownedItems, ownedEvents);
            }
        );

    case RequirementType::NOT:
        return !evaluateRequirement(world, std::get<Requirement>(req.args[0]), ownedItems, ownedEvents);

    case RequirementType::HAS_ITEM:
        item = std::get<Item>(req.args[0]);
        return ownedItems.count(item) > 0;

    case RequirementType::EVENT:
        event = std::get<EventId>(req.args[0]);
        return ownedEvents.count(event) > 0;

    case RequirementType::COUNT:
        expectedCount = std::get<int>(req.args[0]);
        item = std::get<Item>(req.args[1]);
        return ownedItems.count(item) >= expectedCount;

    case RequirementType::CAN_ACCESS:
        return world.getArea(std::get<std::string>(req.args[0])).isAccessible;

    case RequirementType::SETTING:
        // Settings are resolved to a true/false value when building the world
        return std::get<int>(req.args[0]);

    case RequirementType::MACRO:
        return evaluateRequirement(world, world.macros[std::get<MacroIndex>(req.args[0])], ownedItems, ownedEvents);

    case RequirementType::NONE:
    default:
        // actually needs to be some error state?
        return false;
    }
    return false;
}



// Recursively explore new areas based on the given areaEntry
void explore(const SearchMode& searchMode, WorldPool& worlds, const ItemMultiSet& ownedItems, const EventSet& ownedEvents, AreaEntry& areaEntry, std::list<EventAccess*>& eventsToTry, std::list<Entrance*>& exitsToTry, std::list<LocationAccess*>& locationsToTry)
{
    for (auto& eventAccess : areaEntry.events)
    {
        eventsToTry.push_back(&eventAccess);
    }
    for (auto& exit : areaEntry.exits)
    {
        // If the exit is disconnected, then ignore it
        if (exit.getConnectedArea() == "")
        {
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
            if (!worlds[exit.getWorldId()].getSettings().decouple_entrances && exit.getReplaces()->getReverse() != nullptr)
            {
                for (auto& sphere : worlds[0].entranceSpheres)
                {
                    if (std::find(sphere.begin(), sphere.end(), exit.getReplaces()->getReverse()) != sphere.end())
                    {
                        reverseInPlaythrough = true;
                        break;
                    }
                }
            }

            if (!reverseInPlaythrough && evaluateRequirement(worlds[exit.getWorldId()], exit.getRequirement(), ownedItems, ownedEvents))
            {
                worlds[0].entranceSpheres.back().push_back(&exit);
            }
        }

        auto& connectedArea = worlds[exit.getWorldId()].getArea(exit.getConnectedArea());
        // If the connected area is already reachable, then the current exit
        // is ignored since it won't matter for logical access
        if (!connectedArea.isAccessible)
        {
            if (evaluateRequirement(worlds[exit.getWorldId()], exit.getRequirement(), ownedItems, ownedEvents))
            {
                connectedArea.isAccessible = true;
                explore(searchMode, worlds, ownedItems, ownedEvents, connectedArea, eventsToTry, exitsToTry, locationsToTry);
            }
            else
            {
                // Push failed exits to the front so the searching loop doesn't
                // consider them until the next sphere of iteration
                exitsToTry.push_front(&exit);
            }
        }
    }
    for (auto& locAccess : areaEntry.locations)
    {
        // Add new locations we come across to try them and potentially account
        // for any items on the next iteration.
        locationsToTry.push_back(&locAccess);
    }
}

// Argument 2 is a copy of the passed in ItemPool since we want to modify
// it locally. If worldToSearch is not -1 then only the world with that worldId
// will be searched.
static LocationPool search(const SearchMode& searchMode, WorldPool& worlds, ItemPool items, int worldToSearch = -1)
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
            for (auto& exit : world.getArea("Root").exits)
            {
                exitsToTry.push_back(&exit);
            }

            // Reset search variables for all areas and exits
            for (auto& [name, areaEntry] : world.areaEntries)
            {
                areaEntry.isAccessible = false;
            }

            for (auto& [name, location] : world.locationEntries)
            {
                location.hasBeenFound = false;
            }
        }
    }

    // Variables for general searching
    bool newThingsFound = false;
    int sphere = 0;

    do
    {

        // push_back an empty sphere if we're generating the playthroughs
        if (searchMode == SearchMode::GeneratePlaythrough)
        {
            worlds[0].playthroughSpheres.push_back({});
            worlds[0].entranceSpheres.push_back({});
        }

        // Variable to keep track of making logical progress. We want to keep
        // looping as long as we're finding new things on each iteration
        newThingsFound = false;
        // Loop through and see if there are any events that we are now accessible.
        // Add them to the ownedEvents list if they are.
        for (auto eventItr = eventsToTry.begin(); eventItr != eventsToTry.end(); )
        {
            auto eventAccess = *eventItr;
            if (ownedEvents.count(eventAccess->event) > 0)
            {
                eventItr = eventsToTry.erase(eventItr);
                continue;
            }
            if (evaluateRequirement(worlds[eventAccess->worldId], eventAccess->requirement, ownedItems, ownedEvents))
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
            if (evaluateRequirement(worlds[exit->getWorldId()], exit->getRequirement(), ownedItems, ownedEvents)) {

                // Erase the exit from the list of exits if we've met its requirement
                exitItr = exitsToTry.erase(exitItr);
                if (exit->getConnectedArea() == "")
                {
                    continue;
                }
                // If we're generating the playthrough, add it to the entranceSpheres if it's randomized
                if (searchMode == SearchMode::GeneratePlaythrough && exit->isShuffled())
                {
                    worlds[0].entranceSpheres.back().push_back(exit);
                }
                // If this exit's connected region has not been explored yet, then explore it
                auto& connectedArea = worlds[exit->getWorldId()].getArea(exit->getConnectedArea());
                if (!connectedArea.isAccessible)
                {
                    newThingsFound = true;
                    connectedArea.isAccessible = true;
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
            if (evaluateRequirement(worlds[location->worldId], locAccess->requirement, ownedItems, ownedEvents))
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
            if (location->currentItem.getGameItemId() != GameItem::INVALID && !location->currentItem.isJunkItem())
            {
                ownedItems.emplace(location->currentItem.getGameItemId(), location->currentItem.getWorldId());
                if (searchMode == SearchMode::GeneratePlaythrough && location->progression)
                {
                    worlds[0].playthroughSpheres.back().push_back(location);
                }
            }
        }
        sphere++;
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
    // Temporarily take these items away
    for (auto& world : worlds)
    {
        for (auto& [name, location] : world.locationEntries)
        {
            if (!location.progression)
            {
                nonRequiredLocations.insert({&location, location.currentItem});
                location.currentItem = {GameItem::INVALID, location.worldId};
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
            location->currentItem = {GameItem::INVALID, location->worldId};
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
    std::unordered_map<Entrance*, std::string> nonRequiredEntrances = {};
    for (auto entranceSphereItr = entranceSpheres.rbegin(); entranceSphereItr != entranceSpheres.rend(); entranceSphereItr++)
    {
        auto& entranceSphere = *entranceSphereItr;
        for (auto entranceItr = entranceSphere.begin(); entranceItr != entranceSphere.end(); )
        {
            auto entrance = *entranceItr;
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
    return std::all_of(locationsToCheck.begin(), locationsToCheck.end(), [accessibleLocations](const Location* loc){
        bool inPool = elementInPool(loc, accessibleLocations);
        #ifdef ENABLE_DEBUG
            if (!inPool)
            {
                LOG_TO_DEBUG("Missing location " + loc->name);
            }
        #endif
        return inPool;
    });
}

// Checks to see if ALL locations are accessible
bool allLocationsReachable(WorldPool& worlds, ItemPool& items, int worldToSearch /*= -1*/)
{
    size_t totalWorldsLocations = 0;
    for (auto& world : worlds)
    {
        totalWorldsLocations += world.locationEntries.size();
    }
    auto accessibleLocations = search(SearchMode::AllLocationsReachable, worlds, items, worldToSearch);
    return totalWorldsLocations == accessibleLocations.size();
}
