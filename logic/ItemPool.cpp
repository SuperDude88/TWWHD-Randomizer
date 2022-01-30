#include "ItemPool.hpp"
#include "PoolFunctions.hpp"
#include <iostream>

static const GameItemPool alwaysItems = {
    GameItem::WindWaker,
    GameItem::WindsRequiem,
    GameItem::SongOfPassing,
    GameItem::BalladOfGales,
    GameItem::CommandMelody,
    GameItem::EarthGodsLyric,
    GameItem::WindGodsAria,
    GameItem::BoatsSail,
    GameItem::BaitBag,
    GameItem::ProgressiveShield,
    GameItem::ProgressiveShield,
    GameItem::ProgressiveSword,
    GameItem::ProgressiveSword,
    GameItem::ProgressiveSword,
    GameItem::ProgressiveSword,
    GameItem::ProgressivePictoBox,
    GameItem::ProgressivePictoBox,
    GameItem::GrapplingHook,
    GameItem::DekuLeaf,
    GameItem::Boomerang,
    GameItem::Bombs,
    GameItem::ProgressiveWallet,
    GameItem::ProgressiveWallet,
    GameItem::ProgressiveBow,
    GameItem::ProgressiveBow,
    GameItem::ProgressiveBow,
    GameItem::SkullHammer,
    GameItem::PowerBracelets,
    GameItem::IronBoots,
    GameItem::Hookshot,
    GameItem::TriforceShard1,
    GameItem::TriforceShard2,
    GameItem::TriforceShard3,
    GameItem::TriforceShard4,
    GameItem::TriforceShard5,
    GameItem::TriforceShard6,
    GameItem::TriforceShard7,
    GameItem::TriforceShard8,
    GameItem::NayrusPearl,
    GameItem::DinsPearl,
    GameItem::FaroresPearl,
    GameItem::EmptyBottle,
    GameItem::MagicMeterUpgrade,
    GameItem::DragonTingleStatue,
    GameItem::ForbiddenTingleStatue,
    GameItem::GoddessTingleStatue,
    GameItem::EarthTingleStatue,
    GameItem::WindTingleStatue,
    GameItem::GhostShipChart,

    GameItem::TreasureChart7,
    GameItem::TreasureChart27,
    GameItem::TreasureChart21,
    GameItem::TreasureChart13,
    GameItem::TreasureChart32,
    GameItem::TreasureChart19,
    GameItem::TreasureChart41,
    GameItem::TreasureChart26,
    GameItem::TreasureChart8,
    GameItem::TreasureChart37,
    GameItem::TreasureChart25,
    GameItem::TreasureChart17,
    GameItem::TreasureChart36,
    GameItem::TreasureChart22,
    GameItem::TreasureChart9,
    GameItem::TreasureChart14,
    GameItem::TreasureChart10,
    GameItem::TreasureChart40,
    GameItem::TreasureChart3,
    GameItem::TreasureChart4,
    GameItem::TreasureChart28,
    GameItem::TreasureChart16,
    GameItem::TreasureChart18,
    GameItem::TreasureChart34,
    GameItem::TreasureChart29,
    GameItem::TreasureChart1,
    GameItem::TreasureChart35,
    GameItem::TreasureChart12,
    GameItem::TreasureChart6,
    GameItem::TreasureChart24,
    GameItem::TreasureChart39,
    GameItem::TreasureChart38,
    GameItem::TreasureChart2,
    GameItem::TreasureChart33,
    GameItem::TreasureChart31,
    GameItem::TreasureChart23,
    GameItem::TreasureChart5,
    GameItem::TreasureChart20,
    GameItem::TreasureChart30,
    GameItem::TreasureChart15,
    GameItem::TreasureChart11,
    GameItem::TreasureChart46,
    GameItem::TreasureChart45,
    GameItem::TreasureChart44,
    GameItem::TriforceChart3,
    GameItem::TreasureChart43,
    GameItem::TriforceChart2,
    GameItem::TreasureChart42,
    GameItem::TriforceChart1,

    GameItem::DRCSmallKey,
    GameItem::DRCSmallKey,
    GameItem::DRCSmallKey,
    GameItem::DRCSmallKey,
    GameItem::DRCBigKey,

    GameItem::FWSmallKey,
    GameItem::FWBigKey,

    GameItem::TotGSmallKey,
    GameItem::TotGSmallKey,
    GameItem::TotGBigKey,

    GameItem::ETSmallKey,
    GameItem::ETSmallKey,
    GameItem::ETSmallKey,
    GameItem::ETBigKey,

    GameItem::WTSmallKey,
    GameItem::WTSmallKey,
    GameItem::WTBigKey,
};

GameItemPool generateGameItemPool(const Settings& settings, int worldId)
{
    GameItemPool completeItemPool = {};

    // Add items which will always be in the item pool
    addElementsToPool(completeItemPool, alwaysItems);

    return completeItemPool;
}

GameItemPool generateStartingGameItemPool(const Settings& settings, int worldId)
{
    GameItemPool startingItems = {
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
