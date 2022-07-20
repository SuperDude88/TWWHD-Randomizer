
#pragma once

#include <memory>
#include <string>
#include <vector>
#include <unordered_set>
#include "GameItem.hpp"
#include "Requirements.hpp"
#include "../server/command/WriteLocations.hpp"

enum struct LocationId : uint32_t
{
    INVALID = 0,
    OutsetUnderLinksHouseChest,
    OutsetMesasHouseChest,
    OutsetOrca10KnightsCrest,
    OutsetOrca500Hits,
    OutsetGreatFairy,
    OutsetJabunsCaveChest,
    OutsetDigBlackSoil,
    OutsetSavageFloor30Chest,
    OutsetSavageFloor50Chest,
    WindfallTingleFirstGift,
    WindfallTingleSecondGift,
    WindfallJailMazeChest,
    WindfallPotionShop15Green,
    WindfallPotionShop15Blue,
    WindfallIvanCatchKillerBees,
    WindfallMrsMarieCatchKillerBees,
    WindfallMrsMarie1JoyPendant,
    WindfallMrsMarie21JoyPendant,
    WindfallMrsMarie40JoyPendant,
    WindfallLenzoHouseLeftChest,
    WindfallLenzoHouseRightChest,
    WindfallLenzoBecomeAssistant,
    WindfallLenzoBringFirefly,
    WindfallHouseOfWealthChest,
    WindfallMaggiesFatherGive20SkullNecklace,
    WindfallMaggieFreeItem,
    WindfallMaggieDeliveryReward,
    WindfallCafePostmanDelivery,
    WindfallKreebLightLighthouse,
    WindfallTransparentChest,
    WindfallTottTeachRhythm,
    WindfallPirateShipChest,
    WindfallAuction5Rupee,
    WindfallAuction40Rupee,
    WindfallAuction60Rupee,
    WindfallAuction80Rupee,
    WindfallAuction100Rupee,
    WindfallZunariExoticFlower,
    WindfallSamDecorateFlower,
    WindfallKaneDecorateShopGuru,
    WindfallKaneDecoratePostmanStatue,
    WindfallKaneDecorateSixFlags,
    WindfallKaneDecorateSixIdols,
    WindfallMilaCatchThief,
    WindfallBattleSquidFirstPrize,
    WindfallBattleSquidSecondPrize,
    WindfallBattleSquidUnder20Prize,
    WindfallDampaPigMinigame,
    WindfallPompieVeraPictoSecretMeeting,
    WindfallKamoPictoFullMoon,
    WindfallMinecoPictoMissWindfall,
    WindfallAntonPictoLinda,
    DragonRoostIslandWindShrine,
    DragonRoostIslandHoskit20GoldenFeathers,
    DragonRoostIslandBoulderChest,
    DragonRoostIslandLeafPlatforms,
    DragonRoostIslandBaitoMailgame,
    DragonRoostIslandCaveChest,
    DragonRoostCavernFirstRoomChest,
    DragonRoostCavernWaterJugAlcove,
    DragonRoostCavernBoardedUpChest,
    DragonRoostCavernSwingAcrossLavaChest,
    DragonRoostCavernRatRoomChest,
    DragonRoostCavernRatRoomBoardedUpChest,
    DragonRoostCavernBirdsNest,
    DragonRoostCavernDarkRoomChest,
    DragonRoostCavernHubRoomTingleChest,
    DragonRoostCavernPotRoomChest,
    DragonRoostCavernMiniBoss,
    DragonRoostCavernUnderRopeBridge,
    DragonRoostCavernTingleStatueChest,
    DragonRoostCavernBigKeyChest,
    DragonRoostCavernBossStairsRightChest,
    DragonRoostCavernBossStairsLeftChest,
    DragonRoostCavernGohmaHeartContainer,
    ForestHavenOnTreeBranch,
    ForestHavenSmallIslandChest,
    ForbiddenWoodsFirstRoomChest,
    ForbiddenWoodsInsideHollowTree,
    ForbiddenWoodsBokoBabaClimb,
    ForbiddenWoodsHoleInTreeChest,
    ForbiddenWoodsMorthPit,
    ForbiddenWoodsVineMazeLeftChest,
    ForbiddenWoodsVineMazeRightChest,
    ForbiddenWoodsTallRoomChest,
    ForbiddenWoodsMothulaMiniBoss,
    ForbiddenWoodsByHangingSeedsChest,
    ForbiddenWoodsChestAcrossHangingFlower,
    ForbiddenWoodsTingleStatueChest,
    ForbiddenWoodsLockedTreeTrunkChest,
    ForbiddenWoodsBigKeyChest,
    ForbiddenWoodsDoubleMothulaRoomChest,
    ForbiddenWoodsKalleDemosHeartContainer,
    GreatfishHiddenChest,
    TOTGChestBehindBombableWall,
    TOTGHopAcrossFloatingBoxesChest,
    TOTGLightTwoTorches,
    TOTGSkullRoomChest,
    TOTGShootEyeAboveSkulls,
    TOTGTingleStatueChest,
    TOTGFirstArmosKnightsChest,
    TOTGStoneTablet,
    TOTGDarknutMiniBoss,
    TOTGSecondArmostKnightsChest,
    TOTGFloatingPlatformsRoomLowerChest,
    TOTGFloatingPlatformsRoomUpperChest,
    TOTGBigKeyChest,
    TOTGGohdanHeartContainer,
    HyruleCastleSwordChamberChest,
    ForsakenFortressPhantomGanon,
    ForsakenFortressChestOutsideUpperJailCell,
    ForsakenFortressChestInsideLowerJailCell,
    ForsakenFortressBokoblinGuardedChest,
    ForsakenFortressChestOnBed,
    ForsakenFortressHelmarocKingHeartContainer,
    MotherAndChildInsideMotherIsleChest,
    FireMountainInteriorChest,
    FireMountainLookoutPlatformChest,
    FireMountainLookoutPlatformDestroyCannons,
    FireMountainBigOcto,
    IceRingFrozenChest,
    IceRingInteriorChest,
    IceRingInnerCaveChest,
    HeadstoneTopOfIsland,
    HeadstoneIslandSubmarineChest,
    EarthTempleWarpPotRoomChest,
    EarthTempleBehindDestructableWall,
    EarthTempleWarpPotRoomBehindCurtain,
    EarthTempleFirstCryptChest,
    EarthTempleThreeBlocksRoomChest,
    EarthTempleBehindStatuesChest,
    EarthTempleSecondCryptCasket,
    EarthTempleStalfosMiniBoss,
    EarthTempleTingleStatueChest,
    EarthTempleFoggyFloormasterRoomEndChest,
    EarthTempleKillAllFloormastersChest,
    EarthTempleNearHammerButtonBehindCurtain,
    EarthTempleThirdCryptChest,
    EarthTempleManyMirrorsRoomRightChest,
    EarthTempleManyMirrorsRoomLeftChest,
    EarthTempleStalfosCryptRoomChest,
    EarthTempleBigKeyChest,
    EarthTempleJalhallaHeartContainer,
    WindTempleBetweenDirtPatchesChest,
    WindTempleTingleStatueChest,
    WindTempleBehindStoneHeadChest,
    WindTempleLeftAlcoveChest,
    WindTempleBigKeyChest,
    WindTempleManyCyclonesRoomChest,
    WindTempleHubRoomCenterChest,
    WindTempleSpikeWallRoomFirstChest,
    WindTempleSpikeWallRoomDestroyFloors,
    WindTempleWizzrobeMiniBoss,
    WindTempleHubRoomTopChest,
    WindTempleBehindArmosChest,
    WindTempleKillAllBasmentRoomEnemies,
    WindTempleMolgeraHeartContainer,
    GanonsTowerMazeChest,
    DefeatGanondorf,
    MailboxHoskitGirlfriendLetter,
    MailboxBaitoMotherLetter,
    MailboxBaitoLetter,
    MailboxKomaliFatherLetter,
    MailboxBeedleBombAdLetter,
    MailboxRockSpireShopAdLetter,
    MailboxSilverMembershipRewardLetter,
    MailboxGoldMembershipRewardLetter,
    MailboxOrcaLetter,
    MailboxGrandmaLetter,
    MailboxAryllLetter,
    MailboxTingleLetter,
    GreatSeaBeedleShop20Rupee,
    GreatSeaSalvageCorpGift,
    GreatSeaCyclos,
    GreatSeaGoronTradingReward,
    GreatSeaWitheredTrees,
    GhostShipChest,
    PrivateOasisTopOfWaterfallChest,
    PrivateOasisCabanaLabyrinthLowerFloorChest,
    PrivateOasisCabanaLabyrinthUpperFloorChest,
    PrivateOasisBigOcto,
    SpectacleBarrelShootingFirstPrize,
    SpectacleBarrelShootingSecondPrize,
    NeedleRockChest,
    NeedleRockCaveChest,
    NeedleRockGoldenGunboat,
    AngularIslesPeak,
    AngularIslesCaveChest,
    BoatingCourseRaftChest,
    BoatingCourseCaveChest,
    StoneWatcherCaveChest,
    StoneWatcherLookoutPlatformChest,
    StoneWatcherLookoutPlatformDestroyCannons,
    IsletOfSteelInteriorChest,
    IsletOfSteelLookoutPlatformDefeatEnemies,
    OverlookCaveChest,
    BirdsPeakRockCaveChest,
    PawprintChuchuCaveChest,
    PawprintChuchuCaveBehindLeftBoulder,
    PawprintChuchuCaveBehindRightBoulder,
    PawprintChuchuCaveScaleWall,
    PawprintWizzrobeCaveChest,
    PawprintLookoutPlatformDefeatEnemies,
    ThornedFairyGreatFairy,
    ThornedFairyNortheasternLookoutPlatformDestroyCannons,
    ThornedFairySouthwesternLookoutPlatformDefeatEnemies,
    EasternFairyGreatFairy,
    EasternFairyLookoutPlatformDefeatCannonsAndEnemies,
    WesternFairyGreatFairy,
    WesternFairyLookoutPlatformChest,
    SouthernFairyGreatFairy,
    SouthernFairyLookoutPlatformDestroyNorthwestCannons,
    SouthernFairyLookoutPlatformDestroySoutheastCannons,
    NorthernFairyIslandGreatFairy,
    NorthernFairyIslandSubmarineChest,
    TingleIslandAnkleAllStatuesReward,
    TingleIslandBigOcto,
    DiamondSteppeWarpMazeFirstChest,
    DiamondSteppeWarpMazeSecondChest,
    DiamondSteppeBigOcto,
    BombIslandCaveChest,
    BombIslandLookoutPlatformDefeatEnemies,
    BombIslandSubmarineChest,
    RockSpireCaveChest,
    RockSpireBeedle500RupeeItem,
    RockSpireBeedle950RupeeItem,
    RockSpireBeedle900RupeeItem,
    RockSpireWesternLookoutPlatformDestroyCannons,
    RockSpireEasternLookoutPlatformDestroyCannons,
    RockSpireCenterLookoutPlatformChest,
    RockSpireSoutheastGunboat,
    SharkIslandCaveChest,
    CliffPlateauCaveChest,
    CliffPlateauHighestIsleChest,
    CliffPlateauLookoutPlatformChest,
    CrescentMoonChest,
    CrescentMoonSubmarineChest,
    HorseshoePlayGolf,
    HorseshoeCaveChest,
    HorseshoeNorthwesternLookoutPlatformChest,
    HorseshoeSoutheasternLookoutPlatformChest,
    FlightControlBirdManFirstPrize,
    FlightControlSubmarineChest,
    StarIslandCaveChest,
    StarIslandLookoutPlatformChest,
    StarBeltLookoutPlatformChest,
    FiveStarLookoutPlatformDestroyCannons,
    FiveStarRaftChest,
    FiveStarSubmarineChest,
    SevenStarCenterLookoutPlatformChest,
    SevenStarNorthernLookoutPlatformChest,
    SevenStarSouthernLookoutPlatformDefeatWizzrobes,
    SevenStarBigOcto,
    CyclopsReefDestroyCannonsAndGunboats,
    CyclopsReefLookoutPlatformDefeatEnemies,
    TwoEyeReefDestroyCannonsAndGunboats,
    TwoEyeReefLookoutPlatformChest,
    TwoEyeReefBigOctoGreatFairy,
    ThreeEyeReefDestroyCannonsAndGunboats,
    FourEyeReefDestroyCannonsAndGunboats,
    FiveEyeReefDestroyCannonsAndGunboats,
    FiveEyeReefLookoutPlatformChest,
    SixEyeReefDestroyCannonsAndGunboats,
    SixEyeReefLookoutPlatformDestroyCannons,
    SixEyeReefSubmarineChest,
    // Don't change the ordering of the sunken treasure locations
    ForsakenFortressSunkenTreasure,
    StarIslandSunkenTreasure,
    NorthernFairySunkenTreasure,
    GaleIsleSunkenTreasure,
    CrescentMoonSunkenTreasure,
    SevenStarSunkenTreasure,
    OverlookSunkenTreasure,
    FourEyeReefSunkenTreasure,
    MotherAndChildSunkenTreasure,
    SpectacleSunkenTreasure,
    WindfallSunkenTreasure,
    PawprintSunkenTreasure,
    DragonRoostIslandSunkenTreasure,
    FlightControlSunkenTreasure,
    WesternFairySunkenTreasure,
    RockSpireSunkenTreasure,
    TingleIslandSunkenTreasure,
    NorthernTriangleSunkenTreasure,
    EasternFairySunkenTreasure,
    FireMountainSunkenTreasure,
    StarBeltSunkenTreasure,
    ThreeEyeReefSunkenTreasure,
    GreatfishSunkenTreasure,
    CyclopsReefSunkenTreasure,
    SixEyeReefSunkenTreasure,
    TOTGSunkenTreasure,
    EasternTriangleSunkenTreasure,
    ThornedFairySunkenTreasure,
    NeedleRockSunkenTreasure,
    IsletOfSteelSunkenTreasure,
    StoneWatcherSunkenTreasure,
    SouthernTriangleSunkenTreasure,
    PrivateOasisSunkenTreasure,
    BombIslandSunkenTreasure,
    BirdsPeakSunkenTreasure,
    DiamondSteppeSunkenTreasure,
    FiveEyeReefSunkenTreasure,
    SharkIslandSunkenTreasure,
    SouthernFairySunkenTreasure,
    IceRingSunkenTreasure,
    ForestHavenSunkenTreasure,
    CliffPlateauSunkenTreasure,
    HorseshoeSunkenTreasure,
    OutsetSunkenTreasure,
    HeadstoneSunkenTreasure,
    TwoEyeReefSunkenTreasure,
    AngularSunkenTreasure,
    BoatingCourseSunkenTreasure,
    FiveStarSunkenTreasure,
    COUNT
};

#define LOCATION_COUNT static_cast<std::underlying_type_t<LocationId>>(LocationId::COUNT)

// move this and mod type into location entry or own file?
enum struct LocationCategory
{
    INVALID = 0,
    Misc,
    Dungeon,
    GreatFairy,
    IslandPuzzle,
    SpoilsTrading,
    Mail,
    SavageLabyrinth,
    FreeGift,
    Minigame,
    BattleSquid,
    TingleChest,
    PuzzleSecretCave,
    CombatSecretCave,
    Platform,
    Raft,
    EyeReefChests,
    BigOcto,
    Submarine,
    Gunboat,
    LongSideQuest,
    ShortSideQuest,
    ExpensivePurchase,
    SunkenTreasure,
    Obscure, // <-- the good stuff :)
    Junk,
    Other
};

enum struct LocationModificationType
{
    INVALID = 0,
    Chest,
    Actor,
    SCOB,
    Event,
    RPX,
    Custom_Symbol,
    Boss,
    DoNothing
};

LocationId nameToLocationId(const std::string& name);
std::string locationIdToName(LocationId location);
void storeNewLocationPrettyName(const LocationId& locationId, const std::string& prettyName);
LocationId prettyNameToLocationId(const std::string& prettyName);
std::string locationIdToPrettyName(const LocationId& locationId);
LocationCategory nameToLocationCategory(const std::string& name);
LocationModificationType nameToModificationType(const std::string& name);
uint32_t locationIdAsIndex(LocationId loc);
LocationId indexAsLocationId(uint32_t index);

struct Location
{
    LocationId locationId;
    std::unordered_set<LocationCategory> categories;
    bool progression;
    bool plandomized;
    Item originalItem;
    Item currentItem;
    std::unique_ptr<LocationModification> method;
    int worldId = -1;

    // Variables used for the searching algorithm
    bool hasBeenFound = false;

    Location() :
        locationId(LocationId::INVALID),
        categories({LocationCategory::INVALID}),
        progression(false),
        originalItem(GameItem::INVALID, -1),
        currentItem(GameItem::INVALID, -1),
        worldId(-1),
        hasBeenFound(false)
    {
        method = std::make_unique<LocationModification>();
    }
    ~Location() = default;
    Location(const Location& loc) = delete;
    Location& operator=(const Location&) = delete;
    Location(Location&&) = default;
    Location& operator=(Location&&) = default;
    bool operator<(const Location& rhs) const;
};



std::string locationName(const Location* location);
