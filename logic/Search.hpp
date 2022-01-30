
#pragma once

#include "World.hpp"
#include <string>
#include <vector>

using WorldPool = std::vector<World>;

enum struct SearchMode
{
    INVALID = 0,
    AccessibleLocations,
    GameBeatable,
    AllLocationsReachable,
    GeneratePlaythrough,
};

LocationPool getAccessibleLocations(WorldPool& worlds, ItemPool& items, LocationPool& allowedLocations);
bool gameBeatable(WorldPool& worlds);
void generatePlaythrough(WorldPool& worlds);
