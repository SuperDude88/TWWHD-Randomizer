
#pragma once

#include <string>
#include <vector>

#include <logic/World.hpp>

enum struct SearchMode
{
    INVALID = 0,
    AccessibleLocations,
    GameBeatable,
    AllLocationsReachable,
    GeneratePlaythrough,
};

LocationPool getAccessibleLocations(WorldPool& worlds, ItemPool& items, LocationPool& allowedLocations, int worldToSearch = -1);
void runGeneralSearch(WorldPool& worlds, int worldToSearch = -1);
bool gameBeatable(WorldPool& worlds);
void generatePlaythrough(WorldPool& worlds);
bool locationsReachable(WorldPool& worlds, ItemPool& items, LocationPool& locationsToCheck, int worldToSearch = -1);
bool allLocationsReachable(WorldPool& worlds, ItemPool& items, int worldToSearch = -1);
