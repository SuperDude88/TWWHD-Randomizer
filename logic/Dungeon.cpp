
#include "Dungeon.hpp"
#include <unordered_map>

// The last locationId in each list of dungeon locations is the initial race mode location
static const Dungeon DragonRoostCavern = {4, GameItem::DRCSmallKey, GameItem::DRCBigKey, GameItem::DRCDungeonMap, GameItem::DRCCompass, {
            LocationId::DragonRoostCavernFirstRoomChest,
            LocationId::DragonRoostCavernWaterJugAlcove,
            LocationId::DragonRoostCavernBoardedUpChest,
            LocationId::DragonRoostCavernSwingAcrossLavaChest,
            LocationId::DragonRoostCavernRatRoomChest,
            LocationId::DragonRoostCavernRatRoomBoardedUpChest,
            LocationId::DragonRoostCavernBirdsNest,
            LocationId::DragonRoostCavernDarkRoomChest,
            LocationId::DragonRoostCavernHubRoomTingleChest,
            LocationId::DragonRoostCavernPotRoomChest,
            LocationId::DragonRoostCavernMiniBoss,
            LocationId::DragonRoostCavernUnderRopeBridge,
            LocationId::DragonRoostCavernTingleStatueChest,
            LocationId::DragonRoostCavernBigKeyChest,
            LocationId::DragonRoostCavernBossStairsRightChest,
            LocationId::DragonRoostCavernBossStairsLeftChest,
            LocationId::DragonRoostCavernGohmaHeartContainer,
        }
};
static const Dungeon ForbiddenWoods = {1, GameItem::FWSmallKey, GameItem::FWBigKey, GameItem::FWDungeonMap, GameItem::FWCompass, {
            LocationId::ForbiddenWoodsFirstRoomChest,
            LocationId::ForbiddenWoodsInsideHollowTree,
            LocationId::ForbiddenWoodsBokoBabaClimb,
            LocationId::ForbiddenWoodsHoleInTreeChest,
            LocationId::ForbiddenWoodsMorthPit,
            LocationId::ForbiddenWoodsVineMazeLeftChest,
            LocationId::ForbiddenWoodsVineMazeRightChest,
            LocationId::ForbiddenWoodsTallRoomChest,
            LocationId::ForbiddenWoodsMothulaMiniBoss,
            LocationId::ForbiddenWoodsByHangingSeedsChest,
            LocationId::ForbiddenWoodsChestAcrossHangingFlower,
            LocationId::ForbiddenWoodsTingleStatueChest,
            LocationId::ForbiddenWoodsLockedTreeTrunkChest,
            LocationId::ForbiddenWoodsBigKeyChest,
            LocationId::ForbiddenWoodsDoubleMothulaRoomChest,
            LocationId::ForbiddenWoodsKalleDemosHeartContainer,
        }, {     // Outside dungeon dependencies
            LocationId::MailboxOrcaLetter,
        }
};
static const Dungeon TowerOfTheGods = {2, GameItem::TotGSmallKey, GameItem::TotGBigKey, GameItem::TotGDungeonMap, GameItem::TotGCompass, {
            LocationId::TOTGChestBehindBombableWall,
            LocationId::TOTGHopAcrossFloatingBoxesChest,
            LocationId::TOTGLightTwoTorches,
            LocationId::TOTGSkullRoomChest,
            LocationId::TOTGShootEyeAboveSkulls,
            LocationId::TOTGTingleStatueChest,
            LocationId::TOTGFirstArmosKnightsChest,
            LocationId::TOTGStoneTablet,
            LocationId::TOTGDarknutMiniBoss,
            LocationId::TOTGSecondArmostKnightsChest,
            LocationId::TOTGFloatingPlatformsRoomLowerChest,
            LocationId::TOTGFloatingPlatformsRoomUpperChest,
            LocationId::TOTGBigKeyChest,
            LocationId::TOTGGohdanHeartContainer,
        }
};
static const Dungeon ForsakenFortress = {0, GameItem::INVALID, GameItem::INVALID, GameItem::FFDungeonMap, GameItem::FFCompass, {
            LocationId::ForsakenFortressPhantomGanon,
            LocationId::ForsakenFortressChestOutsideUpperJailCell,
            LocationId::ForsakenFortressChestInsideLowerJailCell,
            LocationId::ForsakenFortressBokoblinGuardedChest,
            LocationId::ForsakenFortressChestOnBed,
            LocationId::ForsakenFortressHelmarocKingHeartContainer,
        }, {     // Outside dungeon dependencies
            LocationId::MailboxTingleLetter,
            LocationId::MailboxAryllLetter,
        }
};
static const Dungeon EarthTemple = {3, GameItem::ETSmallKey,   GameItem::ETBigKey,   GameItem::ETDungeonMap,   GameItem::ETCompass, {
            LocationId::EarthTempleWarpPotRoomChest,
            LocationId::EarthTempleBehindDestructableWall,
            LocationId::EarthTempleWarpPotRoomBehindCurtain,
            LocationId::EarthTempleFirstCryptChest,
            LocationId::EarthTempleThreeBlocksRoomChest,
            LocationId::EarthTempleBehindStatuesChest,
            LocationId::EarthTempleSecondCryptCasket,
            LocationId::EarthTempleStalfosMiniBoss,
            LocationId::EarthTempleTingleStatueChest,
            LocationId::EarthTempleFoggyFloormasterRoomEndChest,
            LocationId::EarthTempleKillAllFloormastersChest,
            LocationId::EarthTempleNearHammerButtonBehindCurtain,
            LocationId::EarthTempleThirdCryptChest,
            LocationId::EarthTempleManyMirrorsRoomRightChest,
            LocationId::EarthTempleManyMirrorsRoomLeftChest,
            LocationId::EarthTempleStalfosCryptRoomChest,
            LocationId::EarthTempleBigKeyChest,
            LocationId::EarthTempleJalhallaHeartContainer,
        }, {     // Outside dungeon dependencies
            LocationId::MailboxBaitoLetter,
        }
};
static const Dungeon WindTemple = {2, GameItem::WTSmallKey, GameItem::WTBigKey, GameItem::WTDungeonMap, GameItem::WTCompass, {
            LocationId::WindTempleBetweenDirtPatchesChest,
            LocationId::WindTempleTingleStatueChest,
            LocationId::WindTempleBehindStoneHeadChest,
            LocationId::WindTempleLeftAlcoveChest,
            LocationId::WindTempleBigKeyChest,
            LocationId::WindTempleManyCyclonesRoomChest,
            LocationId::WindTempleHubRoomCenterChest,
            LocationId::WindTempleSpikeWallRoomFirstChest,
            LocationId::WindTempleSpikeWallRoomDestroyFloors,
            LocationId::WindTempleWizzrobeMiniBoss,
            LocationId::WindTempleHubRoomTopChest,
            LocationId::WindTempleBehindArmosChest,
            LocationId::WindTempleKillAllBasmentRoomEnemies,
            LocationId::WindTempleMolgeraHeartContainer,
        }
};
static const Dungeon InvalidDungeon = {0, GameItem::INVALID, GameItem::INVALID, GameItem::INVALID, GameItem::INVALID, {}};

const std::array<DungeonId, 6> getDungeonList()
{
    return {
        DungeonId::DragonRoostCavern,
        DungeonId::ForbiddenWoods,
        DungeonId::TowerOfTheGods,
        DungeonId::ForsakenFortress,
        DungeonId::EarthTemple,
        DungeonId::WindTemple,
    };
}

const Dungeon nameToDungeon(const std::string& name)
{
    static std::unordered_map<std::string, const Dungeon> nameDungeonMap = {
        {"DragonRoostCavern", DragonRoostCavern},
        {"ForbiddenWoods", ForbiddenWoods},
        {"TowerOfTheGods", TowerOfTheGods},
        {"ForsakenFortress", ForsakenFortress},
        {"EarthTemple", EarthTemple},
        {"WindTemple", WindTemple},
    };

    if (nameDungeonMap.count(name) == 0)
    {
        return InvalidDungeon;
    }

    return nameDungeonMap.at(name);
}

std::string dungeonIdToName(const DungeonId& dungeonId)
{
    static std::unordered_map<DungeonId, std::string> dungeonIdNameMap = {
        {DungeonId::DragonRoostCavern, "DragonRoostCavern"},
        {DungeonId::ForbiddenWoods, "ForbiddenWoods"},
        {DungeonId::TowerOfTheGods, "TowerOfTheGods"},
        {DungeonId::ForsakenFortress, "ForsakenFortress"},
        {DungeonId::EarthTemple, "EarthTemple"},
        {DungeonId::WindTemple, "WindTemple"},
    };

    if (dungeonIdNameMap.count(dungeonId) == 0)
    {
        return "INVALID DUNGEON";
    }

    return dungeonIdNameMap.at(dungeonId);
}

DungeonId nameToDungeonId(const std::string& name)
{
    static std::unordered_map<std::string, DungeonId> nameDungeonIdMap = {
        {"DragonRoostCavern", DungeonId::DragonRoostCavern},
        {"ForbiddenWoods", DungeonId::ForbiddenWoods},
        {"TowerOfTheGods", DungeonId::TowerOfTheGods},
        {"ForsakenFortress", DungeonId::ForsakenFortress},
        {"EarthTemple", DungeonId::EarthTemple},
        {"WindTemple", DungeonId::WindTemple},
    };

    if (nameDungeonIdMap.count(name) == 0)
    {
        return DungeonId::INVALID;
    }

    return nameDungeonIdMap.at(name);
}

std::string dungeonIdToFirstRoom(const DungeonId& dungeonId)
{
    static std::unordered_map<DungeonId, std::string> dungeonIdAreaMap = {
        {DungeonId::DragonRoostCavern, "DRCFirstRoom"},
        {DungeonId::ForbiddenWoods, "FWFirstRoom"},
        {DungeonId::TowerOfTheGods, "TOTGEntranceRoom"},
        {DungeonId::ForsakenFortress, "ForsakenFortressInnerCourtyard"},
        {DungeonId::EarthTemple, "ETFirstRoom"},
        {DungeonId::WindTemple, "WTFirstRoom"},
    };

    if (dungeonIdAreaMap.count(dungeonId) == 0)
    {
        return "INVALID";
    }

    return dungeonIdAreaMap.at(dungeonId);
}

const Dungeon dungeonIdToDungeon(const DungeonId& dungeonId)
{
    return nameToDungeon(dungeonIdToName(dungeonId));
}

DungeonId dungeonItemToDungeon(const GameItem& item) {
    static std::unordered_map<GameItem, DungeonId> itemDungeonMap = {
        {GameItem::DRCDungeonMap, DungeonId::DragonRoostCavern},
        {GameItem::DRCCompass, DungeonId::DragonRoostCavern},
        {GameItem::DRCSmallKey, DungeonId::DragonRoostCavern},
        {GameItem::DRCBigKey, DungeonId::DragonRoostCavern},
        {GameItem::FWDungeonMap, DungeonId::ForbiddenWoods},
        {GameItem::FWCompass, DungeonId::ForbiddenWoods},
        {GameItem::FWSmallKey, DungeonId::ForbiddenWoods},
        {GameItem::FWBigKey, DungeonId::ForbiddenWoods},
        {GameItem::TotGDungeonMap, DungeonId::TowerOfTheGods},
        {GameItem::TotGCompass, DungeonId::TowerOfTheGods},
        {GameItem::TotGSmallKey, DungeonId::TowerOfTheGods},
        {GameItem::TotGBigKey, DungeonId::TowerOfTheGods},
        {GameItem::FFDungeonMap, DungeonId::ForsakenFortress},
        {GameItem::FFCompass, DungeonId::ForsakenFortress},
        {GameItem::ETDungeonMap, DungeonId::EarthTemple},
        {GameItem::ETCompass, DungeonId::EarthTemple},
        {GameItem::ETSmallKey, DungeonId::EarthTemple},
        {GameItem::ETBigKey, DungeonId::EarthTemple},
        {GameItem::WTDungeonMap, DungeonId::WindTemple},
        {GameItem::WTCompass, DungeonId::WindTemple},
        {GameItem::WTSmallKey, DungeonId::WindTemple},
        {GameItem::WTBigKey, DungeonId::WindTemple},
    };

    if (itemDungeonMap.count(item) == 0)
    {
        return DungeonId::INVALID;
    }

    return itemDungeonMap.at(item);
}
