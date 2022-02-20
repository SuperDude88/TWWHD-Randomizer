
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
#include "../libs/ryml.hpp"

static std::stringstream lastError;

class World;
using WorldPool = std::vector<World>;
using LocationPool = std::vector<Location*>;


struct Exit
{
    Area parentArea = Area::INVALID;
    Area connectedArea = Area::INVALID;
    Requirement requirement;
    int worldId = -1;
};

struct AreaEntry
{
    Area area = Area::INVALID;
    std::list<Location*> locations;
    std::list<Exit> exits;
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
        MACRO_MISSING_KEY,
        MACRO_MISSING_VAL,
        REQUIREMENT_MISISNG_KEY,
        INVALID_LOCATION_CATEGORY,
        INVALID_MODIFICATION_TYPE,
        INVALID_OFFSET_VALUE,
        INVALID_GAME_ITEM,
        LOGIC_SYMBOL_DOES_NOT_EXIST,
        COULD_NOT_DETERMINE_TYPE,
        SAME_NESTING_LEVEL,
        EXTRA_OR_MISSING_PARENTHESIS,
    };

    World();

    void setSettings(const Settings& settings);
    const Settings& getSettings() const;
    void setWorldId(int newWorldId);
    int getWorldId() const;
    void setItemPools();
    ItemPool getItemPool() const;
    void addToItemPool(const GameItem gameItem);
    ItemPool getStartingItems() const;
    LocationPool getLocations();

    void determineChartMappings();
    void determineProgressionLocations();
    void determineRaceModeDungeons();
    int loadWorld(const std::string& worldFilePath, const std::string& macrosFilePath, const std::string& locationDataPath);
    static const char* errorToName(WorldLoadingError err);
    std::string getLastErrorDetails();
    void dumpWorldGraph(const std::string& filename);

    std::vector<AreaEntry> areaEntries = {};
    std::vector<Location> locationEntries = {};
    std::unordered_map<std::string, MacroIndex> macroNameMap;
    std::vector<Requirement> macros;
    std::vector<std::list<Location*>> playthroughSpheres = {};
    std::array<GameItem, 49> chartMappings;
    std::vector<DungeonId> raceModeDungeons;

private:

    bool chartLeadsToSunkenTreasure(const Location& location, const std::string& itemPrefix);


    WorldLoadingError parseRequirementString( const std::string& str, Requirement& req);
    WorldLoadingError parseMacro(const std::string& macroLogicExpression, Requirement& reqOut);
    WorldLoadingError loadExit(const std::string& connectedAreaName, const std::string& logicExpression, Exit& loadedExit, Area& parentArea);
    WorldLoadingError loadLocation(const ryml::NodeRef& locationObject, LocationId& loadedLocation);
    WorldLoadingError loadLocationRequirement(const std::string& locationName, const std::string& logicExpression, LocationId& loadedLocation);
    WorldLoadingError loadMacros(const ryml::Tree& macroListTree);
    WorldLoadingError loadArea(const ryml::NodeRef& areaObject, Area& loadedArea);
    int getFileContents(const std::string& filename, std::string& fileContents);

    Settings settings;
    ItemPool itemPool;
    ItemPool startingItems;
    int worldId = -1;
};
