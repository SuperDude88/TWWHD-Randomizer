
#include "Search.hpp"
#include "PoolFunctions.hpp"
#include <list>

void printExit(Exit* exit)
{
    std::cout << areaToName(exit->parentArea) << " -> " << areaToName(exit->connectedArea) << " in world " << std::to_string(exit->worldId) << std::endl;
}


bool evaluateRequirement(const World& world, const Requirement& req, const ItemPool& ownedItems)
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
        if (gameItem == GameItem::Nothing) return true;
        item = {gameItem, world.getWorldId()};
        return ElementInPool(item, ownedItems);
    case RequirementType::COUNT:
        expectedCount = std::get<int>(req.args[0]);
        gameItem = std::get<GameItem>(req.args[1]);
        if (gameItem == GameItem::Nothing) return true;
        item = {gameItem, world.getWorldId()};
        return ElementCountInPool(item, ownedItems) >= expectedCount;
    case RequirementType::CAN_ACCESS:
        return evaluateRequirement(world, world.locationEntries[locationAsIndex(std::get<Location>(req.args[0]))].requirement, ownedItems);
    case RequirementType::SETTING:
        // TODO: assuming all boolean settings for now
        return world.getSettings().count(std::get<Setting>(req.args[0])) > 0;
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
void explore(WorldPool& worlds, ItemPool& items, AreaEntry& areaEntry, std::list<Exit*>& exitsToTry, std::list<LocationEntry*>& locationsToTry)
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
                //std::cout << "Now Exploring ";
                //printExit(&exit);
                explore(worlds, items, connectedArea, exitsToTry, locationsToTry);
            }
            else
            {
                // Push failed exits to the front so the searching loop doesn't
                // consider them until the next sphere of iteration
                exitsToTry.push_front(&exit);
            }
        }
        else
        {
            // std::cout << areaToName(connectedArea.area) << " is already accessible. ";
            // printExit(&exit);
            // std::cout << "\twill be discarded" << std::endl;
        }
    }
    for (auto location : areaEntry.locations)
    {
        // Add new locations we come across to try them and potentially account
        // for any items on the next iteration
        // std::cout << "Location for consideration " << locationToName(location->locationId) << " in world " << std::to_string(location->worldId) << std::endl;
        locationsToTry.push_back(location);
    }
}

// Argument 1 is a copy of the passed in ItemPool since we want to modify
// it locally
static LocationPool search(ItemPool items, WorldPool& worlds)
{
    // Add starting inventory items to the pool of items
    for (auto& world : worlds)
    {
        AddElementsToPool(items, world.getStartingItems());
    }

    // std::cout << "Searching with the following items:" << std::endl;
    // for (auto item : items)
    // {
    //     std::cout << "\t" << gameItemToName(item.getGameItemId()) << " for world " << std::to_string(item.getWorldId()) << std::endl;
    // }

    LocationPool accessibleLocations = {};
    // Lists of exits and locations whose requirement returned false
    // These will be tried and explored on the next iteration. Start
    // by putting the root exits into the list of exits to try
    std::list<Exit*> exitsToTry = {};
    std::list<LocationEntry*> locationsToTry = {};
    for (auto& world : worlds)
    {
        for (auto& exit : world.areaEntries[areaAsIndex(Area::Root)].exits)
        {
            exitsToTry.push_back(&exit);
        }

        // Reset search variables for all areas and exits
        for (auto& areaEntry : world.areaEntries)
        {
            areaEntry.isAccessible = false;
            for (auto& exit : areaEntry.exits)
            {
                exit.hasBeenTried = false;
            }
        }
    }

    int itrCount = 0;
    bool newThingsFound = false;
    do
    {
        // std::cout << "ITERATION " << std::to_string(++itrCount) << std::endl;
        newThingsFound = false;
        // Search each exit in the exitsToTry list and explore any new areas found as well.
        // For any exits which we try and don't meet the requirements for, put them
        // into the exits to try for next time. Any locations we come across will
        // be added to locationsToTry.
        for (auto exitItr = exitsToTry.begin(); exitItr != exitsToTry.end(); exitItr++)
        {
            auto exit = *exitItr;
            if (evaluateRequirement(worlds[exit->worldId], exit->requirement, items)) {
                //std::cout << areaToName(exit->parentArea) << " -> " << areaToName(exit->connectedArea) << " in world " << std::to_string(exit->worldId) << " found " << std::endl;
                // Erase the exit from the list of exits if we've met its requirement
                exitItr = exitsToTry.erase(exitItr);
                exitItr--;
                // If this exit's connected region has not been explored yet...
                auto connectedArea = worlds[exit->worldId].areaEntries[areaAsIndex(exit->connectedArea)];
                if (!connectedArea.isAccessible)
                {
                    connectedArea.isAccessible = true;
                    // std::cout << "Time to do some exploring!" << std::endl;
                    explore(worlds, items, connectedArea, exitsToTry, locationsToTry);
                }
            }
        }
        // Note which locations are now accessible on this iteration
        LocationPool accessibleThisIteration = {};
        //std::cout << "New Locations Accessible: " << std::endl;
        for (auto locItr = locationsToTry.begin(); locItr != locationsToTry.end(); locItr++)
        {
            auto location = *locItr;
            if (evaluateRequirement(worlds[location->worldId], location->requirement, items))
            {
                //std::cout << "\t" << locationToName(location->locationId) << " in world " << std::to_string(location->worldId) << std::endl;
                newThingsFound = true;
                // Delete newly accessible locations from the list
                accessibleThisIteration.push_back(location);
                locItr = locationsToTry.erase(locItr);
                locItr--;
            }
        }
        // Now apply any effects of newly accessible locations for the next iteration.
        // This lets us properly keep track of spheres for playthrough generation
        for (auto location : accessibleThisIteration)
        {
            if (location->currentItem.getGameItemId() == GameItem::INVALID)
            {
                accessibleLocations.push_back(location);
            }
            else
            {
                // Eventually optimize only adding advancement items
                items.push_back(location->currentItem);
            }
        }
    }
    while (newThingsFound);
    return accessibleLocations;
}

LocationPool getAccessibleLocations(WorldPool& worlds, ItemPool& items, LocationPool& allowedLocations)
{
    auto accessibleLocations = search(items, worlds);
    // Filter to only those locations which are allowed
    // return filterFromPool(accessibleLocations, [allowedLocations](LocationEntry* loc){return ElementInPool(loc, allowedLocations);});
    return accessibleLocations;
}
