#include "permalink.hpp"

#include <array>



static const std::array<Option, 47> PERMALINK_OPTIONS {
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
    Option::Keylunacy,
    Option::RandomCharts,
    Option::RandomStartIsland,
    Option::RandomizeDungeonEntrances,
    Option::RandomizeCaveEntrances,
    Option::RandomizeDoorEntrances,
    Option::RandomizeMiscEntrances,
    Option::MixEntrancePools,
    Option::DecoupleEntrances,
    Option::InstantText,
    Option::RevealSeaChart,
    Option::NumShards,
    Option::AddShortcutWarps,
    Option::NoSpoilerLog,
    Option::SwordMode,
    Option::SkipRefights,
    Option::RaceMode,
    Option::NumRaceModeDungeons,
    Option::DamageMultiplier,
    Option::StartingGear,
    Option::StartingHP,
    Option::StartingHC,
    Option::RemoveMusic,
    Option::Plandomizer,
};

std::string getSettingsStr(const Settings& settings) {
    std::string gear;
    std::string ret;

    for(const auto& option : PERMALINK_OPTIONS) {
        if(option == Option::StartingGear) {
            for(const auto& item : settings.starting_gear) {
                gear += static_cast<uint8_t>(item);
            }

            continue;
        }

        ret += static_cast<int8_t>(getSetting(settings, option));
    }

    return ret + gear; //let gear be variable length, keep it after other settings
}

std::string create_permalink(const Settings& settings, const std::string& seed) {
    return std::string(RANDOMIZER_VERSION) + '\0' + seed + '\0'+ getSettingsStr(settings);
}
