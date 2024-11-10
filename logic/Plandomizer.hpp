#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include <utility/path.hpp>
#include <logic/GameItem.hpp>
#include <logic/Entrance.hpp>
#include <logic/Location.hpp>

enum struct PlandomizerError
{
    NONE = 0,
    BAD_STARTING_ISLAND,
    BAD_STARTING_ITEM,
    MISSING_ITEM_KEY,
    NO_ITEM_AT_LOCATION,
    INVALID_WORLD_ID,
    MISSING_PARENT_ENTRANCE,
    MISSING_PLANDO_LOCATION,
    UNKNOWN_ITEM_NAME,
};

struct PlandomizerItem
{
    GameItem gameItem = GameItem::INVALID;
    int world = -1;
};

struct Plandomizer
{
    // Load in some plando data as strings first, and then
    // make the actual data during the appropriate part of
    // the world building process
    std::unordered_map<std::string, std::string> entrancesStr = {};
    std::unordered_map<std::string, PlandomizerItem> locationsStr = {};
    std::unordered_map<Location*, Item> locations = {};
    std::unordered_map<Entrance*, Entrance*> entrances = {};
    std::vector<GameItem> randomStartingItemPool = {};
    uint8_t startingIslandRoomNum = 0;
};

PlandomizerError loadPlandomizer(const fspath& plandoFilepath, std::vector<Plandomizer>& plandos, size_t numWorlds);
std::string errorToName(PlandomizerError err);
