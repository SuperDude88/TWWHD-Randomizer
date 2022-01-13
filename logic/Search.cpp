
#include "Search.hpp"
#include "World.hpp"
#include "GameItem.hpp"

#include <unordered_set>
#include <queue>
#include <functional>
#include <algorithm>
#include <random>

std::vector<uint32_t> assumedSearch(std::vector<GameItem>& ownedItems, const World& World)
{
    std::unordered_set<GameItem> newItems;
    std::vector<uint32_t> reachable;
    do
    {
        reachable = accessibleLocations(ownedItems, locations);
        std::unordered_set<uint8_t> newItems;
        for (const auto& r : reachable)
        {
            auto currentItem = locations[r].currentItem;
            if (currentItem == -1) continue;
            newItems.insert(currentItem);
        }
        for (const auto& o : ownedItems)
        {
            newItems.erase(o);
        }
        for ( const auto& n : newItems)
        {
            ownedItems.push_back(n);
        }
    } while(!newItems.empty());
    return reachable;
}

void assumedFill(const std::vector<uint8_t>& items, std::vector<ItemLocation>& locations)
{
    std::vector<uint8_t> ownedItems = items;
    std::unordered_set<uint8_t> availableItems{};
    uint32_t locationsRemaining = locations.size();
    // put seed here later
    auto randomEngine = std::default_random_engine();
    std::shuffle(ownedItems.begin(), ownedItems.end(), randomEngine);
    while(locationsRemaining > 0 && !ownedItems.empty())
    {
        auto removed = ownedItems.back();
        ownedItems.pop_back();
        auto reachable = assumedSearch(ownedItems, locations);
        std::vector<uint32_t> nullReachable;
        for (uint32_t idx = 0; idx < reachable.size(); idx++)
        {
            uint32_t reachableIdx = reachable[idx];
            if (locations[reachableIdx].currentItem == -1)
            {
                nullReachable.push_back(reachableIdx);
            }
        }
        auto rand = std::uniform_int_distribution<uint32_t>(0, nullReachable.size() - 1);
        uint32_t randomNullIdx = nullReachable[rand(randomEngine)];
        locations[randomNullIdx].currentItem = removed;
        availableItems.insert(removed);
    }
}
