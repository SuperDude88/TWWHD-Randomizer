
#include "Search.hpp"
#include "PoolFunctions.hpp"
#include "Debug.hpp"
#include <list>
#include <algorithm>

static bool evaluateRequirement(const World& world, const Requirement& req, const ItemPool& ownedItems)
{
    uint32_t expectedCount = 0;
    GameItem gameItem;
    Item item;
    switch(req.type)
    {
    case RequirementType::OR:
        return std::any_of(
            req.args.begin(),
            req.args.end(),
            [&](const Requirement::Argument& arg){
                return evaluateRequirement(world, std::get<Requirement>(arg), ownedItems);
            }
        );
    case RequirementType::AND:
        return std::all_of(
            req.args.begin(),
            req.args.end(),
            [&](const Requirement::Argument& arg){
                if (arg.index() != 2)
                {
                    return false;
                }
                return evaluateRequirement(world, std::get<Requirement>(arg), ownedItems);
            }
        );
    case RequirementType::NOT:
        if (req.args[0].index() != 2)
        {
            return false;
        }
        return !evaluateRequirement(world, std::get<Requirement>(req.args[0]), ownedItems);
    case RequirementType::HAS_ITEM:
        // we can expect ownedItems will contain entires for every item type
        if (req.args[0].index() != 3)
        {
            return false;
        }
        gameItem = std::get<GameItem>(req.args[0]);
        if (gameItem == GameItem::NOTHING) return true;
        item = {gameItem, world.getWorldId()};
        return elementInPool(item, ownedItems);
    case RequirementType::COUNT:
        expectedCount = std::get<int>(req.args[0]);
        gameItem = std::get<GameItem>(req.args[1]);
        if (gameItem == GameItem::NOTHING) return true;
        item = {gameItem, world.getWorldId()};
        return elementCountInPool(item, ownedItems) >= expectedCount;
    case RequirementType::CAN_ACCESS:
        return world.areaEntries[areaAsIndex(std::get<Area>(req.args[0]))].isAccessible;
    case RequirementType::SETTING:
        // Settings are resolved to a true/false value when building the world
        return std::get<int>(req.args[0]);
    case RequirementType::MACRO:
        return evaluateRequirement(world, world.macros[std::get<MacroIndex>(req.args[0])], ownedItems);
    case RequirementType::NONE:
    default:
        // actually needs to be some error state?
        return false;
    }
    return false;
}

// Recursively explore new areas based on the given areaEntry
void explore(const SearchMode& searchMode, WorldPool& worlds, ItemPool& items, AreaEntry& areaEntry, std::list<Exit*>& exitsToTry, std::list<Location*>& locationsToTry)
{
    for (auto& exit : areaEntry.exits)
    {
        auto& connectedArea = worlds[exit.worldId].areaEntries[areaAsIndex(exit.connectedArea)];
        // If the connected area is already reachable, then the current exit
        // is ignored since it won't matter for logical access
        if (!connectedArea.isAccessible)
        {
            if (evaluateRequirement(worlds[exit.worldId], exit.requirement, items))
            {
                connectedArea.isAccessible = true;
                // std::cout << "Now Exploring " << areaToName(connectedArea.area) << std::endl;
                explore(searchMode, worlds, items, connectedArea, exitsToTry, locationsToTry);
            }
            else
            {
                // Push failed exits to the front so the searching loop doesn't
                // consider them until the next sphere of iteration
                exitsToTry.push_front(&exit);
            }
        }
    }
    for (auto location : areaEntry.locations)
    {
        // Add new locations we come across to try them and potentially account
        // for any items on the next iteration
        locationsToTry.push_back(location);
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
        addElementsToPool(items, world.getStartingItems());
    }

    LocationPool accessibleLocations = {};
    // Lists of exits and locations whose requirement returned false.
    // These will be tried and explored on the next iteration. Start
    // by putting the root exit of each world into the list of exits
    // to try (or only the exit of the single world to explore).
    std::list<Exit*> exitsToTry = {};
    std::list<Location*> locationsToTry = {};
    for (auto& world : worlds)
    {
        if (worldToSearch == -1 || worldToSearch == world.getWorldId())
        {
            for (auto& exit : world.areaEntries[areaAsIndex(Area::Root)].exits)
            {
                exitsToTry.push_back(&exit);
            }

            // Reset search variables for all areas and exits
            for (auto& areaEntry : world.areaEntries)
            {
                areaEntry.isAccessible = false;
            }
        }
    }

    // Variables for general searching
    bool newThingsFound = false;
    int sphere = 0;

    do
    {
        newThingsFound = false;
        // Search each exit in the exitsToTry list and explore any new areas found as well.
        // For any exits which we try and don't meet the requirements for, put them
        // into exitsToTry for the next iteration. Any locations we come across will
        // be added to locationsToTry.
        for (auto exitItr = exitsToTry.begin(); exitItr != exitsToTry.end(); exitItr++)
        {
            auto exit = *exitItr;
            if (evaluateRequirement(worlds[exit->worldId], exit->requirement, items)) {
                newThingsFound = true;
                // Erase the exit from the list of exits if we've met its requirement
                exitItr = exitsToTry.erase(exitItr);
                exitItr--;
                // If this exit's connected region has not been explored yet, then explore it
                auto& connectedArea = worlds[exit->worldId].areaEntries[areaAsIndex(exit->connectedArea)];
                if (!connectedArea.isAccessible)
                {
                    connectedArea.isAccessible = true;
                    explore(searchMode, worlds, items, connectedArea, exitsToTry, locationsToTry);
                }
            }
        }
        // Note which locations are now accessible on this iteration
        LocationPool accessibleThisIteration = {};
        // debugLog("New Locations Accessible:");
        for (auto locItr = locationsToTry.begin(); locItr != locationsToTry.end(); locItr++)
        {
            auto location = *locItr;
            if (evaluateRequirement(worlds[location->worldId], location->requirement, items))
            {
                // debugLog("\t" + locationIdToName(location->locationId) + " in world " + std::to_string(location->worldId));
                newThingsFound = true;
                // Delete newly accessible locations from the list
                accessibleThisIteration.push_back(location);
                locItr = locationsToTry.erase(locItr);
                locItr--;
            }
        }

        // push_back an empty sphere if we're generating the playthrough
        if (searchMode == SearchMode::GeneratePlaythrough)
        {
            worlds[0].playthroughSpheres.push_back({});
        }

        // Now apply any effects of newly accessible locations for the next iteration.
        // This lets us properly keep track of spheres for playthrough generation
        for (auto location : accessibleThisIteration)
        {
            accessibleLocations.push_back(location);
            if (location->currentItem.getGameItemId() != GameItem::INVALID && !location->currentItem.isJunkItem())
            {
                items.emplace_back(location->currentItem.getGameItemId(), location->currentItem.getWorldId());
                if (searchMode == SearchMode::GeneratePlaythrough && location->progression)
                {
                    worlds[0].playthroughSpheres[sphere].push_back(location);
                }
            }
        }
        sphere++;
    }
    while (newThingsFound);

    if (searchMode == SearchMode::GeneratePlaythrough)
    {
        std::sort(items.begin(), items.end());
        logItemPool("Items found in playthrough:", items);
    }

    return accessibleLocations;
}

LocationPool getAccessibleLocations(WorldPool& worlds, ItemPool& items, LocationPool& allowedLocations, int worldToSearch /*= -1*/)
{
    auto accessibleLocations = search(SearchMode::AccessibleLocations, worlds, items, worldToSearch);
    // Filter to only those locations which are allowed
    return filterFromPool(accessibleLocations, [allowedLocations](Location* loc){return elementInPool(loc, allowedLocations) && loc->currentItem.getGameItemId() == GameItem::INVALID;});
}

bool gameBeatable(WorldPool& worlds)
{
    ItemPool emptyItems = {};
    auto accessibleLocations = search(SearchMode::AccessibleLocations, worlds, emptyItems);
    auto worldsBeatable = filterFromPool(accessibleLocations, [](Location* loc){return loc->currentItem.getGameItemId() == GameItem::GameBeatable;});
    return worldsBeatable.size() == worlds.size();
}

// Whittle down the playthrough to only the items which are absolutely necessary
// for beating the game
static void pareDownPlaythrough(WorldPool& worlds)
{

    auto& playthroughSpheres = worlds[0].playthroughSpheres;

    for (size_t sphere = 0; sphere < playthroughSpheres.size(); sphere++)
    {
        for (auto loc = playthroughSpheres[sphere].begin(); loc != playthroughSpheres[sphere].end(); loc++)
        {
            // Remove the item at the current location and check if the game is still beatable
            auto location = *loc;
            auto itemAtLocation = location->currentItem;
            location->currentItem = {GameItem::INVALID, location->worldId};
            if (gameBeatable(worlds))
            {
                // If the game is still beatable, then this item is not required
                // and we can erase it from the playthrough
                loc = playthroughSpheres[sphere].erase(loc);
                loc--;
            }
            location->currentItem = itemAtLocation;
        }
    }

    // Get rid of any empty spheres
    filterAndEraseFromPool(playthroughSpheres, [](std::list<Location*> locPool){return locPool.empty();});
}

void generatePlaythrough(WorldPool& worlds)
{
    ItemPool emptyItems = {};
    search(SearchMode::GeneratePlaythrough, worlds, emptyItems);
    pareDownPlaythrough(worlds);
}

bool locationsReachable(WorldPool& worlds, ItemPool& items, LocationPool& locationsToCheck, int worldToSearch /*= -1*/)
{
    auto accessibleLocations = search(SearchMode::AccessibleLocations, worlds, items, worldToSearch);
    // Check if every location in locationsToCheck is in accessibleLocations
    return std::all_of(locationsToCheck.begin(), locationsToCheck.end(), [accessibleLocations, items](const Location* loc){
        bool inPool = elementInPool(loc, accessibleLocations);
        #ifdef ENABLE_DEBUG
            if (!inPool)
            {
                debugLog("Missing location " + locationName(loc));
            }
        #endif
        return inPool;
    });
}
