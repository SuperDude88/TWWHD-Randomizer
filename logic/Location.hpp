
#pragma once

#include <memory>
#include <string>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include "GameItem.hpp"
#include "Requirements.hpp"
#include "../server/command/WriteLocations.hpp"

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
    Other,
    AlwaysProgression,
    HoHoHint,
    PlandomizerProgression,
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

LocationCategory nameToLocationCategory(const std::string& name);
LocationModificationType nameToModificationType(const std::string& name);

class World;
struct Location
{
    std::unordered_map<std::string, std::string> names = {};
    std::unordered_set<LocationCategory> categories;
    bool progression;
    bool isRaceModeLocation;
    bool plandomized;
    bool hasBeenHinted;
    Item originalItem;
    Item currentItem;
    int sortPriority = -1;
    std::unordered_set<std::string> hintRegions;
    std::string hintPriority = "";
    std::unique_ptr<LocationModification> method;
    World* world = nullptr;

    // Variables used for the searching algorithm
    bool hasBeenFound = false;

    // Message Label if this is a hint location
    std::string messageLabel;

    // hint message for this location (one for each language)
    std::unordered_map<std::string, std::u16string> hintText = {};

    // goal names if this is a race mode location (one for each language)
    std::unordered_map<std::string, std::string> goalNames = {};

    Location() :
        names({}),
        categories({LocationCategory::INVALID}),
        progression(false),
        isRaceModeLocation(false),
        plandomized(false),
        hasBeenHinted(false),
        originalItem(GameItem::INVALID, nullptr),
        currentItem(GameItem::INVALID, nullptr),
        sortPriority(-1),
        hintRegions({}),
        hintPriority(""),
        method(std::make_unique<LocationModification>()),
        world(nullptr),
        hasBeenFound(false),
        messageLabel(""),
        hintText({}),
        goalNames({})
    {
    }
    ~Location() = default;
    Location(const Location& loc) = delete;
    Location& operator=(const Location&) = delete;
    Location(Location&&) = default;
    Location& operator=(Location&&) = default;
    bool operator<(const Location& rhs) const;

    std::string getName() const;
};
