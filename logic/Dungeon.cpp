
#include "Dungeon.hpp"
#include <unordered_map>

// The last locationId in each list of dungeon locations is the race mode location
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
}};
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
}};
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
}};
static const Dungeon ForsakenFortress = {0, GameItem::INVALID, GameItem::INVALID, GameItem::FFDungeonMap, GameItem::FFCompass, {
         LocationId::ForsakenFortressPhantomGanon,
         LocationId::ForsakenFortressChestOutsideUpperJailCell,
         LocationId::ForsakenFortressChestInsideLowerJailCell,
         LocationId::ForsakenFortressBokoblinGuardedChest,
         LocationId::ForsakenFortressChestOnBed,
         LocationId::ForsakenFortressHelmarocKingHeartContainer,
}};
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
}};
static const Dungeon WindTemple = {2, GameItem::WTSmallKey, GameItem::WTBigKey, GameItem::WTDungeonMap, GameItem::WTCompass, {
         LocationId::WindTempleBetweenDirtPatchesChest,
         LocationId::WindTempleTingleStatueChest,
         LocationId::WindTempleBehindStoneHeadChest,
         LocationId::WindTempleLeftAlcoveChest,
         LocationId::WindTempleBigKeyChest,
         LocationId::WindTempleManyCyclesRoomChest,
         LocationId::WindTempleHubRoomCenterChest,
         LocationId::WindTempleSpikeWallRoomFirstChest,
         LocationId::WindTempleSpikeWallRoomDestroyFloors,
         LocationId::WindTempleWizzrobeMiniBoss,
         LocationId::WindTempleHubRoomTopChest,
         LocationId::WindTempleBehindArmosChest,
         LocationId::WindTempleKillAllBasmentRoomEnemies,
         LocationId::WindTempleMolgeraHeartContainer,
}};
static const Dungeon InvalidDungeon = {0, GameItem::INVALID, GameItem::INVALID, GameItem::INVALID, GameItem::INVALID, {}};

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

const Dungeon dungeonIdToDungeon(const DungeonId& dungeonId)
{
    return nameToDungeon(dungeonIdToName(dungeonId));
}
