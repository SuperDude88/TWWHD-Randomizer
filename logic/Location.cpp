
#include "Location.hpp"

#include <unordered_map>
#include <string>

#include <logic/World.hpp>
#include <filetypes/util/msbtMacros.hpp>

LocationCategory nameToLocationCategory(const std::string& name)
{
    static std::unordered_map<std::string, LocationCategory> categoryNameMap = {
        {"Misc", LocationCategory::Misc},
        {"Dungeon", LocationCategory::Dungeon},
        {"Great Fairy", LocationCategory::GreatFairy},
        {"Island Puzzle", LocationCategory::IslandPuzzle},
        {"Spoils Trading", LocationCategory::SpoilsTrading},
        {"Mail", LocationCategory::Mail},
        {"Savage Labyrinth", LocationCategory::SavageLabyrinth},
        {"Free Gift", LocationCategory::FreeGift},
        {"Minigame", LocationCategory::Minigame},
        {"Battle Squid", LocationCategory::BattleSquid},
        {"Tingle Chest", LocationCategory::TingleChest},
        {"Puzzle Secret Cave", LocationCategory::PuzzleSecretCave},
        {"Combat Secret Cave", LocationCategory::CombatSecretCave},
        {"Platform", LocationCategory::Platform},
        {"Raft", LocationCategory::Raft},
        {"Eye Reef Chests", LocationCategory::EyeReefChests},
        {"Big Octo", LocationCategory::BigOcto},
        {"Submarine", LocationCategory::Submarine},
        {"Gunboat", LocationCategory::Gunboat},
        {"Long Side Quest", LocationCategory::LongSideQuest},
        {"Short Side Quest", LocationCategory::ShortSideQuest},
        {"Expensive Purchase", LocationCategory::ExpensivePurchase},
        {"Sunken Treasure", LocationCategory::SunkenTreasure},
        {"Blue Chu Chu", LocationCategory::BlueChuChu},
        {"Dungeon Secret", LocationCategory::DungeonSecret},
        {"Obscure", LocationCategory::Obscure},
        {"Other", LocationCategory::Other},
        {"Always Progression", LocationCategory::AlwaysProgression},
        {"Ho Ho Hint", LocationCategory::HoHoHint},
    };

    if (!categoryNameMap.contains(name))
    {
        return LocationCategory::INVALID;
    }

    return categoryNameMap.at(name);
}

std::string locationCategoryToName(LocationCategory category)
{
    static std::unordered_map<LocationCategory, std::string> nameCategoryMap = {
        {LocationCategory::Misc, "Misc"},
        {LocationCategory::Dungeon, "Dungeon"},
        {LocationCategory::GreatFairy, "Great Fairy"},
        {LocationCategory::IslandPuzzle, "Island Puzzle"},
        {LocationCategory::SpoilsTrading, "Spoils Trading"},
        {LocationCategory::Mail, "Mail"},
        {LocationCategory::SavageLabyrinth, "Savage Labyrinth"},
        {LocationCategory::FreeGift, "Free Gift"},
        {LocationCategory::Minigame, "Minigame"},
        {LocationCategory::BattleSquid, "Battle Squid"},
        {LocationCategory::TingleChest, "Tingle Chest"},
        {LocationCategory::PuzzleSecretCave, "Puzzle Secret Cave"},
        {LocationCategory::CombatSecretCave, "Combat Secret Cave"},
        {LocationCategory::Platform, "Platform"},
        {LocationCategory::Raft, "Raft"},
        {LocationCategory::EyeReefChests, "Eye Reef Chests"},
        {LocationCategory::BigOcto, "Big Octo"},
        {LocationCategory::Submarine, "Submarine"},
        {LocationCategory::Gunboat, "Gunboat"},
        {LocationCategory::LongSideQuest, "Long Side Quest"},
        {LocationCategory::ShortSideQuest, "Short Side Quest"},
        {LocationCategory::ExpensivePurchase, "Expensive Purchase"},
        {LocationCategory::SunkenTreasure, "Sunken Treasure"},
        {LocationCategory::BlueChuChu, "Blue Chu Chu"},
        {LocationCategory::DungeonSecret,  "Dungeon Secret"},
        {LocationCategory::Obscure,  "Obscure"},
        {LocationCategory::Other, "Other"},
        {LocationCategory::AlwaysProgression, "Always Progression"},
        {LocationCategory::HoHoHint, "Ho Ho Hint"},
    };

    if (!nameCategoryMap.contains(category))
    {
        return "INVALID";
    }

    return nameCategoryMap.at(category);
}

LocationModificationType nameToModificationType(const std::string& name)
{
    static std::unordered_map<std::string, LocationModificationType> methodNameMap = {
        {"Chest", LocationModificationType::Chest},
        {"Actor", LocationModificationType::Actor},
        {"Boss", LocationModificationType::Boss},
        {"SCOB", LocationModificationType::SCOB},
        {"Event", LocationModificationType::Event},
        {"RPX", LocationModificationType::RPX},
        {"Custom_Symbol", LocationModificationType::Custom_Symbol},
        {"Do Nothing", LocationModificationType::DoNothing}
    };

    if (!methodNameMap.contains(name))
    {
        return LocationModificationType::INVALID;
    }

    return methodNameMap.at(name);
}

bool Location::operator<(const Location& rhs) const
{
    if (this->world->getWorldId() != rhs.world->getWorldId())
    {
        return this->world->getWorldId() < rhs.world->getWorldId();
    }

    return this->sortPriority < rhs.sortPriority;
}

std::string Location::getName() const
{
    if (names.contains("English"))
    {
        return names.at("English");
    }
    return "Names not loaded?";
}

// Calculates whether the current item can be barren given it's placement at this specific location in mind
bool Location::currentItemCanBeBarren() const
{
    if (currentItem.canBeInBarrenRegion())
    {
        return true;
    }

    // Get a pool of start items and all items from this location's logically required path locations
    ItemMultiSet logicallyRequiredItems = {};
    for (const auto& item : world->getStartingItems())
    {
        logicallyRequiredItems.insert(item);
    }
    for (const auto& pathLoc : this->pathLocations)
    {
        logicallyRequiredItems.insert(pathLoc->currentItem);
    }

    // Dummy ownedEvents for evaluation
    EventSet ownedEvents = {};

    // Get all the progression chain locations for this location's item
    auto chainLocations = currentItem.getChainLocations();
    for (auto& loc : currentItem.getChainLocations())
    {
        // If any of the item's chain locations can be obtained with the items logically necessary to get the item at this location,
        // then remove those locations from the list of chain locations, as this item will not help to obtain that specific chain location
        if (!loc->progression || evaluateRequirement(world, loc->computedRequirement, &logicallyRequiredItems, &ownedEvents))
        {
            chainLocations.erase(loc);
        }
    }

    // If any of the remaining chain locations have an item which can't be barren, then this location's item isn't barren either
    for (auto& location : chainLocations)
    {
        if (!location->currentItem.canBeInBarrenRegion())
        {
            return false;
        }
    }

    return true;
}

std::u16string Location::generateImportanceText(const std::string& language)
{
    auto& item = currentItem;

    std::u16string required = u"required";
    std::u16string possiblyRequired = u"possibly required";
    std::u16string notRequired = u"not required";

    if (language == "Spanish")
    {
        required = u"requerido";
        possiblyRequired = u"posiblemente requerido";
        notRequired = u"no requerido";
    }
    else if (language == "French")
    {
        // TODO
    }

    // Get all the progression chain locations for this location's item
    auto chainLocations = item.getChainLocations();
    for (auto loc : item.getChainLocations())
    {
        if (!loc->progression)
        {
            chainLocations.erase(loc);
        }
    }

    // If this item was always junk, or has no progression chain locations, or if hint importance is off
    // then we won't generate any hint importance text for it
    if (item.wasAlwaysJunkItem() || chainLocations.empty() || !world->getSettings().hint_importance)
    {
        return u"";
    }

    // If this item is on the path to Ganondorf, then it is required
    auto& requiredLocations = world->locationTable["Ganon's Tower - Defeat Ganondorf"]->pathLocations;
    if (elementInPool(this, requiredLocations))
    {
        return u" (" + TEXT_COLOR_GREEN + required + TEXT_COLOR_DEFAULT + u")";
    }

    // If this item can be in a barren region, then it's not required
    if (currentItemCanBeBarren())
    {
        return u" (" + TEXT_COLOR_GRAY + notRequired + TEXT_COLOR_DEFAULT + u")";
    }

    // If the item doesn't fall into required or not required, then it's possibly required
    return u" (" + TEXT_COLOR_YELLOW + possiblyRequired + TEXT_COLOR_DEFAULT + u")";
}
