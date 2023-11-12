
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

    std::list<std::string> findIslands();
    std::list<std::string> findHintRegions();
    std::list<std::string> findDungeons();
};

std::string roomIndexToIslandName(const uint8_t& startingIslandRoomIndex);
uint8_t islandNameToRoomIndex(const std::string& islandArea);
