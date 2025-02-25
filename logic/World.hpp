
#pragma once

#include <string>
#include <unordered_set>
#include <list>
#include <memory>

#include <libs/yaml.hpp>
#include <logic/Requirements.hpp>
#include <logic/Area.hpp>
#include <logic/Location.hpp>
#include <logic/ItemPool.hpp>
#include <logic/Dungeon.hpp>
#include <logic/Entrance.hpp>
#include <logic/Plandomizer.hpp>
#include <logic/WorldPool.hpp>
#include <utility/text.hpp>

#define GET_COMPLETE_ITEM_POOL(itemPool, worlds) for (auto& world : worlds) {addElementsToPool(itemPool, world.getItemPool());}
#define GET_COMPLETE_PROGRESSION_LOCATION_POOL(locationPool, worlds) for (auto& world : worlds) {addElementsToPool(locationPool, world.getProgressionLocations());}
#define ANY_WORLD_HAS_RACE_MODE(worlds) std::any_of(worlds.begin(), worlds.end(), [](World& world){return world.getSettings().progression_dungeons == ProgressionDungeons::RaceMode;})

using LocationPool = std::vector<Location*>;
using EntrancePool = std::vector<Entrance*>;


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
        DUNGEON_MISSING_KEY,
        REQUIREMENT_MISSING_KEY,
        INVALID_LOCATION_CATEGORY,
        INVALID_MODIFICATION_TYPE,
        INVALID_OFFSET_VALUE,
        INVALID_GAME_ITEM,
        BAD_REQUIREMENT,
        PLANDOMIZER_ERROR,
        DUNGEON_HAS_NO_RACE_MODE_LOCATION,
        INVALID_DUNGEON_NAME,
        UNKNOWN,
        COUNT
    };

    World() = default;
    explicit World(size_t numWorlds_) :
        numWorlds(numWorlds_)
    {}

    void setSettings(const Settings& settings);
    const Settings& getSettings() const;
    void setWorldId(int newWorldId);
    int getWorldId() const;
    WorldLoadingError setItemPools();
    ItemPool getItemPool() const;
    ItemPool& getItemPoolReference();
    ItemPool getStartingItems() const;
    ItemPool& getStartingItemsReference();
    int getStartingHeartCount() const;
    LocationPool getLocations(bool onlyProgression = false);
    LocationPool getRaceModeLocations() const;
    LocationPool getProgressionLocations();
    size_t getNumOverworldProgressionLocations();
    Area* getArea(const std::string& areaName);

    void resolveRandomSettings();
    void addSpoilsToStartingGear();
    void determineChartMappings();
    WorldLoadingError determineProgressionLocations();
    WorldLoadingError setDungeonLocations(WorldPool& worlds);
    WorldLoadingError determineRaceModeDungeons(WorldPool& worlds);
    int loadWorld(const fspath& worldFilePath, const fspath& macrosFilePath, const fspath& locationDataPath, const fspath& itemDataPath, const fspath& areaDataPath);
    Entrance* getEntrance(const std::string& parentArea, const std::string& connectedArea);
    Entrance* getEntrance(Area* parentArea, Area* connectedArea);
    void removeEntrance(Entrance* entranceToRemove);
    EntrancePool getShuffleableEntrances(const EntranceType& type, const bool& onlyPrimary = false);
    EntrancePool getShuffledEntrances(const EntranceType& type, const bool& onlyPrimary = false);
    Dungeon& getDungeon(const std::string& dungeonName);
    WorldLoadingError processPlandomizerLocations(WorldPool& worlds);
    std::string getUTF8HintRegion(const std::string& hintRegion, const std::string& language = "English", const Text::Type& type = Text::Type::STANDARD, const Text::Color& color = Text::Color::RAW) const;
    std::u16string getUTF16HintRegion(const std::string& hintRegion, const std::string& language = "English", const Text::Type& type = Text::Type::STANDARD, const Text::Color& color = Text::Color::RED) const;
    void addEvent(const std::string& eventName);
    void addLocation(const std::string& locationName);
    Item getItem(const std::string& itemName);
    void flattenLogicRequirements();

    // Stuff to help with debugging
    std::string errorToName(WorldLoadingError err);
    std::string getLastErrorDetails();
    void dumpWorldGraph(const std::string& filename, bool onlyRandomizedExits = false);

    std::unordered_map<std::string, MacroIndex> macroNameMap;
    std::vector<Requirement> macros;
    std::unordered_map<MacroIndex, std::string> macroNames;
    std::unordered_map<std::string, std::string> macroStrings;
    std::map<std::string, Item> itemTable = {};
    std::map<std::string, std::unique_ptr<Area>> areaTable = {};
    std::map<std::string, std::unique_ptr<Location>> locationTable = {};
    std::map<GameItem, std::map<std::string, Text::Translation>> itemTranslations; // game item names for all languages, keyed by GameItemId, language, and type
    std::map<std::string, std::map<std::string, Text::Translation>> hintRegions; // hint region names for all languages, keyed by name, language, and type
    std::unordered_map<std::string, EventId> eventMap = {};
    std::unordered_map<EventId, std::string> reverseEventMap = {};
    std::map<std::string, Dungeon> dungeons = {};
    LocationPool raceModeLocations = {};
    std::list<Location*> goalLocations = {};
    std::map<std::string, std::unordered_set<Location*>> barrenRegions = {};
    std::list<Location*> korlHints = {};
    std::map<Location*, std::unordered_set<Location*>, PointerLess<Location>> hohoHints = {}; // map of Ho Ho Hint Location to hinted locations
    Location* bigOctoFairyHintLocation = nullptr;
    std::list<std::list<Location*>> playthroughSpheres = {};
    std::list<std::list<Entrance*>> entranceSpheres = {};
    std::map<uint8_t, GameItem> chartMappings = {};
    Settings originalSettings;

    uint8_t startingIslandRoomNum = 44; // Outset Island by default
    Plandomizer plandomizer;


private:

    bool chartLeadsToSunkenTreasure(Location* location, const std::string& itemPrefix);


    // WorldLoadingError parseRequirementString( const std::string& str, Requirement& req);
    RequirementError parseMacro(const std::string& macroLogicExpression, Requirement& reqOut);
    WorldLoadingError loadExit(const std::string& connectedAreaName, const std::string& logicExpression, Entrance& loadedExit, const std::string& parentArea);
    WorldLoadingError loadLocation(const YAML::Node& locationObject);
    WorldLoadingError loadEventRequirement(const std::string& eventName, const std::string& logicExpression, EventAccess& eventAccess);
    WorldLoadingError loadLocationRequirement(const std::string& locationName, const std::string& logicExpression, LocationAccess& loadedLocation);
    WorldLoadingError loadMacros(const YAML::Node& macroListTree);
    WorldLoadingError loadArea(const YAML::Node& areaObject);
    WorldLoadingError loadItem(const YAML::Node& itemObject);
    WorldLoadingError loadAreaTranslations(const YAML::Node& areaObject);
    WorldLoadingError loadDungeonExitInfo();

    Settings settings;
    ItemPool itemPool;
    ItemPool startingItems;
    int worldId = -1;
    size_t numWorlds = 1;

public:
    static int eventCounter;
};
