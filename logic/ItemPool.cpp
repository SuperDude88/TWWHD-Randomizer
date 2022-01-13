#include "ItemPool.hpp"
#include "PoolFunctions.hpp"

static const ItemPool alwaysItems = {
    GameItem::Boomerang,
    GameItem::Hookshot,
    GameItem::DekuLeaf,
    GameItem::Bombs,
    GameItem::BaitBag,
    GameItem::SpoilsBag,
    GameItem::Sword,
};

static const ItemPool startingItems = {
    GameItem::WindWaker,
    GameItem::WindsRequiem,
    GameItem::Shield,
    GameItem::SongofPassing,
    GameItem::BalladOfGales,
    GameItem::Sail,
};

ItemPool generateItemPool(const Settings& settings, int worldId)
{
    ItemPool completeItemPool = {};

    // Add items which will always be in the item pool
    AddElementsToPool(completeItemPool, alwaysItems);

    return completeItemPool;
}

ItemPool getStartingInventory(const Settings& settings, int worldId)
{

    // Add more items depending on settings

    return startingItems;
}
