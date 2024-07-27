
#include "Area.hpp"

#include <iostream>
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

std::string roomIndexToIslandName(const uint8_t& startingIslandRoomIndex)
{
    // Island room number corresponds with index in the below array
    const std::array<std::string, 50> startingIslandAreaArray = {
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

    return startingIslandAreaArray[startingIslandRoomIndex];
}

uint8_t islandNameToRoomIndex(const std::string& islandName)
{
    static std::unordered_map<std::string, uint8_t> islandAreaMap = {
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

    return islandAreaMap[islandName];
}

uint8_t chartToRoomIndex(const GameItem& chart)
{
    std::unordered_map<GameItem, uint8_t> charts = {
        {GameItem::TreasureChart25, 1}, // Sector 1 Forsaken Fortress
        {GameItem::TreasureChart7,  2}, // Sector 2 Star Island
        {GameItem::TreasureChart24, 3}, // etc...
        {GameItem::TreasureChart42, 4},
        {GameItem::TreasureChart11, 5},
        {GameItem::TreasureChart45, 6},
        {GameItem::TreasureChart13, 7},
        {GameItem::TreasureChart41, 8},
        {GameItem::TreasureChart29, 9},
        {GameItem::TreasureChart22, 10},
        {GameItem::TreasureChart18, 11},
        {GameItem::TreasureChart30, 12},
        {GameItem::TreasureChart39, 13},
        {GameItem::TreasureChart19, 14},
        {GameItem::TreasureChart8,  15},
        {GameItem::TreasureChart2,  16},
        {GameItem::TreasureChart10, 17},
        {GameItem::TreasureChart26, 18},
        {GameItem::TreasureChart3,  19},
        {GameItem::TreasureChart37, 20},
        {GameItem::TreasureChart27, 21},
        {GameItem::TreasureChart38, 22},
        {GameItem::TriforceChart1,  23},
        {GameItem::TreasureChart21, 24},
        {GameItem::TreasureChart6,  25},
        {GameItem::TreasureChart14, 26},
        {GameItem::TreasureChart34, 27},
        {GameItem::TreasureChart5,  28},
        {GameItem::TreasureChart28, 29},
        {GameItem::TreasureChart35, 30},
        {GameItem::TriforceChart2,  31},
        {GameItem::TreasureChart44, 32},
        {GameItem::TreasureChart1,  33},
        {GameItem::TreasureChart20, 34},
        {GameItem::TreasureChart36, 35},
        {GameItem::TreasureChart23, 36},
        {GameItem::TreasureChart12, 37},
        {GameItem::TreasureChart16, 38},
        {GameItem::TreasureChart4,  39},
        {GameItem::TreasureChart17, 40},
        {GameItem::TreasureChart31, 41},
        {GameItem::TriforceChart3,  42},
        {GameItem::TreasureChart9,  43},
        {GameItem::TreasureChart43, 44},
        {GameItem::TreasureChart40, 45},
        {GameItem::TreasureChart46, 46},
        {GameItem::TreasureChart15, 47},
        {GameItem::TreasureChart32, 48},
        {GameItem::TreasureChart33, 49}, // Sector 49 Five Star Isles
    };

    if (!charts.contains(chart))
    {
        return 0;
    }

    return charts[chart];
}

GameItem roomIndexToChart(const uint8_t& room)
{
    std::unordered_map<uint8_t, GameItem> rooms = {
        {1,  GameItem::TreasureChart25}, // Sector 1 Forsaken Fortress
        {2,  GameItem::TreasureChart7 }, // Sector 2 Star Island
        {3,  GameItem::TreasureChart24}, // etc...
        {4,  GameItem::TreasureChart42},
        {5,  GameItem::TreasureChart11},
        {6,  GameItem::TreasureChart45},
        {7,  GameItem::TreasureChart13},
        {8,  GameItem::TreasureChart41},
        {9,  GameItem::TreasureChart29},
        {10, GameItem::TreasureChart22},
        {11, GameItem::TreasureChart18},
        {12, GameItem::TreasureChart30},
        {13, GameItem::TreasureChart39},
        {14, GameItem::TreasureChart19},
        {15, GameItem::TreasureChart8 },
        {16, GameItem::TreasureChart2 },
        {17, GameItem::TreasureChart10},
        {18, GameItem::TreasureChart26},
        {19, GameItem::TreasureChart3 },
        {20, GameItem::TreasureChart37},
        {21, GameItem::TreasureChart27},
        {22, GameItem::TreasureChart38},
        {23, GameItem::TriforceChart1 },
        {24, GameItem::TreasureChart21},
        {25, GameItem::TreasureChart6 },
        {26, GameItem::TreasureChart14},
        {27, GameItem::TreasureChart34},
        {28, GameItem::TreasureChart5 },
        {29, GameItem::TreasureChart28},
        {30, GameItem::TreasureChart35},
        {31, GameItem::TriforceChart2 },
        {32, GameItem::TreasureChart44},
        {33, GameItem::TreasureChart1 },
        {34, GameItem::TreasureChart20},
        {35, GameItem::TreasureChart36},
        {36, GameItem::TreasureChart23},
        {37, GameItem::TreasureChart12},
        {38, GameItem::TreasureChart16},
        {39, GameItem::TreasureChart4 },
        {40, GameItem::TreasureChart17},
        {41, GameItem::TreasureChart31},
        {42, GameItem::TriforceChart3 },
        {43, GameItem::TreasureChart9 },
        {44, GameItem::TreasureChart43},
        {45, GameItem::TreasureChart40},
        {46, GameItem::TreasureChart46},
        {47, GameItem::TreasureChart15},
        {48, GameItem::TreasureChart32},
        {49, GameItem::TreasureChart33}, // Sector 49 Five Star Isles
    };

    if (!rooms.contains(room))
    {
        return GameItem::INVALID;
    }

    return rooms[room];
}