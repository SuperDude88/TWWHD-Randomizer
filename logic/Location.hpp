
#pragma once

#include <memory>
#include <string>
#include <vector>
#include <unordered_set>
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

struct Location
{
    std::string name;
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
    int worldId = -1;

    // Variables used for the searching algorithm
    bool hasBeenFound = false;

    // Message Label if this is a hint location
    std::string messageLabel;

    // hint message for this location
    std::u16string hintText = u"";

    // goal name if this is a race mode location
    std::string goalName = "";

    Location() :
        name(""),
        categories({LocationCategory::INVALID}),
        progression(false),
        isRaceModeLocation(false),
        plandomized(false),
        hasBeenHinted(false),
        originalItem(GameItem::INVALID, -1),
        currentItem(GameItem::INVALID, -1),
        sortPriority(-1),
        hintRegions({}),
        hintPriority(""),
        method(std::make_unique<LocationModification>()),
        worldId(-1),
        hasBeenFound(false),
        messageLabel(""),
        hintText(u""),
        goalName("")
    {
    }
    ~Location() = default;
    Location(const Location& loc) = delete;
    Location& operator=(const Location&) = delete;
    Location(Location&&) = default;
    Location& operator=(Location&&) = default;
    bool operator<(const Location& rhs) const;
};



// std::string locationName(const Location* location);
