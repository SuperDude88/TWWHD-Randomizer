
#pragma once

#include "GameItem.hpp"
#include <list>

struct Dungeon {
    int keyCount = -1;
    GameItem smallKey = GameItem::INVALID;
    GameItem bigKey = GameItem::INVALID;
    GameItem map = GameItem::INVALID;
    GameItem compass = GameItem::INVALID;
    std::list<std::string> locations = {};
    std::list<std::string> outsideDependentLocations = {}; // Locations which depend on beating the dungeon
    std::string raceModeLocation = "";
    std::string entranceRoom = "";
    std::string island = "";
    std::string name = "";
    bool isRaceModeDungeon = false;
};

bool isValidDungeon(const std::string& dungeonName);
