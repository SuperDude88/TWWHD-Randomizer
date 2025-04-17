
#pragma once

#include <logic/Requirements.hpp>
#include <logic/Entrance.hpp>

#include <cstdint>
#include <string>

class World;
struct EventAccess
{
    EventId event;
    Requirement requirement;
    Area* area = nullptr;
    World* world = nullptr;
};

class Area;
struct LocationAccess
{
    Area* area = nullptr;
    Location* location = nullptr;
    Requirement requirement;
};

class Area
{
public:
    std::string name = "";
    std::string island = "";
    std::string dungeon = "";
    std::string hintRegion = "";
    std::list<EventAccess> events;
    std::list<LocationAccess> locations;
    std::list<Entrance> exits;
    std::list<Entrance*> entrances;
    World* world = nullptr;

    // variables used for the searching algorithm
    bool isAccessible = false;

    std::string getRegion();
    std::list<std::string> findIslands();
    std::list<std::string> findHintRegions(bool onlyNonIslands = false);
    std::list<std::string> findDungeons();
    std::list<std::list<Entrance*>> findShuffledEntrances(const std::list<Area*>& startingQueue = {});
    std::unordered_map<Area*, EntrancePath> findEntrancePaths();

    bool operator<(const Area& rhs) const;
};

std::string roomNumToIslandName(const uint8_t& startingIslandRoomNum);
uint8_t islandNameToRoomNum(const std::string& islandArea);
