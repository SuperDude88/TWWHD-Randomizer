
#pragma once

#include <string>
#include <sstream>
#include <unordered_set>
#include "GameItem.hpp"
#include "Requirements.hpp"
#include "Location.hpp"

class LocationManager
{
public:

    enum struct LocationError
    {
        NONE = 0,
        DUPLICATE_MACRO_NAME,
        MACRO_DOES_NOT_EXIST,
        REQUIREMENT_TYPE_DOES_NOT_EXIST,
        GAME_ITEM_DOES_NOT_EXIST,
        LOCATION_DOES_NOT_EXIST,
        SETTING_DOES_NOT_EXIST,
        INCORRECT_ARG_COUNT,
        EXPECTED_JSON_OBJECT,
        LOCATION_MISSING_KEY,
        MACRO_MISSING_KEY,
        REQUIREMENT_MISISNG_KEY,
        INVALID_LOCATION_CATEGORY,
        INVALID_MODIFICATION_TYPE,
        INVALID_OFFSET_VALUE,
        INVALID_GAME_ITEM
    };

    using LocationIndex = uint32_t;
    using ItemSet = std::unordered_multiset<GameItem>;
    // right now assuming all settings are boolean (will change later)
    using Settings = std::unordered_set<Setting>;
    constexpr static uint32_t LOCATION_COUNT = static_cast<std::underlying_type_t<Location>>(Location::COUNT);

    LocationManager();

    LocationError loadMacros(const std::vector<json>& macroObjectList);
    LocationError loadLocation(const json& locationObject, Location& loadedLocation);
    bool isAccessible(Location location, const ItemSet& ownedItems, const Settings& settings);
    void getAccessibleLocations(const ItemSet& ownedItems, const Settings& settings, std::vector<Location>& accessibleLocations);
    std::vector<Location> assumedSearch(ItemSet& ownedItems, 
                                        const Settings& settings);
    void assumedFill(const ItemSet& items, const Settings& settings);
    static const char* errorToName(LocationError err);

    std::string getLastErrorDetails();
private:
    struct LocationEntry
    {
        Location location = Location::INVALID;
        std::unordered_set<LocationCategory> categories;
        GameItem originalItem = GameItem::INVALID;
        GameItem currentItem = GameItem::INVALID;
        Requirement requirement;
        LocationModificationMethod method;
    };

    constexpr static uint32_t locationAsIndex(Location loc) {
        return static_cast<std::underlying_type_t<Location>>(loc);
    }
    static Location indexAsLocation(uint32_t index) {
        if (index >= LOCATION_COUNT) return Location::INVALID;
        return static_cast<Location>(index);
    }

    LocationError parseElement(RequirementType type, const std::vector<json>& args, std::vector<Requirement::Argument>& out);
    LocationError parseRequirement(const json& requirementsObject, Requirement& out);
    LocationError parseMacro(const json& macroObject, Requirement& reqOut);
    bool evaluateRequirement(const Requirement& req, const ItemSet& ownedItems, const Settings& settings);

    std::stringstream lastError;
    // invalid game item indicates unitialized
    std::array<LocationEntry, LOCATION_COUNT> locationEntries;
    std::vector<Requirement> macros;
    std::unordered_map<std::string, MacroIndex> macroNameMap;
};
