
#include "LocationManager.hpp"
#include "Requirements.hpp"
#include "../options.hpp"
#include <algorithm>
#include <cstdlib>
#include <climits>
#include <vector>
#include <random>

// some error checking macros for brevity and since we can't use exceptions
#define FIELD_CHECK(j, field, err) if(!j.contains(field)) {lastError << "Unable to retrieve field: \"" << field << '"'; return err;}
#define OBJECT_CHECK(j, msg) if(!j.is_object()) {lastError << msg << ": Not an Object."; return LocationError::EXPECTED_JSON_OBJECT;}
#define VALID_CHECK(e, invalid, msg, err) if(e == invalid) {lastError << msg; return err;}
#define ITEM_VALID_CHECK(item, msg) VALID_CHECK(item, GameItem::INVALID, msg, LocationError::GAME_ITEM_DOES_NOT_EXIST)
#define LOCATION_VALID_CHECK(loc, msg) VALID_CHECK(loc, Location::INVALID, msg, LocationError::LOCATION_DOES_NOT_EXIST)
#define SETTING_VALID_CHECK(set, msg) VALID_CHECK(set, Option::INVALID, msg, LocationError::SETTING_DOES_NOT_EXIST)


LocationManager::LocationManager()
{

}

LocationManager::LocationError LocationManager::parseElement(RequirementType type, const std::vector<json>& args, std::vector<Requirement::Argument>& out)
{
    std::string argStr;
    GameItem argItem = GameItem::INVALID;
    Location argLoc = Location::INVALID;
    Option argSetting = Option::INVALID;
    int countValue = 0;
    LocationError err = LocationError::NONE;
    if (args.size() == 0) return LocationError::INCORRECT_ARG_COUNT; // all ops require at least one arg
    switch(type)
    {
    case RequirementType::OR: // and/or expects list of sub-requirements
    case RequirementType::AND:
        for (const auto& arg : args) {
            out.emplace_back(Requirement());
            OBJECT_CHECK(arg, "Requirement element of type " << static_cast<int>(type) << " has unexpected type");
            if ((err = parseRequirement(arg, std::get<Requirement>(out.back()))) != LocationError::NONE) return err;
        }
        return LocationError::NONE;
    case RequirementType::NOT:
        out.emplace_back(Requirement());
        OBJECT_CHECK(args[0], "Requirement element of type " << static_cast<int>(type) << " has unexpected type");
        if ((err = parseRequirement(args[0], std::get<Requirement>(out.back()))) != LocationError::NONE) return err;
        return LocationError::NONE;
    case RequirementType::HAS_ITEM: 
        // has item expects a single string arg that maps to an item
        argStr = args[0].get<std::string>();
        argItem = nameToGameItem(argStr);
        ITEM_VALID_CHECK(argItem, "Game Item of name \"" << argStr << " Does Not Exist");
        out.push_back(argItem);
        return LocationError::NONE;
    case RequirementType::COUNT:
        // count expects a value count and a string mapping to an item
        if (args.size() != 2) return LocationError::INCORRECT_ARG_COUNT;
        countValue = args[0].get<int>();
        argStr = args[1].get<std::string>();
        argItem = nameToGameItem(argStr);
        ITEM_VALID_CHECK(argItem, "Game Item of name \"" << argStr << " Does Not Exist");
        out.push_back(countValue);
        out.push_back(argItem);
        return LocationError::NONE;
    case RequirementType::CAN_ACCESS:
        argLoc = nameToLocation(args[0].get<std::string>());
        LOCATION_VALID_CHECK(argLoc, "Location " << args[0].get<std::string>() << "does not exist");
        out.push_back(argLoc);
        return LocationError::NONE;
    case RequirementType::SETTING:
        // setting just expects a settings name
        argStr = args[0].get<std::string>();
        argSetting = nameToSetting(argStr);
        SETTING_VALID_CHECK(argSetting, "Setting " << argStr << " Does Not Exist.");
        out.push_back(argSetting);
        return LocationError::NONE;
    case RequirementType::MACRO:
        // macro just stores index into macro vector
        argStr = args[0].get<std::string>();
        if (macroNameMap.count(argStr) == 0)
        {
            lastError << "Macro of name " << argStr << " Does Not Exist.";
            return LocationError::MACRO_DOES_NOT_EXIST;
        }
        out.push_back(macroNameMap.at(argStr));
        return LocationError::NONE;
    case RequirementType::NONE:
    default:
        return LocationError::REQUIREMENT_TYPE_DOES_NOT_EXIST;
    }
    return LocationError::REQUIREMENT_TYPE_DOES_NOT_EXIST;
}

LocationManager::LocationError LocationManager::parseRequirement(const json &requirementsObject, Requirement& out)
{
    static std::unordered_map<std::string, RequirementType> typeMap = {
        {"or", RequirementType::OR},
        {"and", RequirementType::AND},
        {"not", RequirementType::NOT},
        {"has_item", RequirementType::HAS_ITEM},
        {"count", RequirementType::COUNT},
        {"can_access", RequirementType::CAN_ACCESS},
        {"setting", RequirementType::SETTING},
        {"macro", RequirementType::MACRO}
    };

    LocationError err = LocationError::NONE;

    FIELD_CHECK(requirementsObject, "type", LocationError::REQUIREMENT_MISISNG_KEY);
    const std::string& typeStr = requirementsObject.at("type").get<std::string>();
    if(typeMap.count(typeStr) == 0)
    {
        lastError << "Requirement Type " << typeStr << " Does Not Exist.";
        return LocationError::REQUIREMENT_TYPE_DOES_NOT_EXIST;
    }
    out.type = typeMap[typeStr];
    FIELD_CHECK(requirementsObject, "args", LocationError::REQUIREMENT_MISISNG_KEY);
    const auto& args = requirementsObject.at("args").get<std::vector<json>>();
    if ((err = parseElement(out.type, args, out.args)) != LocationError::NONE)
    {
        return err;
    }
    return LocationError::NONE;
}

LocationManager::LocationError LocationManager::parseMacro(const json& macroObject, Requirement& reqOut)
{
    LocationError err = LocationError::NONE;
    // readd prechecks?
    FIELD_CHECK(macroObject, "Expression", LocationError::MACRO_MISSING_KEY);
    if ((err = parseRequirement(macroObject.at("Expression"), reqOut)) != LocationError::NONE) return err;
    return LocationError::NONE;
}

LocationManager::LocationError LocationManager::loadMacros(const std::vector<json>& macroObjectList)
{
    std::string name;
    LocationError err = LocationError::NONE;
    uint32_t macroCount = 0;

    // first pass to get all macro names
    for (const auto& macroObject : macroObjectList)
    {
        FIELD_CHECK(macroObject, "Name", LocationError::MACRO_MISSING_KEY);
        name = macroObject.at("Name").get<std::string>();
        if (macroNameMap.count(name) > 0)
        {
            lastError << "Macro of name " << name << " has already been added";
            return LocationError::DUPLICATE_MACRO_NAME;
        }
        macroNameMap.emplace(name, macroCount++);
    }
    for (const auto& macroObject : macroObjectList)
    {
        macros.emplace_back();
        if ((err = parseMacro(macroObject, macros.back())) != LocationError::NONE)
        {
            lastError << " | Encountered parsing macro of name " << macroObject.at("Name").get<std::string>();
            return err;
        }
    }
    return LocationError::NONE;
}

LocationManager::LocationError LocationManager::loadLocation(const json& locationObject, Location& loadedLocation)
{
    // failure indicated by INVALID type for category
    // maybe change to Optional later if thats determined to work
    // on wii u
    OBJECT_CHECK(locationObject, locationObject.dump());
    FIELD_CHECK(locationObject, "Name", LocationError::LOCATION_MISSING_KEY);
    std::string locationName = locationObject.at("Name").get<std::string>();
    loadedLocation = nameToLocation(locationName);
    LOCATION_VALID_CHECK(loadedLocation, "Location of name \"" << locationName << "\" does not exist!");
    LocationEntry& newEntry = locationEntries[locationAsIndex(loadedLocation)];
    newEntry.location = loadedLocation;
    LocationError err = LocationError::NONE;
    FIELD_CHECK(locationObject, "Category", LocationError::LOCATION_MISSING_KEY);
    const auto& categories = locationObject.at("Category").get<std::vector<json>>();
    for (const auto& categoryName : categories)
    {
        const std::string& categoryNameStr = categoryName.get<std::string>();
        const auto& cat = nameToLocationCategory(categoryNameStr);
        if (cat == LocationCategory::INVALID)
        {
            lastError << "Encountered unknown location category " << categoryNameStr;
            lastError << " while parsing location " << locationName;
            return LocationError::INVALID_LOCATION_CATEGORY;
        }
        newEntry.categories.insert(cat);
    }
    FIELD_CHECK(locationObject, "Needs", LocationError::LOCATION_MISSING_KEY);
    if((err = parseRequirement(locationObject.at("Needs"), newEntry.requirement)) != LocationError::NONE)
    {
        lastError << "| Encountered parsing location " << locationName;
        return err;
    }
    FIELD_CHECK(locationObject, "Path", LocationError::LOCATION_MISSING_KEY);
    newEntry.method.filePath = locationObject.at("Path").get<std::string>();
    FIELD_CHECK(locationObject, "Type", LocationError::LOCATION_MISSING_KEY);
    const std::string& modificationType = locationObject.at("Type").get<std::string>();
    newEntry.method.type = nameToModificationType(modificationType);
    if (newEntry.method.type == LocationModificationType::INVALID)
    {
        lastError << "Error processing location " << locationName << ": ";
        lastError << "Modificaiton Type \"" << modificationType << "\" Does Not Exist";
        return LocationError::INVALID_MODIFICATION_TYPE;
    }
    FIELD_CHECK(locationObject, "Offsets", LocationError::LOCATION_MISSING_KEY);
    const auto& offsets = locationObject.at("Offsets").get<std::vector<json>>();
    for (const auto& offset_j : offsets)
    {
        if (offset_j.is_number())
        {
            newEntry.method.offsets.push_back(offset_j.get<uint32_t>());
        }
        else
        {
            unsigned long offsetValue = std::strtoul(offset_j.get<std::string>().c_str(), nullptr, 16);
            if (offsetValue == 0 || offsetValue == ULONG_MAX)
            {
                lastError << "Encountered an invalid offset for location " << locationName;
                return LocationError::INVALID_OFFSET_VALUE;
            }
            newEntry.method.offsets.push_back(offsetValue);
        }
    }
    FIELD_CHECK(locationObject, "OriginalItem", LocationError::LOCATION_MISSING_KEY);
    const std::string& itemName = locationObject.at("OriginalItem").get<std::string>();
    newEntry.originalItem = nameToGameItem(itemName);
    ITEM_VALID_CHECK(
        newEntry.originalItem, 
        "Error processing location " << locationName << ": Item of name " << itemName << " Does Not Exist."
    );
    return LocationError::NONE;
}

bool LocationManager::evaluateRequirement(const Requirement& req, const ItemSet& ownedItems, const Settings& settings)
{
    uint32_t expectedCount = 0;
    GameItem item;
    switch(req.type)
    {
    case RequirementType::OR:
        return std::any_of(
            req.args.begin(), 
            req.args.end(), 
            [&](const Requirement::Argument& arg){
                return evaluateRequirement(std::get<Requirement>(arg), ownedItems, settings);
            }
        );
    case RequirementType::AND:
        return std::all_of(
            req.args.begin(), 
            req.args.end(), 
            [&](const Requirement::Argument& arg){
                if (arg.index() != 2)
                {
                    return false;
                }
                return evaluateRequirement(std::get<Requirement>(arg), ownedItems, settings);
            }
        );
    case RequirementType::NOT:
        if (req.args[0].index() != 2)
        {
            return false;
        }
        return !evaluateRequirement(std::get<Requirement>(req.args[0]), ownedItems, settings);
    case RequirementType::HAS_ITEM:
        // we can expect ownedItems will contain entires for every item type
        if (req.args[0].index() != 3)
        {
            return false;
        }
        item = std::get<GameItem>(req.args[0]);
        if (item == GameItem::NOTHING) return true;
        return ownedItems.count(item) > 0;
    case RequirementType::COUNT:
        expectedCount = std::get<int>(req.args[0]);
        item = std::get<GameItem>(req.args[0]);
        if (item == GameItem::NOTHING) return true;
        return ownedItems.count(item) >= expectedCount;
    case RequirementType::CAN_ACCESS:
        return evaluateRequirement(locationEntries[locationAsIndex(std::get<Location>(req.args[0]))].requirement, ownedItems, settings);
    case RequirementType::SETTING:
        // TODO: assuming all boolean settings for now
        return settings.count(std::get<Option>(req.args[0])) > 0;
    case RequirementType::MACRO:
        return evaluateRequirement(macros[std::get<MacroIndex>(req.args[0])], ownedItems, settings);
    case RequirementType::NONE:
    default:
        // actually needs to be some error state?
        return false;
    }
    return false;
}

bool LocationManager::isAccessible(Location location, const ItemSet& ownedItems, const Settings& settings)
{
    const LocationEntry& entry = locationEntries[locationAsIndex(location)];
    return evaluateRequirement(entry.requirement, ownedItems, settings);
}

void LocationManager::getAccessibleLocations(const ItemSet& ownedItems, 
                                             const Settings& settings, 
                                             std::vector<Location>& accessibleLocations)
{
    for (uint32_t locationIdx = 0; locationIdx < LOCATION_COUNT; locationIdx++)
    {
        if (evaluateRequirement(locationEntries[locationIdx].requirement, ownedItems, settings))
        {
            accessibleLocations.push_back(indexAsLocation(locationIdx));
        }
    }
}

std::vector<Location> LocationManager::assumedSearch(ItemSet& ownedItems,
                                                     const Settings& settings)
{
    ItemSet newItems;
    std::vector<Location> reachable;
    do
    {
        reachable.clear();
        getAccessibleLocations(ownedItems, settings, reachable);
        std::unordered_set<GameItem> newItems;
        // newItems = R.GetItems()
        for (const auto& r : reachable)
        {
            auto currentItem = locationEntries[locationAsIndex(r)].currentItem;
            // invalid => not yet filled
            if (currentItem == GameItem::INVALID) continue;
            newItems.insert(currentItem);
        }
        // NewItems = NewItems - I
        for (const auto& o : ownedItems)
        {
            newItems.erase(o);
        }
        // I.Add(NewItems);
        for ( const auto& n : newItems)
        {
            ownedItems.insert(n);
        }
    } while(!newItems.empty());
    return reachable;
}

void LocationManager::assumedFill(const ItemSet& allItems, const Settings& settings)
{
    std::vector<GameItem> ownedItemsList;
    ItemSet ownedItemsSet = allItems;
    ItemSet availableItems{};
    std::vector<Location> nullReachable;

    for (const auto& item : allItems)
    {
        ownedItemsList.push_back(item);
    }

    // null out all locations
    for (size_t locIdx = 0; locIdx < locationEntries.size(); locIdx++)
    {
        locationEntries[locIdx].currentItem = GameItem::INVALID;
    }
    
    size_t nullLocationsRemaining = locationEntries.size();
    // put seed here later
    auto randomEngine = std::default_random_engine();
    std::shuffle(ownedItemsList.begin(), ownedItemsList.end(), randomEngine);
    while(nullLocationsRemaining > 0 && !ownedItemsList.empty())
    {
        auto removed = ownedItemsList.back();
        ownedItemsList.pop_back();
        ownedItemsSet.erase(removed);
        auto reachable = assumedSearch(ownedItemsSet, settings);
        nullReachable.clear();
        for (uint32_t idx = 0; idx < reachable.size(); idx++)
        {
            auto reachableLocation = reachable[idx];
            if (locationEntries[locationAsIndex(reachableLocation)].currentItem == GameItem::INVALID)
            {
                nullReachable.push_back(reachableLocation);
            }
        }
        auto rand = std::uniform_int_distribution<size_t>(0, nullReachable.size() - 1);
        auto randomNullLoc = nullReachable[rand(randomEngine)];
        locationEntries[locationAsIndex(randomNullLoc)].currentItem = removed;
        availableItems.insert(removed);
        nullLocationsRemaining--;
    }
}

const char* LocationManager::errorToName(LocationError err)
{
    switch(err)
    {
    case LocationError::NONE:
        return "NONE";
    case LocationError::DUPLICATE_MACRO_NAME:
        return "DUPLICATE_MACRO_NAME";
    case LocationError::MACRO_DOES_NOT_EXIST:
        return "MACRO_DOES_NOT_EXIST";
    case LocationError::REQUIREMENT_TYPE_DOES_NOT_EXIST:
        return "REQUIREMENT_TYPE_DOES_NOT_EXIST";
    case LocationError::GAME_ITEM_DOES_NOT_EXIST:
        return "GAME_ITEM_DOES_NOT_EXIST";
    case LocationError::LOCATION_DOES_NOT_EXIST:
        return "LOCATION_DOES_NOT_EXIST";
    case LocationError::SETTING_DOES_NOT_EXIST:
        return "SETTING_DOES_NOT_EXIST";
    case LocationError::INCORRECT_ARG_COUNT:
        return "INCORRECT_ARG_COUNT";
    case LocationError::EXPECTED_JSON_OBJECT:
        return "EXPECTED_JSON_OBJECT";
    case LocationError::LOCATION_MISSING_KEY:
        return "LOCATION_MISSING_KEY";
    case LocationError::MACRO_MISSING_KEY:
        return "MACRO_MISSING_KEY";
    case LocationError::REQUIREMENT_MISISNG_KEY:
        return "REQUIREMENT_MISISNG_KEY";
    case LocationError::INVALID_LOCATION_CATEGORY:
        return "INVALID_LOCATION_CATEGORY";
    case LocationError::INVALID_MODIFICATION_TYPE:
        return "INVALID_MODIFICATION_TYPE";
    case LocationError::INVALID_OFFSET_VALUE:
        return "INVALID_OFFSET_VALUE";
    case LocationError::INVALID_GAME_ITEM:
        return "INVALID_GAME_ITEM";
    default:
        return "UNKNOWN";
    }
}


std::string LocationManager::getLastErrorDetails()
{
    std::string out = lastError.str();
    lastError.str(std::string());
    lastError.clear();
    return out;
}
