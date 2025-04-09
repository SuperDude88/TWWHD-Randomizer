
#include "Area.hpp"

#include <unordered_map>
#include <array>

#include <logic/PoolFunctions.hpp>
#include <logic/World.hpp>

std::string Area::getRegion()
{
    if (!island.empty()) return island;
    if (!dungeon.empty()) return dungeon;
    if (!hintRegion.empty()) return hintRegion;
    return "";
}

std::list<std::string> Area::findIslands()
{
    std::list<std::string> islands = {};
    std::unordered_set<Area*> alreadyChecked = {};
    std::list<Area*> areaQueue = {this};

    while (!areaQueue.empty())
    {
        auto area = areaQueue.back();
        alreadyChecked.insert(area);
        areaQueue.pop_back();

        // If we found Hyrule, then add the Tower of the Gods Sector to the islands
        if (area->hintRegion == "Hyrule" && !elementInPool("Tower of the Gods Sector", islands))
        {
            islands.push_back("Tower of the Gods Sector");
        }

        // Block searching through areas that have general hint regions
        if (area->hintRegion != "")
        {
            continue;
        }

        // If we found an island, add it to the list
        if (area->island != "")
        {
            if (!elementInPool(area->island, islands))
            {
                islands.push_back(area->island);  
            }
            continue;
        }

        // If this area isn't an island, add its entrances to the queue as long
        // as they haven't been checked yet
        for (auto entrance : area->entrances)
        {
            if (!alreadyChecked.contains(entrance->getParentArea()))
            {
                areaQueue.push_front(entrance->getParentArea());
            }
        }
    }

    return islands;
}

std::list<std::string> Area::findDungeons()
{
    std::list<std::string> dungeons = {};
    std::unordered_set<Area*> alreadyChecked = {};
    std::list<Area*> areaQueue = {this};

    while (!areaQueue.empty())
    {
        auto area = areaQueue.back();
        alreadyChecked.insert(area);
        areaQueue.pop_back();

        // If we found an island or general hint region, then this
        // area isn't part of a dungeon
        if (area->hintRegion != "" || area->island != "")
        {
            return {};
        }

        if (area->dungeon != "")
        {
            if (!elementInPool(area->dungeon, dungeons))
            {
                dungeons.push_back(area->dungeon);
            }
            continue;
        }

        // If this area isn't a dungeon, add its entrances to the queue as long
        // as they haven't been checked yet
        for (auto entrance : area->entrances)
        {
            if (!alreadyChecked.contains(entrance->getParentArea()))
            {
                areaQueue.push_front(entrance->getParentArea());
            }
        }
    }
    return dungeons;
}

std::list<std::string> Area::findHintRegions(bool onlyNonIslands /* = false */)
{
    std::list<std::string> regions = {};
    std::unordered_set<Area*> alreadyChecked = {};
    std::list<Area*> areaQueue = {this};

    while (!areaQueue.empty())
    {
        auto area = areaQueue.back();
        alreadyChecked.insert(area);
        areaQueue.pop_back();

        // If we found an island or general hint region, then this
        // area isn't part of a dungeon
        if (area->hintRegion != "")
        {
            if (!elementInPool(area->hintRegion, regions))
            {
                regions.push_back(area->hintRegion);
            }
        }

        if (area->island != "" && !onlyNonIslands)
        {
            if (!elementInPool(area->island, regions))
            {
                regions.push_back(area->island);
            }
        }

        if (area->dungeon != "")
        {
            if (!elementInPool(area->dungeon, regions))
            {
                regions.push_back(area->dungeon);
            }
        }

        // If this area doesn't have any possible hint regions, add its entrances
        // to the queue as long as they haven't been checked yet
        if (area->dungeon == "" && area->hintRegion == "" && area->island == "")
        {
            for (const Entrance* entrance : area->entrances)
            {
                if (!alreadyChecked.contains(entrance->getParentArea()))
                {
                    areaQueue.push_front(entrance->getParentArea());
                }
            }
        }
    }

    // Erase dungeons from the list if we have any islands or other hint regions
    std::list<std::string> dungeons = {};
    std::ranges::copy_if(regions, std::back_inserter(dungeons), [&](const std::string& region){return this->world->dungeons.contains(region);});
    if (dungeons.size() < regions.size())
    {
        std::erase_if(regions, [&](const std::string& region){return this->world->dungeons.contains(region);});
    }

    return regions;
}

// Performs a breadth first search to find all the shuffled entrances
// within a given area. The area must have its own defined island, hint region, or dungeon
// Returns the shuffled entrances in the order they were discovered by shuffled entrance spheres
std::list<std::list<Entrance*>> Area::findShuffledEntrances(const std::list<Area*>& startingQueue /* = {} */)
{
    // Don't search if this area doesn't have a hard assigned region
    if (this->getRegion().empty()) return {};

    

    std::list<std::list<Entrance*>> shuffledEntrances = {};
    std::unordered_set<Area*> alreadyCheckedAreas = {};
    std::unordered_set<Entrance*> alreadyCheckedEntrances = {};
    std::list<Area*> areaQueue = startingQueue;
    if (areaQueue.empty())
    {
        areaQueue.push_front(this);
    }

    std::list<Entrance*> entrancesToTry = {};
    do
    {
        entrancesToTry.clear();
        for (auto area : areaQueue)
        {
            for (auto& e : area->exits)
            {   
                auto entrance = &e;
                if (alreadyCheckedEntrances.contains(entrance))
                {
                    continue;
                }
                // Only add entrances which fit the following criteria:
                // - The entrance is shuffled
                // - The entrance is decoupled OR the entrance isn't connected OR the entrance's replaced reverse hasn't been added yet
                if (entrance->isShuffled())
                {
                    if (entrance->isDecoupled() || 
                     entrance->getReplaces() == nullptr || 
                     !alreadyCheckedEntrances.contains(entrance->getReplaces()->getReverse()))
                    {
                        entrancesToTry.push_back(entrance);
                    }
                }
                else
                {
                    auto connectedArea = entrance->getConnectedArea();
                    if (connectedArea)
                    {
                        if (!alreadyCheckedAreas.contains(connectedArea) && (connectedArea->getRegion() == this->getRegion() || connectedArea->getRegion().empty()))
                        {
                            areaQueue.push_back(connectedArea);
                            alreadyCheckedAreas.insert(connectedArea);
                        }
                    }
                }
                alreadyCheckedEntrances.insert(entrance);
            }
        }
        areaQueue.clear();

        if (!entrancesToTry.empty())
        {
            shuffledEntrances.push_back(entrancesToTry);
        }

        // Gather all the new areas we can find to try for shuffled entrances
        for (auto entrance : entrancesToTry)
        {
            auto connectedArea = entrance->getConnectedArea();
            if (connectedArea)
            {
                if (!alreadyCheckedAreas.contains(connectedArea) && (connectedArea->getRegion() == this->getRegion() || connectedArea->getRegion().empty()))
                {
                    areaQueue.push_back(connectedArea);
                    alreadyCheckedAreas.insert(connectedArea);
                }
            }
        }
        
    } while (!entrancesToTry.empty());

    return shuffledEntrances;
}

// Finds the shortest path of *shuffled* entrances from this area to all sub-areas.
// This should only be called from Islands or general hint regions (e.g. Hyrule)
std::unordered_map<Area*, EntrancePath> Area::findEntrancePaths()
{
    std::unordered_map<Area*, EntrancePath> paths = {{this, {{}, EntrancePath::Logicality::Full}}};
    std::unordered_map<Area*, int> dists = {{this, 0}};
    std::list<Area*> queue = {this};
    std::list<Entrance*> nextDistanceEntrances = {};
    int curShuffledEntranceDistance = 0;

    auto region = this->island;
    if (region.empty()) region = this->hintRegion;
    if (region.empty()) return paths;

    while (!queue.empty())
    {
        auto area = queue.front();
        queue.pop_front();

        for (auto& e : area->exits)
        {
            auto nextArea = e.getConnectedArea();
            // If this entrance is not connected, or if this is another island/Hyrule/The Great Sea,
            // then don't try to find a path with this entrance.
            if (nextArea == nullptr || (islandNameToRoomNum(nextArea->name) != 0) || nextArea->hintRegion == "Hyrule" || nextArea->name == "The Great Sea")
            {
                continue;
            }

            // If this entrance is shuffled, put it into the list of entrances to
            // try for the next iteration of entrance distances
            if (e.isShuffled())
            {
                nextDistanceEntrances.push_back(&e);
            }
            else if (!dists.contains(nextArea) || dists[nextArea] > curShuffledEntranceDistance)
            {
                paths[nextArea] = paths[area];
                // If this non-shuffled entrance has not been found, mark the path we take
                // with it as partially logical. A partially logical path is one that
                // has all of the shuffled entrances in it logically available, but there's
                // some non-shuffled entrance after them all which is not logically
                // accessible
                if (paths[nextArea].logicality == EntrancePath::Logicality::Full && !e.hasBeenFound())
                {
                    paths[nextArea].logicality = EntrancePath::Logicality::Partial;
                }
                dists[nextArea] = curShuffledEntranceDistance;
                queue.push_back(nextArea);
            }
        }

        // Once the queue is empty, put in all the areas for the next shuffle distance
        // based on the shuffled entrances we've found so far
        if (queue.empty())
        {
            curShuffledEntranceDistance++;
            for (auto e : nextDistanceEntrances)
            {
                auto nextArea = e->getConnectedArea();
                auto parentArea = e->getParentArea();

                // Don't bother queueing the area if it already is inserted and has a distance
                // less than or equal to the current distance
                if (islandNameToRoomNum(nextArea->name) != 0 || nextArea->hintRegion == "Hyrule" || 
                     (dists.contains(nextArea) && dists[nextArea] <= curShuffledEntranceDistance))
                {
                    continue;
                }

                // Add the new shuffled entrance to the list of entrances on this path
                paths[nextArea] = paths[parentArea];
                paths[nextArea].list.push_back(e);
                
                // If this path is partially logical and we've found another shuffled
                // entrance to add onto it, then it becomes fully nonlogical because
                // some entrance before the final shuffled one is not able to be
                // logically traversed.
                if (paths[nextArea].logicality == EntrancePath::Logicality::Partial)
                {
                    paths[nextArea].logicality = EntrancePath::Logicality::None;
                }
                dists[nextArea] = curShuffledEntranceDistance;
                queue.push_back(nextArea);
            }
            nextDistanceEntrances.clear();
        }
    }

    return paths;
}

std::string roomNumToIslandName(const uint8_t& startingIslandRoomNum)
{
    // Island room number corresponds with index in the below array
    static const std::array<std::string, 50> startingIslandAreaArray = {
        "INVALID",
        "Forsaken Fortress Sector",
        "Star Island",
        "Northern Fairy Island",
        "Gale Isle",
        "Crescent Moon Island",
        "Seven Star Isles",
        "Overlook Island",
        "Four Eye Reef",
        "Mother & Child Isles",
        "Spectacle Island",
        "Windfall Island",
        "Pawprint Isle",
        "Dragon Roost Island",
        "Flight Control Platform",
        "Western Fairy Island",
        "Rock Spire Isle",
        "Tingle Island",
        "Northern Triangle Island",
        "Eastern Fairy Island",
        "Fire Mountain",
        "Star Belt Archipelago",
        "Three Eye Reef",
        "Greatfish Isle",
        "Cyclops Reef",
        "Six Eye Reef",
        "Tower of the Gods Sector",
        "Eastern Triangle Island",
        "Thorned Fairy Island",
        "Needle Rock Isle",
        "Islet of Steel",
        "Stone Watcher Island",
        "Southern Triangle Island",
        "Private Oasis",
        "Bomb Island",
        "Birds Peak Rock",
        "Diamond Steppe Island",
        "Five Eye Reef",
        "Shark Island",
        "Southern Fairy Island",
        "Ice Ring Isle",
        "Forest Haven",
        "Cliff Plateau Isles",
        "Horseshoe Island",
        "Outset Island",
        "Headstone Island",
        "Two Eye Reef",
        "Angular Isles",
        "Boating Course",
        "Five Star Isles",
    };

    if(startingIslandRoomNum <= 0 || startingIslandRoomNum >= 50) {
        return "INVALID";
    }

    return startingIslandAreaArray[startingIslandRoomNum];
}

uint8_t islandNameToRoomNum(const std::string& islandName)
{
    static const std::unordered_map<std::string, uint8_t> islandAreaMap = {
        {"Forsaken Fortress Sector", 1},
        {"Star Island", 2},
        {"Northern Fairy Island", 3},
        {"Gale Isle", 4},
        {"Crescent Moon Island", 5},
        {"Seven Star Isles", 6},
        {"Overlook Island", 7},
        {"Four Eye Reef", 8},
        {"Mother & Child Isles", 9},
        {"Spectacle Island", 10},
        {"Windfall Island", 11},
        {"Pawprint Isle", 12},
        {"Dragon Roost Island", 13},
        {"Flight Control Platform", 14},
        {"Western Fairy Island", 15},
        {"Rock Spire Isle", 16},
        {"Tingle Island", 17},
        {"Northern Triangle Island", 18},
        {"Eastern Fairy Island", 19},
        {"Fire Mountain", 20},
        {"Star Belt Archipelago", 21},
        {"Three Eye Reef", 22},
        {"Greatfish Isle", 23},
        {"Cyclops Reef", 24},
        {"Six Eye Reef", 25},
        {"Tower of the Gods Sector", 26},
        {"Eastern Triangle Island", 27},
        {"Thorned Fairy Island", 28},
        {"Needle Rock Isle", 29},
        {"Islet of Steel", 30},
        {"Stone Watcher Island", 31},
        {"Southern Triangle Island", 32},
        {"Private Oasis", 33},
        {"Bomb Island", 34},
        {"Birds Peak Rock", 35},
        {"Diamond Steppe Island", 36},
        {"Five Eye Reef", 37},
        {"Shark Island", 38},
        {"Southern Fairy Island", 39},
        {"Ice Ring Isle", 40},
        {"Forest Haven", 41},
        {"Cliff Plateau Isles", 42},
        {"Horseshoe Island", 43},
        {"Outset Island", 44},
        {"Headstone Island", 45},
        {"Two Eye Reef", 46},
        {"Angular Isles", 47},
        {"Boating Course", 48},
        {"Five Star Isles", 49},
    };

    if (!islandAreaMap.contains(islandName))
    {
        return 0;
    }

    return islandAreaMap.at(islandName);
}
