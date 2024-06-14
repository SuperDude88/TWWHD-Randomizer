
#pragma once

#include <logic/World.hpp>

enum struct FillError
{
    NONE = 0,
    RAN_OUT_OF_RETRIES,
    MORE_ITEMS_THAN_LOCATIONS,
    NO_REACHABLE_LOCATIONS,
    GAME_NOT_BEATABLE,
    NOT_ENOUGH_PROGRESSION_LOCATIONS,
    PLANDOMIZER_ERROR,
};

void placeVanillaItems(WorldPool& worlds);
FillError forwardFillUntilMoreFreeSpace(WorldPool& worlds, ItemPool& itemsToPlace, LocationPool allowedLocations, size_t openLocations = 3);
FillError validateEnoughLocations(WorldPool& worlds);
void determineMajorItems(WorldPool& worlds, ItemPool& itemPool, LocationPool& allLocations);
FillError fill(std::vector<World>& worlds);
void clearWorlds(WorldPool& worlds);
std::string errorToName(FillError err);
