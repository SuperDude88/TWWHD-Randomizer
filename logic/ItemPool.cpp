#include "ItemPool.hpp"
#include "PoolFunctions.hpp"
#include <iostream>

static const GameItemPool alwaysItems = {
    GameItem::WindWaker,
    GameItem::WindsRequiem,
    GameItem::ProgressiveShield,
    GameItem::SongOfPassing,
    GameItem::BalladOfGales,
    GameItem::BoatsSail,
    GameItem::DekuLeaf,
    GameItem::ProgressiveSword,
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
        GameItem::ProgressiveShield,
        GameItem::SongOfPassing,
        GameItem::BalladOfGales,
        GameItem::BoatsSail,
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
