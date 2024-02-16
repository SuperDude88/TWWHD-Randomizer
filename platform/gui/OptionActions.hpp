#pragma once

#include <seedgen/config.hpp>
#include <logic/GameItem.hpp>

// toggles setting, returns new value to display
using TriggerCallback = std::string(*)();

namespace OptionCB {
    void setSeed(const std::string& seed_);
    void changeSeed();
    void loadPermalink(const std::string& permalink_);

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
    std::string cycleCaveEntranceShuffle();
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
    std::string togglePerformance();
    std::string toggleFullSeaChart();
    std::string toggleDungeonWarps();
    std::string toggleSpoilerLog();
    std::string toggleSwords();
    std::string toggleTrials();
    std::string toggleInvertCompass();
    std::string cycleNumDungeons();
    std::string cycleDamageMultiplier();
    std::string toggleCTMC();

    std::string toggleCasualClothes();
    std::string isCasual();
    std::string randomizeColorsOrderly();
    std::string randomizeColorsChaotically();

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

    std::string invalidCB();

    void clearStartingItems();
    bool hasStartingItem(const GameItem& item, const size_t& num = 1); //num is for duplicated items
    void addStartingItem(const GameItem& item);

    void resetInternal();
    void setInternal(const Config& in);
    Config getInternal();
}

namespace ColorCB {
    std::string randomizeColor(const std::string& name_);
    //std::string pickColor(const std::string& name_); //handled as subpage
    std::string resetColor(const std::string& name_);
    std::string invalidCB(const std::string&);
}

bool wasUpdated();
bool wasConverted();
std::string getSeed();
std::string getSeedHash();
std::string getPermalink();
std::string getValue(const Option& option);
CustomModel& getModel();
TriggerCallback getCallback(const Option& option);
std::pair<std::string, std::string> getNameDesc(const Option& option);
