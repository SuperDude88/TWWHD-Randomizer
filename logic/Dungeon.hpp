
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
    std::list<Location*> outsideDependentLocations = {}; // Locations which depend on beating the dungeon
    Location* raceModeLocation = nullptr;
    Area* startingArea = nullptr;
    Entrance* startingEntrance = nullptr;
    std::list<std::string> islands = {};
    std::string name = "";
    bool isRequiredDungeon = false;
    bool hasNaturalRaceModeLocation = false;
};

bool isValidDungeon(const std::string& dungeonName);
