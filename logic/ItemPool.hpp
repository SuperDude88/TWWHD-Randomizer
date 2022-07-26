
#pragma once

#include "GameItem.hpp"
#include "../options.hpp"
#include <vector>

using GameItemPool = std::vector<GameItem>;
using ItemPool = std::vector<Item>;

class World;
GameItem getRandomJunk();
GameItemPool generateGameItemPool(const Settings& settings, World* world);
GameItemPool generateStartingGameItemPool(const Settings& settings);
void logItemPool(const std::string& poolName, const ItemPool& itemPool);
