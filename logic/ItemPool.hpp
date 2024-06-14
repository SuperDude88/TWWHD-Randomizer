
#pragma once

#include <vector>

#include <options.hpp>
#include <logic/GameItem.hpp>

using GameItemPool = std::vector<GameItem>;
using ItemNamePool = std::vector<std::string>;
using ItemPool = std::vector<Item>;

class World;
std::string getRandomJunk();
ItemNamePool generateGameItemPool(const Settings& settings, World* world);
ItemNamePool generateStartingGameItemPool(const Settings& settings);
void logItemPool(const std::string& poolName, const ItemPool& itemPool);
