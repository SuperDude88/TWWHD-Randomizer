
#pragma once

#include "GameItem.hpp"
#include "../options.hpp"
#include <vector>

using GameItemPool = std::vector<GameItem>;
using ItemPool = std::vector<Item>;

GameItemPool generateGameItemPool(const Settings& settings, int worldId);
GameItemPool generateStartingGameItemPool(const Settings& settings, int worldId);
void printItemPool(const std::string& poolName, const ItemPool& itemPool);
