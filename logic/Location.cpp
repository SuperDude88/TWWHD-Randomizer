
#include "Location.hpp"
#include "GameItem.hpp"
#include "World.hpp"
#include "../server/utility/stringUtil.hpp"
#include <string>
#include <unordered_map>

LocationCategory nameToLocationCategory(const std::string& name)
{
    static std::unordered_map<std::string, LocationCategory> categoryNameMap = {
        {"Misc", LocationCategory::Misc},
        {"Dungeon", LocationCategory::Dungeon},
        {"GreatFairy", LocationCategory::GreatFairy},
        {"IslandPuzzle", LocationCategory::IslandPuzzle},
        {"SpoilsTrading", LocationCategory::SpoilsTrading},
        {"Mail", LocationCategory::Mail},
        {"SavageLabyrinth", LocationCategory::SavageLabyrinth},
        {"FreeGift", LocationCategory::FreeGift},
        {"Minigame", LocationCategory::Minigame},
        {"BattleSquid", LocationCategory::BattleSquid},
        {"TingleChest", LocationCategory::TingleChest},
        {"PuzzleSecretCave", LocationCategory::PuzzleSecretCave},
        {"CombatSecretCave", LocationCategory::CombatSecretCave},
        {"Platform", LocationCategory::Platform},
        {"Raft", LocationCategory::Raft},
        {"EyeReefChests", LocationCategory::EyeReefChests},
        {"BigOcto", LocationCategory::BigOcto},
        {"Submarine", LocationCategory::Submarine},
        {"Gunboat", LocationCategory::Gunboat},
        {"LongSideQuest", LocationCategory::LongSideQuest},
        {"ShortSideQuest", LocationCategory::ShortSideQuest},
        {"ExpensivePurchase", LocationCategory::ExpensivePurchase},
        {"SunkenTreasure", LocationCategory::SunkenTreasure},
        {"Obscure", LocationCategory::Obscure},
        {"Junk", LocationCategory::Junk},
        {"Other", LocationCategory::Other},
        {"AlwaysProgression", LocationCategory::AlwaysProgression},
        {"HoHoHint", LocationCategory::HoHoHint},
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
        {LocationCategory::GreatFairy, "GreatFairy"},
        {LocationCategory::IslandPuzzle, "IslandPuzzle"},
        {LocationCategory::SpoilsTrading, "SpoilsTrading"},
        {LocationCategory::Mail, "Mail"},
        {LocationCategory::SavageLabyrinth, "SavageLabyrinth"},
        {LocationCategory::FreeGift, "FreeGift"},
        {LocationCategory::Minigame, "Minigame"},
        {LocationCategory::BattleSquid, "BattleSquid"},
        {LocationCategory::TingleChest, "TingleChest"},
        {LocationCategory::PuzzleSecretCave, "PuzzleSecretCave"},
        {LocationCategory::CombatSecretCave, "CombatSecretCave"},
        {LocationCategory::Platform, "Platform"},
        {LocationCategory::Raft, "Raft"},
        {LocationCategory::EyeReefChests, "EyeReefChests"},
        {LocationCategory::BigOcto, "BigOcto"},
        {LocationCategory::Submarine, "Submarine"},
        {LocationCategory::Gunboat, "Gunboat"},
        {LocationCategory::LongSideQuest, "LongSideQuest"},
        {LocationCategory::ShortSideQuest, "ShortSideQuest"},
        {LocationCategory::ExpensivePurchase, "ExpensivePurchase"},
        {LocationCategory::SunkenTreasure, "SunkenTreasure"},
        {LocationCategory::Obscure,  "Obscure"},
        {LocationCategory::Junk, "Junk"},
        {LocationCategory::Other, "Other"},
        {LocationCategory::AlwaysProgression, "AlwaysProgression"},
        {LocationCategory::HoHoHint, "HoHoHint"},
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
        {"DoNothing", LocationModificationType::DoNothing}
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
