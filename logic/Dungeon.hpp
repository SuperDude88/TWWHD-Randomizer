
#pragma once

#include "GameItem.hpp"
#include <list>

struct Dungeon {
    int keyCount = -1;
    std::string smallKey = "";
    std::string bigKey = "";
    std::string map = "";
    std::string compass = "";
    std::list<std::string> locations = {};
    std::list<std::string> outsideDependentLocations = {}; // Locations which depend on beating the dungeon
    std::string raceModeLocation = "";
    std::string entranceRoom = "";
    std::string island = "";
    std::string name = "";
    bool isRaceModeDungeon = false;
};

bool isValidDungeon(const std::string& dungeonName);
