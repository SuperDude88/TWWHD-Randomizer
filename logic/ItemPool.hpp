
#pragma once

#include "GameItem.hpp"
#include "../options.hpp"
#include <vector>

using GameItemPool = std::vector<GameItem>;
using ItemPool = std::vector<Item>;

GameItem getRandomJunk();
GameItemPool generateGameItemPool(const Settings& settings);
GameItemPool generateStartingGameItemPool(const Settings& settings);
void logItemPool(const std::string& poolName, const ItemPool& itemPool);
