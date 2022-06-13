
#pragma once

#include "GameItem.hpp"
#include "Location.hpp"
#include "Area.hpp"
#include <array>

enum struct DungeonId : uint32_t
{
    DragonRoostCavern = 0,
    ForbiddenWoods,
    TowerOfTheGods,
    ForsakenFortress,
    EarthTemple,
    WindTemple,
    INVALID,
};

struct Dungeon {
    int keyCount = -1;
    GameItem smallKey = GameItem::INVALID;
    GameItem bigKey = GameItem::INVALID;
    GameItem map = GameItem::INVALID;
    GameItem compass = GameItem::INVALID;
    std::vector<LocationId> locations = {};
    std::vector<LocationId> outsideDependentLocations = {}; // Locations which depend on beating the dungeon
};

const std::array<DungeonId, 6> getDungeonList();
const Dungeon nameToDungeon(const std::string& name);
std::string dungeonIdToName(const DungeonId& dungeonId);
DungeonId nameToDungeonId(const std::string& name);
Area dungeonIdToFirstRoom(const DungeonId& dungeonId);
const Dungeon dungeonIdToDungeon(const DungeonId& dungeonId);
