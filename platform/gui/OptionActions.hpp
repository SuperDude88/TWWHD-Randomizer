#pragma once

#include <seedgen/config.hpp>

// toggles setting, returns new value to display
using TriggerCallback = std::string(*)();

namespace OptionCB {
    void changeSeed();

    std::string cycleDungeonMode();
    std::string toggleFairies();
    std::string togglePuzzleCaves();
    std::string toggleCombatCaves();
    std::string toggleShortSidequests();
    std::string toggleLongSidequests();
    std::string toggleSpoilsTrading();
    std::string toggleMinigames();
    std::string toggleFreeGifts();
    std::string toggleMail();
    std::string togglePlatforms();
    std::string toggleSubmarines();
    std::string toggleReefs();
    std::string toggleOctos();
    std::string toggleTriforceCharts();
    std::string toggleTreasureCharts();
    std::string toggleExpensivePurchases();
    std::string toggleMisc();
    std::string toggleTingleChests();
    std::string toggleBattlesquid();
    std::string toggleSavage();
    std::string toggleIslandPuzzles();
    std::string toggleDungeonSecrets();
    std::string toggleObscure();

    std::string cycleSmallKeyMode();
    std::string cycleBigKeyMode();
    std::string cycleMapCompassMode();
    std::string toggleChartShuffle();
    std::string toggleRandomIsland();
    std::string toggleDungeonEntranceShuffle();
    std::string toggleBossEntranceShuffle();
    std::string toggleMinibossEntranceShuffle();
    std::string toggleCaveEntranceShuffle();
    std::string toggleDoorEntranceShuffle();
    std::string toggleMiscEntranceShuffle();
    std::string toggleDungeonEntranceMix();
    std::string toggleBossEntranceMix();
    std::string toggleMinibossEntranceMix();
    std::string toggleCaveEntranceMix();
    std::string toggleDoorEntranceMix();
    std::string toggleMiscEntranceMix();
    std::string toggleDecoupleEntrances();

    std::string toggleHoHoHints();
    std::string toggleKorlHints();
    std::string toggleClearHints();
    std::string toggleAlwaysHints();
    std::string cyclePathHints();
    std::string cycleBarrenHints();
    std::string cycleItemHints();
    std::string cycleLocationHints();

    std::string toggleInstantText();
    std::string toggleRNG();
    std::string toggleFullSeaChart();
    std::string cycleStartingShards();
    std::string toggleDungeonWarps();
    std::string toggleSpoilerLog();
    std::string cycleSwordMode();
    std::string toggleTrials();
    std::string toggleInvertCompass();
    std::string cycleNumDungeons();
    std::string cycleDamageMultiplier();
    std::string toggleCTMC();

    std::string toggleCasualClothes();
    std::string cyclePigColor();

    //this one gets special handling
    //std::string setStartingGear();
    std::string cycleStartingHP();
    std::string cycleStartingHC();
    std::string cycleStartingJP();
    std::string cycleStartingSN();
    std::string cycleStartingBS();
    std::string cycleStartingGF();
    std::string cycleStartingKC();
    std::string cycleStartingRC();
    std::string cycleStartingGC();
    std::string cycleStartingBC();
    std::string toggleMusic();

    std::string toggleRandomStartItem();
    std::string togglePlandomizer();

    std::string toggleTargetPref();
    std::string toggleCameraPref();
    std::string toggleFirstPersonPref();
    std::string toggleGyroPref();
    std::string toggleUIPref();

    //special handling for these too
    //std::string setColors();

    std::string invalidCB();

    void setInternal(const Config& in);
    Config getInternal();
}

std::string getSeed();
std::string getSeedHash();
std::string getValue(const Option& option);
TriggerCallback getCallback(const Option& option);
std::pair<std::string, std::string> getNameDesc(const Option& option);
