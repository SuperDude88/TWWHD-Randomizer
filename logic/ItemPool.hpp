
#pragma once

#include "GameItem.hpp"
#include "Setting.hpp"

using ItemPool = std::unordered_multiset<GameItem>;

ItemPool generateItemPool(const Settings& settings, int worldId);
ItemPool getStartingInventory(const Settings& settings, int worldId);
