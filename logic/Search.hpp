
#pragma once

#include "World.hpp"
#include <string>
#include <vector>

using WorldPool = std::vector<World>;

LocationPool getAccessibleLocations(WorldPool& worlds, ItemPool& items, LocationPool& allowedLocations);
