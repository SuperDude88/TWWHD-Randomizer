
#pragma once

#include <memory>
#include <string>
#include <unordered_set>
#include <unordered_map>

#include <logic/GameItem.hpp>
#include <logic/PoolFunctions.hpp>
#include <logic/Hints.hpp>
#include <logic/Requirements.hpp>
#include <command/WriteLocations.hpp>

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
    BlueChuChu,
    DungeonSecret,
    Obscure, // <-- the good stuff :)
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

class World;
struct LocationAccess;
class Location
{
public:
    std::unordered_map<std::string, std::string> names = {};
    std::unordered_set<LocationCategory> categories;
    bool progression;
    bool isRaceModeLocation;
    bool plandomized;
    bool hasBeenHinted;
    bool hasKnownVanillaItem;
    bool hasDungeonDependency;
    Item originalItem;
    Item currentItem;
    int sortPriority = -1;
    std::list<std::string> hintRegions;
    std::list<LocationAccess*> accessPoints;
    std::string hintPriority = "";
    std::unique_ptr<LocationModification> method;
    World* world = nullptr;
    Requirement computedRequirement = Requirement{RequirementType::IMPOSSIBLE, {}};
    std::unordered_set<GameItem> itemsInComputedRequirement = {};
    std::vector<Location*> pathLocations = {};

    // Variables used for the searching algorithm
    bool hasBeenFound = false;

    // Message Label if this is a hint location
    std::string messageLabel;

    // Hint for this location
    Hint hint;

    // goal names if this is a race mode location (one for each language)
    std::unordered_map<std::string, std::string> goalNames = {};

    // Tracker properties
    bool marked;
    std::string trackerNote;
    std::set<std::string> trackerNoteAreas;

    Location() :
        names({}),
        categories({LocationCategory::INVALID}),
        progression(false),
        isRaceModeLocation(false),
        plandomized(false),
        hasBeenHinted(false),
        hasKnownVanillaItem(false),
        hasDungeonDependency(false),
        originalItem(GameItem::INVALID, nullptr),
        currentItem(GameItem::INVALID, nullptr),
        sortPriority(-1),
        hintRegions({}),
        accessPoints({}),
        hintPriority(""),
        method(std::make_unique<LocationModification>()),
        world(nullptr),
        hasBeenFound(false),
        messageLabel(""),
        hint(Hint()),
        goalNames({}),
        marked(false)
    {}
    ~Location() = default;
    Location(const Location& loc) = delete;
    Location& operator=(const Location&) = delete;
    Location(Location&&) = default;
    Location& operator=(Location&&) = default;
    bool operator<(const Location& rhs) const;

    std::string getName() const;
    std::u16string generateImportanceText(const std::string& language);
    bool currentItemCanBeBarren() const;
};

using LocationSet = std::set<Location*, PointerLess<Location>>;
