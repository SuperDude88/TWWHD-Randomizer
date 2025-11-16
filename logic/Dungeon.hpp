
#pragma once

#include <list>

#include <logic/GameItem.hpp>

class Entrance;
class Area;
class Location;

struct Dungeon {
    int keyCount = -1;
    Item smallKey = Item();
    Item bigKey = Item();
    Item map = Item();
    Item compass = Item();
    std::list<Location*> locations = {};
    Location* raceModeLocation = nullptr;
    Area* startingArea = nullptr;
    Entrance* startingEntrance = nullptr;
    std::list<std::string> islands = {};
    std::string name = "";
    bool isRequiredDungeon = false;
    bool hasNaturalRaceModeLocation = false;
    std::string windWarpExitStage = "";
    uint8_t windWarpExitRoom = 0;
    uint8_t windWarpExitSpawn = 0;
    std::string savewarpStage = "";
    uint8_t savewarpRoom = 0;
    uint8_t savewarpSpawn = 0;

    std::list<Location*> getOutsideDependentLocations() const;
};

bool isValidDungeon(const std::string& dungeonName);
