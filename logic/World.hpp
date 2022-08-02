
#pragma once

#include <string>
#include <sstream>
#include <unordered_set>
#include <list>
#include "Requirements.hpp"
#include "Area.hpp"
#include "Location.hpp"
#include "ItemPool.hpp"
#include "Dungeon.hpp"
#include "Entrance.hpp"
#include "../libs/Yaml.hpp"

static std::stringstream lastError;

class World;
using WorldPool = std::vector<World>;
using LocationPool = std::vector<Location*>;
using EntrancePool = std::vector<Entrance*>;

struct LocationAccess
{
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
        EXPECTED_JSON_OBJECT,
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
    ItemPool getStartingItems() const;
    LocationPool getLocations(bool onlyProgression = false);
    AreaEntry& getArea(const std::string& area);

    void resolveRandomSettings();
    void determineChartMappings();
    void determineProgressionLocations();
    WorldLoadingError determineRaceModeDungeons();
    int loadWorld(const std::string& worldFilePath, const std::string& macrosFilePath, const std::string& locationDataPath, const std::string& itemDataPath);
    Entrance* getEntrance(const std::string& parentArea, const std::string& connectedArea);
    void removeEntrance(Entrance* entranceToRemove);
    EntrancePool getShuffleableEntrances(const EntranceType& type, const bool& onlyPrimary = false);
    EntrancePool getShuffledEntrances(const EntranceType& type, const bool& onlyPrimary = false);
    std::unordered_set<std::string> getIslands(const std::string& area);
    Dungeon& getDungeon(const std::string& dungeonName);
    WorldLoadingError loadPlandomizer();
    static const char* errorToName(WorldLoadingError err);
    std::string getLastErrorDetails();
    void dumpWorldGraph(const std::string& filename, bool onlyRandomizedExits = false);

    std::unordered_map<std::string, MacroIndex> macroNameMap;
    std::vector<Requirement> macros;
    std::map<std::string, Item> itemEntries = {};
    std::map<std::string, AreaEntry> areaEntries = {};
    std::map<std::string, Location> locationEntries = {};
    std::unordered_map<std::string, EventId> eventMap = {};
    std::unordered_map<EventId, std::string> reverseEventMap = {};
    std::unordered_map<Location*, Item> plandomizerLocations = {};
    std::unordered_map<Entrance*, Entrance*> plandomizerEntrances = {};
    std::unordered_map<std::string, Dungeon> dungeons = {};
    std::unordered_map<Location*, std::vector<Location*>> pathLocations = {};
    std::unordered_map<std::string, std::unordered_set<Location*>> barrenRegions = {};
    std::list<Location*> korlHints = {};
    std::unordered_map<Location*, std::unordered_set<Location*>> hohoHints = {}; // map of Ho Ho Hint Location to hinted locations
    std::list<std::list<Location*>> playthroughSpheres = {};
    std::list<std::list<Entrance*>> entranceSpheres = {};
    std::array<GameItem, 49> chartMappings;

    uint8_t startingIslandRoomIndex = 44;


private:

    bool chartLeadsToSunkenTreasure(const Location& location, const std::string& itemPrefix);


    WorldLoadingError parseRequirementString( const std::string& str, Requirement& req);
    WorldLoadingError parseMacro(const std::string& macroLogicExpression, Requirement& reqOut);
    WorldLoadingError loadExit(const std::string& connectedAreaName, const std::string& logicExpression, Entrance& loadedExit, const std::string& parentArea);
    WorldLoadingError loadLocation(Yaml::Node& locationObject);
    WorldLoadingError loadEventRequirement(const std::string& eventName, const std::string& logicExpression, EventAccess& eventAccess);
    WorldLoadingError loadLocationRequirement(const std::string& locationName, const std::string& logicExpression, LocationAccess& loadedLocation);
    WorldLoadingError loadMacros(Yaml::Node& macroListTree);
    WorldLoadingError loadArea(Yaml::Node& areaObject);
    WorldLoadingError loadItem(Yaml::Node& itemObject);
    int getFileContents(const std::string& filename, std::string& fileContents);

    Settings settings;
    ItemPool itemPool;
    ItemPool startingItems;
    int worldId = -1;
    size_t numWorlds = 1;
};
