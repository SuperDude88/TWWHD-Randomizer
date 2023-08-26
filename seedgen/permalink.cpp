#include "permalink.hpp"

#include <vector>
#include <set>
#include <iostream>

#include <version.hpp>
#include <libs/base64pp.hpp>
#include <seedgen/packed_bits.hpp>

#define BYTES_EXIST_CHECK(value) if (value == 0xFFFFFFFF) return PermalinkError::BAD_PERMALINK;

static const std::vector<GameItem> REGULAR_ITEMS = {
    GameItem::BaitBag,
    GameItem::BalladOfGales,
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
    GameItem::MoblinsLetter,
    GameItem::NayrusPearl,
    GameItem::NoteToMom,
    GameItem::PowerBracelets,
    GameItem::SkullHammer,
    GameItem::SongOfPassing,
    GameItem::SpoilsBag,
    GameItem::Telescope,
    GameItem::TingleBottle,
    GameItem::WindGodsAria,
    GameItem::DRCBigKey,
    GameItem::DRCCompass,
    GameItem::DRCDungeonMap,
    GameItem::FWBigKey,
    GameItem::FWCompass,
    GameItem::FWDungeonMap,
    GameItem::TotGBigKey,
    GameItem::TotGCompass,
    GameItem::TotGDungeonMap,
    GameItem::ETBigKey,
    GameItem::ETCompass,
    GameItem::ETDungeonMap,
    GameItem::WTBigKey,
    GameItem::WTCompass,
    GameItem::WTDungeonMap,
    GameItem::FFCompass,
    GameItem::FFDungeonMap,
    GameItem::DragonTingleStatue,
    GameItem::ForbiddenTingleStatue,
    GameItem::GoddessTingleStatue,
    GameItem::EarthTingleStatue,
    GameItem::WindTingleStatue,
};

static const std::vector<GameItem> PROGRESSIVE_ITEMS = {
    GameItem::ProgressiveBombBag,
    GameItem::ProgressiveBow,
    GameItem::ProgressiveMagicMeter,
    GameItem::ProgressivePictoBox,
    GameItem::ProgressiveQuiver,
    GameItem::ProgressiveShield,
    GameItem::ProgressiveSword,
    GameItem::ProgressiveSail,
    GameItem::ProgressiveWallet,
    GameItem::DRCSmallKey,
    GameItem::FWSmallKey,
    GameItem::TotGSmallKey,
    GameItem::ETSmallKey,
    GameItem::WTSmallKey,
};
// These are options that should affect seed generation even with the same seed
static const std::vector<Option> PERMALINK_OPTIONS {
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
    Option::ProgressObscure,

    // Additional Randomization Options
    Option::SwordMode,
    Option::DungeonSmallKeys,
    Option::DungeonBigKeys,
    Option::DungeonMapsAndCompasses,
    Option::NumRequiredDungeons,
    Option::NumShards,
    Option::RandomCharts,
    Option::CTMC,
    Option::DamageMultiplier,

    // Convenience Tweaks
    Option::InstantText,
    Option::FixRNG,
    Option::RevealSeaChart,
    Option::SkipRefights,
    Option::AddShortcutWarps,
    Option::RemoveMusic,

    // Starting Gear
    Option::StartingGear,
    Option::StartingHC,
    Option::StartingHP,
    Option::StartingJoyPendants,
    Option::StartingSkullNecklaces,
    Option::StartingBokoBabaSeeds,
    Option::StartingGoldenFeathers,
    Option::StartingKnightsCrests,
    Option::StartingRedChuJellys,
    Option::StartingGreenChuJellys,
    Option::StartingBlueChuJellys,

    // Advanced Options
    Option::NoSpoilerLog,
    Option::StartWithRandomItem,
    Option::Plandomizer,

    // Hints
    Option::HoHoHints,
    Option::KorlHints,
    Option::PathHints,
    Option::BarrenHints,
    Option::ItemHints,
    Option::LocationHints,
    Option::UseAlwaysHints,
    Option::ClearerHints,

    // Entrance Randomizer
    Option::RandomizeDungeonEntrances,
    Option::RandomizeBossEntrances,
    Option::RandomizeCaveEntrances,
    Option::RandomizeDoorEntrances,
    Option::RandomizeMiscEntrances,
    Option::MixDungeons,
    Option::MixBosses,
    Option::MixCaves,
    Option::MixDoors,
    Option::MixMisc,
    Option::DecoupleEntrances,
    Option::RandomStartIsland,

};

std::string create_permalink(const Settings& settings, const std::string& seed) {

    std::string permalink = "";
    permalink += std::string(RANDOMIZER_VERSION).substr(0, 3);
    permalink += '\0';
    permalink += seed;
    permalink += '\0';

    // Pack the settings up
    auto bitsWriter = PackedBitsWriter();
    for(const auto& option : PERMALINK_OPTIONS) {

        // Handle starting gear
        if (option == Option::StartingGear) {
            std::multiset<GameItem> startingGear  (settings.starting_gear.begin(), settings.starting_gear.end());
            for (size_t i = 0; i < REGULAR_ITEMS.size(); i++)
            {
                size_t bit = startingGear.contains(REGULAR_ITEMS[i]);
                bitsWriter.write(bit, 1);
            }
            for (auto& item : PROGRESSIVE_ITEMS)
            {
                bitsWriter.write(startingGear.count(item), 3);
            }
        }
        // ComboBox Options (and 8-bit SpinBox options)
        else if (option == Option::SwordMode               ||
                 option == Option::DungeonSmallKeys        ||
                 option == Option::DungeonBigKeys          ||
                 option == Option::DungeonMapsAndCompasses ||
                 option == Option::NumRequiredDungeons     ||
                 option == Option::NumShards               ||
                 option == Option::DamageMultiplier)
        {
            bitsWriter.write(getSetting(settings, option), 8);
        }
        // 3-bit SpinBox options
        else if (option == Option::ProgressDungeons ||
                 option == Option::PathHints        ||
                 option == Option::BarrenHints      ||
                 option == Option::LocationHints    ||
                 option == Option::ItemHints        ||
                 option == Option::StartingHC)
        {
            bitsWriter.write(getSetting(settings, option), 3);
        }
        // 6-bit SpinBox options
        else if (option == Option::StartingHP             ||
                 option == Option::StartingJoyPendants    ||
                 option == Option::StartingSkullNecklaces ||
                 option == Option::StartingBokoBabaSeeds  ||
                 option == Option::StartingGoldenFeathers ||
                 option == Option::StartingKnightsCrests  ||
                 option == Option::StartingRedChuJellys   ||
                 option == Option::StartingGreenChuJellys ||
                 option == Option::StartingBlueChuJellys)
        {
            bitsWriter.write(getSetting(settings, option), 6);
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

PermalinkError parse_permalink(std::string b64permalink, Settings& settings, std::string& seed)
{
    // Strip trailing spaces
    erase_if(b64permalink, [](unsigned char ch){ return std::isspace(ch);});

    if (b64permalink.empty())
    {
        return PermalinkError::NONE;
    }
    auto permalink = b64_decode(b64permalink);
    // Empty string gets returned if there was an error
    if (permalink == "")
    {
        return PermalinkError::BAD_PERMALINK;
    }

    // Split the string into 3 parts along the null terminator delimiter
    // 1st part - Version string
    // 2nd part - seed string
    // 3rd part - packed bits representing settings
    std::vector<std::string> permaParts = {};
    size_t pos = 0;
    char delimiter = '\0';
    while ((pos = permalink.find(delimiter)) != std::string::npos) {
        if (permaParts.size() != 2)
        {
            permaParts.push_back(permalink.substr(0, pos));
            permalink.erase(0, pos + 1);
        }
        else
        {
            permaParts.push_back(permalink);
            break;
        }

    }

    if (permaParts.size() != 3)
    {
        for (auto& str : permaParts)
        {
            std::cout << str << std::endl;
        }
        return PermalinkError::BAD_PERMALINK;
    }

    std::string version = permaParts[0];
    seed = permaParts[1];
    std::string optionsBytes = permaParts[2];

    if (version != std::string(RANDOMIZER_VERSION).substr(0, 3))
    {
        return PermalinkError::INVALID_VERSION;
    }

    std::vector<char> bytes (optionsBytes.begin(), optionsBytes.end());
    auto bitsReader = PackedBitsReader(bytes);

    for(const auto& option : PERMALINK_OPTIONS) {
        size_t value = 0;
        // Handle starting gear
        if (option == Option::StartingGear) {
            settings.starting_gear.clear();
            for (size_t i = 0; i < REGULAR_ITEMS.size(); i++)
            {
                value = bitsReader.read(1);
                BYTES_EXIST_CHECK(value);
                if (value == 1)
                {
                    settings.starting_gear.push_back(REGULAR_ITEMS[i]);
                }
            }
            for (auto& item : PROGRESSIVE_ITEMS)
            {
                value = bitsReader.read(3);
                BYTES_EXIST_CHECK(value);
                for (size_t i = 0; i < value; i++)
                {
                    settings.starting_gear.push_back(item);
                }
            }
        }
        // ComboBox Options (and 8-bit spinbox options)
        else if (option == Option::SwordMode               ||
                 option == Option::DungeonSmallKeys        ||
                 option == Option::DungeonBigKeys          ||
                 option == Option::DungeonMapsAndCompasses ||
                 option == Option::NumRequiredDungeons     ||
                 option == Option::NumShards               ||
                 option == Option::DamageMultiplier)
        {
            value = bitsReader.read(8);
            BYTES_EXIST_CHECK(value);
            setSetting(settings, option, value);
        }
        // 3-bit SpinBox options
        else if (option == Option::PathHints     ||
                 option == Option::BarrenHints   ||
                 option == Option::LocationHints ||
                 option == Option::ItemHints     ||
                 option == Option::StartingHC)
        {
            value = bitsReader.read(3);
            BYTES_EXIST_CHECK(value);
            setSetting(settings, option, value);
        }
        // 6-bit SpinBox options
        else if (option == Option::StartingHP             ||
                 option == Option::StartingJoyPendants    ||
                 option == Option::StartingSkullNecklaces ||
                 option == Option::StartingBokoBabaSeeds  ||
                 option == Option::StartingGoldenFeathers ||
                 option == Option::StartingKnightsCrests  ||
                 option == Option::StartingRedChuJellys   ||
                 option == Option::StartingGreenChuJellys ||
                 option == Option::StartingBlueChuJellys)
        {
            value = bitsReader.read(6);
            BYTES_EXIST_CHECK(value);
            setSetting(settings, option, value);
        }
        // 1-bit Checkbox options
        else
        {
            value = bitsReader.read(1);
            BYTES_EXIST_CHECK(value);
            setSetting(settings, option, value);
        }
    }
    if (bitsReader.current_byte_index != bitsReader.bytes.size() - 1)
    {
        return PermalinkError::BAD_PERMALINK;
    }
    return PermalinkError::NONE;
}

std::string errorToName(PermalinkError err)
{
    switch(err)
    {
        case PermalinkError::NONE:
            return "NONE";
        case PermalinkError::INVALID_VERSION:
            return "INVALID_VERSION";
        case PermalinkError::BAD_PERMALINK:
            return "BAD_PERMALINK";
        default:
            return "UNKNOWN";
    }
}
