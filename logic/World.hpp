
#pragma once

#include <string>
#include <sstream>
#include <unordered_set>
#include <list>

#include <libs/yaml.h>
#include <logic/Requirements.hpp>
#include <logic/Area.hpp>
#include <logic/Location.hpp>
#include <logic/ItemPool.hpp>
#include <logic/Dungeon.hpp>
#include <logic/Entrance.hpp>
#include <logic/Plandomizer.hpp>
#include <logic/PoolFunctions.hpp>
#include <logic/WorldPool.hpp>
#include <utility/text.hpp>

#define GET_COMPLETE_ITEM_POOL(itemPool, worlds) for (auto& world : worlds) {addElementsToPool(itemPool, world.getItemPool());}
#define GET_COMPLETE_PROGRESSION_LOCATION_POOL(locationPool, worlds) for (auto& world : worlds) {addElementsToPool(locationPool, world.getProgressionLocations());}
#define ANY_WORLD_HAS_RACE_MODE(worlds) std::any_of(worlds.begin(), worlds.end(), [](World& world){return world.getSettings().progression_dungeons == ProgressionDungeons::RaceMode;})

static std::stringstream lastError;

using LocationPool = std::vector<Location*>;
using EntrancePool = std::vector<Entrance*>;

struct AreaEntry;
struct LocationAccess
{
    AreaEntry* area = nullptr;
    Location* location = nullptr;
    Requirement requirement;
};

struct EventAccess
{
    EventId event;
    Requirement requirement;
    int worldId = -1;
};

struct AreaEntry
{
    std::string name = "";
    std::string island = "";
    std::string dungeon = "";
    std::string hintRegion = "";
    std::list<EventAccess> events;
    std::list<LocationAccess> locations;
    std::list<Entrance> exits;
    std::list<Entrance*> entrances;
    int worldId = -1;

    // variables used for the searching algorithm
    bool isAccessible = false;
};


class World
{
public:

    enum struct WorldLoadingError
    {
        NONE = 0,
        DUPLICATE_MACRO_NAME,
        MACRO_DOES_NOT_EXIST,
        REQUIREMENT_TYPE_DOES_NOT_EXIST,
        MAPPING_MISMATCH,
        GAME_ITEM_DOES_NOT_EXIST,
        AREA_DOES_NOT_EXIST,
        LOCATION_DOES_NOT_EXIST,
        EXIT_MISSING_KEY,
        OPTION_DOES_NOT_EXIST,
        INCORRECT_ARG_COUNT,
        AREA_MISSING_KEY,
        LOCATION_MISSING_KEY,
        LOCATION_MISSING_VAL,
        MACRO_MISSING_KEY,
        MACRO_MISSING_VAL,
        ITEM_MISSING_KEY,
        REQUIREMENT_MISSING_KEY,
        INVALID_LOCATION_CATEGORY,
        INVALID_MODIFICATION_TYPE,
        INVALID_OFFSET_VALUE,
        INVALID_GAME_ITEM,
        LOGIC_SYMBOL_DOES_NOT_EXIST,
        COULD_NOT_DETERMINE_TYPE,
        SAME_NESTING_LEVEL,
        EXTRA_OR_MISSING_PARENTHESIS,
        PLANDOMIZER_ERROR,
        DUNGEON_HAS_NO_RACE_MODE_LOCATION,
        INVALID_DUNGEON_NAME,
        UNKNOWN,
        COUNT
    };

    World();
    World(size_t numWorlds_);

    void setSettings(const Settings& settings);
    const Settings& getSettings() const;
    void setWorldId(int newWorldId);
    int getWorldId() const;
    WorldLoadingError setItemPools();
    ItemPool getItemPool() const;
    ItemPool& getItemPoolReference();
    ItemPool getStartingItems() const;
    LocationPool getLocations(bool onlyProgression = false);
    LocationPool getRaceModeLocations() const;
    LocationPool getProgressionLocations();
    size_t getNumOverworldProgressionLocations();
    AreaEntry& getArea(const std::string& area);

    void resolveRandomSettings();
    void addSpoilsToStartingGear();
    void determineChartMappings();
    WorldLoadingError determineProgressionLocations();
    WorldLoadingError setDungeonLocations();
    WorldLoadingError determineRaceModeDungeons(WorldPool& worlds);
    int loadWorld(const std::string& worldFilePath, const std::string& macrosFilePath, const std::string& locationDataPath, const std::string& itemDataPath, const std::string& areaDataPath);
    Entrance* getEntrance(const std::string& parentArea, const std::string& connectedArea);
    void removeEntrance(Entrance* entranceToRemove);
    EntrancePool getShuffleableEntrances(const EntranceType& type, const bool& onlyPrimary = false);
    EntrancePool getShuffledEntrances(const EntranceType& type, const bool& onlyPrimary = false);
    std::unordered_set<std::string> getRegions(const std::string& area, const std::string& regionType, const std::unordered_set<std::string>& typesToIgnore = {});
    std::unordered_set<std::string> getIslands(const std::string& area);
    std::unordered_set<std::string> getDungeons(const std::string& area);
    Dungeon& getDungeon(const std::string& dungeonName);
    WorldLoadingError processPlandomizerLocations(WorldPool& worlds);
    std::string getUTF8HintRegion(const std::string& hintRegion, const std::string& language = "English", const Text::Type& type = Text::Type::STANDARD, const Text::Color& color = Text::Color::RAW) const;
    std::u16string getUTF16HintRegion(const std::string& hintRegion, const std::string& language = "English", const Text::Type& type = Text::Type::STANDARD, const Text::Color& color = Text::Color::RED) const;

    // Stuff to help with debugging
    std::string errorToName(WorldLoadingError err);
    std::string getLastErrorDetails();
    void dumpWorldGraph(const std::string& filename, bool onlyRandomizedExits = false);

    std::unordered_map<std::string, MacroIndex> macroNameMap;
    std::vector<Requirement> macros;
    std::map<std::string, Item> itemEntries = {};
    std::map<std::string, AreaEntry> areaEntries = {};
    std::map<std::string, Location> locationEntries = {};
    std::map<GameItem, std::map<std::string, Text::Translation>> itemTranslations; // game item names for all languages, keyed by GameItemId, language, and type
    std::map<std::string, std::map<std::string, Text::Translation>> hintRegions; // hint region names for all languages, keyed by name, language, and type
    std::unordered_map<std::string, EventId> eventMap = {};
    std::unordered_map<EventId, std::string> reverseEventMap = {};
    std::unordered_map<std::string, Dungeon> dungeons = {};
    LocationPool raceModeLocations = {};
    std::unordered_map<Location*, std::vector<Location*>> pathLocations = {};
    std::unordered_map<std::string, std::unordered_set<Location*>> barrenRegions = {};
    std::list<Location*> korlHints = {};
    std::unordered_map<Location*, std::unordered_set<Location*>> hohoHints = {}; // map of Ho Ho Hint Location to hinted locations
    Location* bigOctoFairyHintLocation = nullptr;
    std::list<std::list<Location*>> playthroughSpheres = {};
    std::list<std::list<Entrance*>> entranceSpheres = {};
    std::unordered_map<uint8_t, GameItem> chartMappings = {};
    Settings originalSettings;

    uint8_t startingIslandRoomIndex = 44; // Outset Island by default
    Plandomizer plandomizer;


private:

    bool chartLeadsToSunkenTreasure(const Location& location, const std::string& itemPrefix);


    WorldLoadingError parseRequirementString( const std::string& str, Requirement& req);
    WorldLoadingError parseMacro(const std::string& macroLogicExpression, Requirement& reqOut);
    WorldLoadingError loadExit(const std::string& connectedAreaName, const std::string& logicExpression, Entrance& loadedExit, const std::string& parentArea);
    WorldLoadingError loadLocation(const YAML::Node& locationObject);
    WorldLoadingError loadEventRequirement(const std::string& eventName, const std::string& logicExpression, EventAccess& eventAccess);
    WorldLoadingError loadLocationRequirement(const std::string& locationName, const std::string& logicExpression, LocationAccess& loadedLocation);
    WorldLoadingError loadMacros(const YAML::Node& macroListTree);
    WorldLoadingError loadArea(const YAML::Node& areaObject);
    WorldLoadingError loadItem(const YAML::Node& itemObject);
    WorldLoadingError loadAreaTranslations(const YAML::Node& areaObject);

    Settings settings;
    ItemPool itemPool;
    ItemPool startingItems;
    int worldId = -1;
    size_t numWorlds = 1;
};
