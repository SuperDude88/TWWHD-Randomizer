#include "ItemPool.hpp"
#include "PoolFunctions.hpp"
#include <iostream>

static const GameItemPool alwaysItems = {
    GameItem::WindWaker,
    GameItem::WindsRequiem,
    GameItem::Shield,
    GameItem::SongofPassing,
    GameItem::BalladOfGales,
    GameItem::Sail,
    GameItem::DekuLeaf,
    GameItem::Sword,
    GameItem::PowerBracelets,
    GameItem::Bombs,
};

GameItemPool generateGameItemPool(const Settings& settings, int worldId)
{
    GameItemPool completeItemPool = {};

    // Add items which will always be in the item pool
    AddElementsToPool(completeItemPool, alwaysItems);

    return completeItemPool;
}

GameItemPool generateStartingGameItemPool(const Settings& settings, int worldId)
{
    static const GameItemPool startingItems = {
        GameItem::WindWaker,
        GameItem::WindsRequiem,
        GameItem::Shield,
        GameItem::SongofPassing,
        GameItem::BalladOfGales,
        GameItem::Sail,
    };
    // Add more items depending on settings

    return startingItems;
}

void printItemPool(const std::string& poolName, const ItemPool& itemPool)
{
    std::cout << poolName << ": " << std::endl;
    for (auto item : itemPool) {
        std::cout << "\t" << gameItemToName(item.getGameItemId()) << " for world " << std::to_string(item.getWorldId()) << std::endl;
    }
}
