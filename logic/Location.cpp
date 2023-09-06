
#include "Location.hpp"

#include <unordered_map>
#include <string>

#include <logic/GameItem.hpp>
#include <logic/World.hpp>
#include <utility/string.hpp>

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
        {"Obscure", LocationCategory::Obscure},
        {"Junk", LocationCategory::Junk},
        {"Other", LocationCategory::Other},
        {"Always Progression", LocationCategory::AlwaysProgression},
        {"Ho Ho Hint", LocationCategory::HoHoHint},
        {"Plandomizer Progression", LocationCategory::PlandomizerProgression},
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
        {LocationCategory::Obscure,  "Obscure"},
        {LocationCategory::Junk, "Junk"},
        {LocationCategory::Other, "Other"},
        {LocationCategory::AlwaysProgression, "Always Progression"},
        {LocationCategory::HoHoHint, "Ho Ho Hint"},
        {LocationCategory::PlandomizerProgression, "Plandomizer Progression"},
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
    return names.at("English");
}
