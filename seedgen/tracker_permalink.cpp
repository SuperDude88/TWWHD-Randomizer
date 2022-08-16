#include "tracker_permalink.hpp"
#include "packed_bits.hpp"
#include "../libs/base64pp.hpp"

#include <array>

// The ordering of these arrays is meant to match that of the wind waker randomizer
// tracker mantained by wooferzfg. This includes some dummy options that Wind Waker
// HD Randomizer doesn't have for cpmpatibility
static const std::array<Option, 46> TRACKER_PERMALINK_OPTIONS {
    // Progression
    Option::ProgressDungeons,
    Option::ProgressGreatFairies,
    Option::ProgressPuzzleCaves,
    Option::ProgressCombatCaves,
    Option::ProgressShortSidequests,
    Option::ProgressLongSidequests,
    Option::ProgressSpoilsTrading,
    Option::ProgressMinigames,
    Option::ProgressFreeGifts,
    Option::ProgressMail,
    Option::ProgressPlatformsRafts,
    Option::ProgressSubmarines,
    Option::ProgressEyeReefs,
    Option::ProgressOctosGunboats,
    Option::ProgressTriforceCharts,
    Option::ProgressTreasureCharts,
    Option::ProgressExpPurchases,
    Option::ProgressMisc,
    Option::ProgressTingleChests,
    Option::ProgressBattlesquid,
    Option::ProgressSavageLabyrinth,
    Option::ProgressIslandPuzzles,

    Option::Keylunacy,
    Option::RandomizeEntrances,
    Option::RandomCharts,
    Option::RandomStartIsland,

    Option::SwiftSail,
    Option::InstantText,
    Option::RevealSeaChart,
    Option::NumShards,
    Option::AddShortcutWarps,
    Option::NoSpoilerLog,
    Option::SwordMode,
    Option::SkipRefights,
    Option::RaceMode,
    Option::NumRaceModeDungeons,
    Option::RandomizeMusic,
    Option::DisableTingleChestsWithTingleBombs,
    Option::RandomizeEnemyPalettes,

    Option::StartingGear,
    Option::StartingHP,
    Option::StartingHC,
    Option::RemoveMusic,
    Option::RandomizeEnemies,
    Option::WindWakerHD,
};

static const std::array<GameItem, 29> TRACKER_REGULAR_ITEMS = {
    GameItem::BaitBag,
    GameItem::Bombs,
    GameItem::Boomerang,
    GameItem::CabanaDeed,
    GameItem::CommandMelody,
    GameItem::DekuLeaf,
    GameItem::DeliveryBag,
    GameItem::DinsPearl,
    GameItem::EarthGodsLyric,
    GameItem::EmptyBottle,
    GameItem::FaroresPearl,
    GameItem::GhostShipChart,
    GameItem::GrapplingHook,
    GameItem::HerosCharm,
    GameItem::Hookshot,
    GameItem::HurricaneSpin,
    GameItem::IronBoots,
    GameItem::MaggiesLetter,
    GameItem::MagicArmor,
    GameItem::ProgressiveMagicMeter,
    GameItem::MoblinsLetter,
    GameItem::NayrusPearl,
    GameItem::NoteToMom,
    GameItem::PowerBracelets,
    GameItem::SkullHammer,
    GameItem::SpoilsBag,
    GameItem::Telescope,
    GameItem::TingleBottle,
    GameItem::WindGodsAria,
};

static const std::array<GameItem, 7> TRACKER_PROGRESSIVE_ITEMS = {
    GameItem::ProgressiveBombBag,
    GameItem::ProgressiveBow,
    GameItem::ProgressivePictoBox,
    GameItem::ProgressiveQuiver,
    GameItem::ProgressiveShield,
    GameItem::ProgressiveSword,
    GameItem::ProgressiveWallet,
};

std::string create_tracker_permalink(const Settings& settings, const std::string& seed) {

    std::string permalink = "";
    permalink += "1.9.0";
    permalink += '\0';
    permalink += "Seed";
    permalink += '\0';

    // Pack the settings up
    auto bitsWriter = PackedBitsWriter();
    for(const auto& option : TRACKER_PERMALINK_OPTIONS) {

        // Handle starting gear
        if (option == Option::StartingGear) {
            std::multiset<GameItem> startingGear  (settings.starting_gear.begin(), settings.starting_gear.end());
            startingGear.clear();
            startingGear.insert(GameItem::ProgressiveShield);
            for (size_t i = 0; i < TRACKER_REGULAR_ITEMS.size(); i++)
            {
                size_t bit = startingGear.contains(TRACKER_REGULAR_ITEMS[i]);
                bitsWriter.write(bit, 1);
            }
            for (auto& item : TRACKER_PROGRESSIVE_ITEMS)
            {
                bitsWriter.write(startingGear.count(item), 2);
            }
        }
        // ComboBox Options
        else if (option == Option::SwordMode || option == Option::NumRaceModeDungeons || option == Option::NumShards)
        {
            bitsWriter.write(getSetting(settings, option), 8);
        }
        // 3-bit SpinBox options
        else if (option == Option::PathHints || option == Option::BarrenHints || option == Option::LocationHints || option == Option::ItemHints || option == Option::StartingHC)
        {
            bitsWriter.write(getSetting(settings, option), 3);
        }
        // 6-bit SpinBox options
        else if (option == Option::StartingHP)
        {
            bitsWriter.write(getSetting(settings, option), 6);
        }
        else if (option == Option::SwiftSail  || option == Option::DisableTingleChestsWithTingleBombs || option == Option::WindWakerHD)
        {
            size_t value = 1;
            bitsWriter.write(value, 1);
        }
        else if (option == Option::RandomizeEntrances)
        {
            size_t value = 0;
            if (settings.randomize_dungeon_entrances && !settings.randomize_cave_entrances && !settings.mix_entrance_pools)
            {
                value = 1;
            }
            else if (!settings.randomize_dungeon_entrances && settings.randomize_cave_entrances && !settings.mix_entrance_pools)
            {
                value = 2;
            }
            else if (settings.randomize_dungeon_entrances && settings.randomize_cave_entrances && !settings.mix_entrance_pools)
            {
                value = 3;
            }
            else if (settings.randomize_dungeon_entrances && settings.randomize_cave_entrances && settings.mix_entrance_pools)
            {
                value = 4;
            }
            bitsWriter.write(value, 8);
        }
        else if (option == Option::RandomizeMusic || option == Option::RandomizeEnemies || option == Option::RandomizeEnemyPalettes || option == Option::InvertCameraXAxis)
        {
            size_t value = 0;
            bitsWriter.write(value, 1);
        }
        // 1-bit Checkbox options
        else
        {
            bitsWriter.write(getSetting(settings, option), 1);
        }
    }

    // Add the packed bits to the permalink
    bitsWriter.flush();
    for (auto& byte : bitsWriter.bytes)
    {
        permalink += byte;
    }

    return b64_encode(permalink);
}
