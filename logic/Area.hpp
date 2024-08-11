
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
};

std::string roomIndexToIslandName(const uint8_t& startingIslandRoomIndex);
uint8_t islandNameToRoomIndex(const std::string& islandArea);
uint8_t chartToRoomIndex(const GameItem& chart);
GameItem roomIndexToChart(const uint8_t& room);
