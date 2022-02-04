#include "ItemPool.hpp"
#include "PoolFunctions.hpp"
#include "Random.hpp"
#include "Debug.hpp"
#include "Dungeon.hpp"

static const GameItemPool alwaysItems = {

    // Potentially progress items
    GameItem::WindWaker,
    GameItem::SpoilsBag,
    GameItem::GrapplingHook,
    GameItem::PowerBracelets,
    GameItem::IronBoots,
    GameItem::BaitBag,
    GameItem::Boomerang,
    GameItem::Hookshot,
    GameItem::DeliveryBag,
    GameItem::Bombs,
    GameItem::SkullHammer,
    GameItem::DekuLeaf,

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

    GameItem::WindsRequiem,
    GameItem::SongOfPassing,
    GameItem::BalladOfGales,
    GameItem::CommandMelody,
    GameItem::EarthGodsLyric,
    GameItem::WindGodsAria,

    GameItem::BoatsSail,

    GameItem::NoteToMom,
    GameItem::MaggiesLetter,
    GameItem::MoblinsLetter,
    GameItem::CabanaDeed,

    GameItem::DragonTingleStatue,
    GameItem::ForbiddenTingleStatue,
    GameItem::GoddessTingleStatue,
    GameItem::EarthTingleStatue,
    GameItem::WindTingleStatue,

    GameItem::ProgressiveBombBag,
    GameItem::ProgressiveBombBag,
    GameItem::ProgressiveQuiver,
    GameItem::ProgressiveQuiver,
    GameItem::MagicMeterUpgrade,

    GameItem::ProgressiveShield,
    GameItem::ProgressiveShield,
    GameItem::ProgressiveBow,
    GameItem::ProgressiveBow,
    GameItem::ProgressiveBow,
    GameItem::ProgressiveWallet,
    GameItem::ProgressiveWallet,
    GameItem::ProgressivePictoBox,
    GameItem::ProgressivePictoBox,
    GameItem::EmptyBottle,
    GameItem::EmptyBottle,
    GameItem::EmptyBottle,
    GameItem::EmptyBottle,

    GameItem::GhostShipChart,

    GameItem::TreasureChart1,
    GameItem::TreasureChart2,
    GameItem::TreasureChart3,
    GameItem::TreasureChart4,
    GameItem::TreasureChart5,
    GameItem::TreasureChart6,
    GameItem::TreasureChart7,
    GameItem::TreasureChart8,
    GameItem::TreasureChart9,
    GameItem::TreasureChart10,
    GameItem::TreasureChart11,
    GameItem::TreasureChart12,
    GameItem::TreasureChart13,
    GameItem::TreasureChart14,
    GameItem::TreasureChart15,
    GameItem::TreasureChart16,
    GameItem::TreasureChart17,
    GameItem::TreasureChart18,
    GameItem::TreasureChart19,
    GameItem::TreasureChart20,
    GameItem::TreasureChart21,
    GameItem::TreasureChart22,
    GameItem::TreasureChart23,
    GameItem::TreasureChart24,
    GameItem::TreasureChart25,
    GameItem::TreasureChart26,
    GameItem::TreasureChart27,
    GameItem::TreasureChart28,
    GameItem::TreasureChart29,
    GameItem::TreasureChart30,
    GameItem::TreasureChart31,
    GameItem::TreasureChart32,
    GameItem::TreasureChart33,
    GameItem::TreasureChart34,
    GameItem::TreasureChart35,
    GameItem::TreasureChart36,
    GameItem::TreasureChart37,
    GameItem::TreasureChart38,
    GameItem::TreasureChart39,
    GameItem::TreasureChart40,
    GameItem::TreasureChart41,
    GameItem::TreasureChart42,
    GameItem::TreasureChart43,
    GameItem::TreasureChart44,
    GameItem::TreasureChart45,
    GameItem::TreasureChart46,
    GameItem::TriforceChart1,
    GameItem::TriforceChart2,
    GameItem::TriforceChart3,

    // Junk non-consumable items
    GameItem::Telescope,
    GameItem::MagicArmor,
    GameItem::HerosCharm,
    GameItem::FillUpCoupon,

    GameItem::HurricaneSpin,

    GameItem::SubmarineChart,
    GameItem::BeedlesChart,
    GameItem::PlatformChart,
    GameItem::LightRingChart,
    GameItem::SecretCaveChart,
    GameItem::GreatFairyChart,
    GameItem::OctoChart,
    GameItem::TinglesChart,

    // Junk consumable items
    GameItem::GreenRupee,   // 1 Green Rupee
    GameItem::BlueRupee,    // 2 Blue Rupees
    GameItem::BlueRupee,
    GameItem::YellowRupee,  // 3 Yellow Rupees
    GameItem::YellowRupee,
    GameItem::YellowRupee,
    GameItem::RedRupee,     // 5 Red Rupees
    GameItem::RedRupee,
    GameItem::RedRupee,
    GameItem::RedRupee,
    GameItem::RedRupee,
    GameItem::PurpleRupee,  // 10 Purple Rupees
    GameItem::PurpleRupee,
    GameItem::PurpleRupee,
    GameItem::PurpleRupee,
    GameItem::PurpleRupee,
    GameItem::PurpleRupee,
    GameItem::PurpleRupee,
    GameItem::PurpleRupee,
    GameItem::PurpleRupee,
    GameItem::PurpleRupee,
    GameItem::OrangeRupee,  // 15 Orange Rupees
    GameItem::OrangeRupee,
    GameItem::OrangeRupee,
    GameItem::OrangeRupee,
    GameItem::OrangeRupee,
    GameItem::OrangeRupee,
    GameItem::OrangeRupee,
    GameItem::OrangeRupee,
    GameItem::OrangeRupee,
    GameItem::OrangeRupee,
    GameItem::OrangeRupee,
    GameItem::OrangeRupee,
    GameItem::OrangeRupee,
    GameItem::OrangeRupee,
    GameItem::OrangeRupee,
    GameItem::SilverRupee,  // 15 Silver Rupees
    GameItem::SilverRupee,
    GameItem::SilverRupee,
    GameItem::SilverRupee,
    GameItem::SilverRupee,
    GameItem::SilverRupee,
    GameItem::SilverRupee,
    GameItem::SilverRupee,
    GameItem::SilverRupee,
    GameItem::SilverRupee,
    GameItem::SilverRupee,
    GameItem::SilverRupee,
    GameItem::SilverRupee,
    GameItem::SilverRupee,
    GameItem::SilverRupee,
    GameItem::RainbowRupee, // 1 Rainbow Rupee
    GameItem::JoyPendant,   // 9 Joy Pendants
    GameItem::JoyPendant,
    GameItem::JoyPendant,
    GameItem::JoyPendant,
    GameItem::JoyPendant,
    GameItem::JoyPendant,
    GameItem::JoyPendant,
    GameItem::JoyPendant,
    GameItem::JoyPendant,
    GameItem::SkullNecklace,  // 9 Skull Necklaces
    GameItem::SkullNecklace,
    GameItem::SkullNecklace,
    GameItem::SkullNecklace,
    GameItem::SkullNecklace,
    GameItem::SkullNecklace,
    GameItem::SkullNecklace,
    GameItem::SkullNecklace,
    GameItem::SkullNecklace,
    GameItem::BokoBabaSeed,   // 1 Boko Baba Seed
    GameItem::GoldenFeather,  // 9 Golden Feathers
    GameItem::GoldenFeather,
    GameItem::GoldenFeather,
    GameItem::GoldenFeather,
    GameItem::GoldenFeather,
    GameItem::GoldenFeather,
    GameItem::GoldenFeather,
    GameItem::GoldenFeather,
    GameItem::GoldenFeather,
    GameItem::KnightsCrest,   // 3 Knight's Crests
    GameItem::KnightsCrest,
    GameItem::KnightsCrest,
    GameItem::RedChuJelly,    // 1 Red Chu Jelly
    GameItem::GreenChuJelly,  // 1 Green Chu Jelly
    GameItem::AllPurposeBait, // 1 All-Purpose Bait
    GameItem::HyoiPear,       // 4 Hyoi Pears
    GameItem::HyoiPear,
    GameItem::HyoiPear,
    GameItem::HyoiPear,
};

// The distribution of elements in this pool is to give some items
// a higher chance of being randomly selected than others
static const GameItemPool junkPool = {
    GameItem::YellowRupee,  // 3 Yellow Rupees
    GameItem::YellowRupee,
    GameItem::YellowRupee,
    GameItem::RedRupee,     // 7 Red Rupees
    GameItem::RedRupee,
    GameItem::RedRupee,
    GameItem::RedRupee,
    GameItem::RedRupee,
    GameItem::RedRupee,
    GameItem::RedRupee,
    GameItem::PurpleRupee,  // 10 Purple Rupees
    GameItem::PurpleRupee,
    GameItem::PurpleRupee,
    GameItem::PurpleRupee,
    GameItem::PurpleRupee,
    GameItem::PurpleRupee,
    GameItem::PurpleRupee,
    GameItem::PurpleRupee,
    GameItem::PurpleRupee,
    GameItem::PurpleRupee,
    GameItem::OrangeRupee,  // 15 Orange Rupees
    GameItem::OrangeRupee,
    GameItem::OrangeRupee,
    GameItem::OrangeRupee,
    GameItem::OrangeRupee,
    GameItem::OrangeRupee,
    GameItem::OrangeRupee,
    GameItem::OrangeRupee,
    GameItem::OrangeRupee,
    GameItem::OrangeRupee,
    GameItem::OrangeRupee,
    GameItem::OrangeRupee,
    GameItem::OrangeRupee,
    GameItem::OrangeRupee,
    GameItem::OrangeRupee,
    GameItem::JoyPendant,   // 3 Joy Pendants
    GameItem::JoyPendant,
    GameItem::JoyPendant,
};

GameItem getRandomJunk()
{
    return RandomElement(junkPool);
}

GameItemPool generateGameItemPool(const Settings& settings)
{
    GameItemPool completeItemPool = {};

    // Add items which will always be in the item pool
    addElementsToPool(completeItemPool, alwaysItems);

    // Add dungeon items
    const static std::array<std::string, 6> dungeonNames = {
        "DragonRoostCavern",
        "ForbiddenWoods",
        "TowerOfTheGods",
        "ForsakenFortress",
        "EarthTemple",
        "WindTemple",
    };

    for (auto& dungeonName : dungeonNames)
    {
        const auto& dungeon = nameToDungeon(dungeonName);
        if (dungeon.smallKey != GameItem::INVALID)
        {
            addElementToPool(completeItemPool, dungeon.smallKey, dungeon.keyCount);
        }
        if (dungeon.bigKey != GameItem::INVALID)
        {
            addElementToPool(completeItemPool, dungeon.bigKey);
        }
        addElementToPool(completeItemPool, dungeon.map);
        addElementToPool(completeItemPool, dungeon.compass);
    }


    // Determine how many swords should be in the pool
    if (settings.sword_mode != SwordMode::NoSword)
    {
        addElementToPool(completeItemPool, GameItem::ProgressiveSword, 4);
    }

    return completeItemPool;
}

GameItemPool generateStartingGameItemPool(const Settings& settings)
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
    if (settings.sword_mode == SwordMode::StartWithSword)
    {
        startingItems.push_back(GameItem::ProgressiveSword);
    }

    for (auto& item : settings.starting_gear)
    {
        startingItems.push_back(item);
    }

    return startingItems;
}

void logItemPool(const std::string& poolName, const ItemPool& itemPool)
{
    debugLog(poolName + ":");
    for (auto& item : itemPool) {
        debugLog("\t" + item.getName());
    }
}
