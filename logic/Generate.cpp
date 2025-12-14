
#include "Generate.hpp"

#include <string>

#include <logic/Plandomizer.hpp>
#include <logic/World.hpp>
#include <logic/Fill.hpp>
#include <logic/Search.hpp>
#include <logic/SpoilerLog.hpp>
#include <logic/Hints.hpp>
#include <logic/EntranceShuffle.hpp>
#include <seedgen/random.hpp>
#include <command/Log.hpp>
#include <utility/path.hpp>
#include <utility/platform.hpp>
#include <utility/time.hpp>

#include <gui/desktop/update_dialog_header.hpp>

#define WORLD_LOADING_ERROR_CHECK(err) if (err != World::WorldLoadingError::NONE) {ErrorLog::getInstance().log(world.getLastErrorDetails()); return 1;}

int generateWorlds(WorldPool& worlds, std::vector<Settings>& settingsVector)
{
  #ifdef ENABLE_TIMING
      ScopedTimer<"Building and Filling took ", std::chrono::milliseconds> timer;
  #endif
  // Build worlds on a per-world basis incase we ever support different world graphs
  // per player
  #ifndef LOGIC_TESTS
      Utility::platformLog(std::string("Building World") + (worlds.size() > 1 ? "s" : ""));
      UPDATE_DIALOG_LABEL("Building World");
  #endif
  int buildRetryCount = 20;
  int fillAttemptCount = 0;
  while (buildRetryCount > 0)
  {
      for (size_t i = 0; i < worlds.size(); i++)
      {
          LOG_TO_DEBUG("Building World " + std::to_string(i));
          worlds[i] = World();
          worlds[i].setWorldId(i);
          worlds[i].setSettings(settingsVector[i]);
      }

      // Load plando data if any worlds have plandomizer enabled
      // Choose the filepath from the first world which has it enabled
      fspath plandoFilepath = "";
      bool usePlando = false;
      for (auto& world : worlds)
      {
          if (world.getSettings().plandomizer)
          {
              usePlando = true;
              plandoFilepath = world.getSettings().plandomizerFile;
              break;
          }
      }

      if (usePlando)
      {
          std::vector<Plandomizer> plandos(worlds.size());
          PlandomizerError err = loadPlandomizer(plandoFilepath, plandos, worlds.size());
          if (err != PlandomizerError::NONE)
          {
              return 1;
          }
          for (size_t i = 0; i < worlds.size(); i++)
          {
              worlds[i].plandomizer = plandos[i];
          }
      }

      // Once plandomizer data has been loaded, continue with building each world
      for (auto& world : worlds)
      {
          world.resolveRandomSettings();
          if (world.loadWorld(Utility::get_data_path() / "logic/world.yaml", Utility::get_data_path() / "logic/macros.yaml", Utility::get_data_path() / "logic/location_data.yaml", Utility::get_data_path() / "logic/item_data.yaml", Utility::get_data_path() / "logic/area_names.yaml"))
          {
              return 1;
          }
          world.determineChartMappings();
          WORLD_LOADING_ERROR_CHECK(world.determineProgressionLocations());
          WORLD_LOADING_ERROR_CHECK(world.setItemPools());
      }

      // Vanilla items must be placed before plandomizer items so that players
      // don't plandomize a different item into a known vanilla location
      placeVanillaItems(worlds);

      // Process Plandomized locations now after building each world
      for (auto& world : worlds)
      {
          WORLD_LOADING_ERROR_CHECK(world.processPlandomizerLocations(worlds));
      }

      // If race mode is not enabled in any world, then the number of progression
      // locations is already fixed and we can check if we have enough locations now
      if (!ANY_WORLD_HAS_RACE_MODE(worlds) && validateEnoughLocations(worlds) == FillError::NOT_ENOUGH_PROGRESSION_LOCATIONS) {
          return 1;
      }

      // If the user(s) selected "Overworld" as the placement option for small/big keys
      // but didn't enable enough overworld locations, then don't continue
      for (auto& world : worlds)
      {
          auto& settings = world.getSettings();
          if (settings.progression_dungeons != ProgressionDungeons::Disabled)
          {
              size_t neededOverworldLocations = 0;
              const size_t numOverworldLocations = world.getNumOverworldProgressionLocations();
              if (settings.dungeon_small_keys == PlacementOption::Overworld)
              {
                  neededOverworldLocations += filterFromPool(world.getItemPoolReference(), [](const Item& item){return Utility::Str::contains(item.getName(), "Small Key");}).size();
              }
              if (settings.dungeon_big_keys == PlacementOption::Overworld)
              {
                  neededOverworldLocations += filterFromPool(world.getItemPoolReference(), [](const Item& item){return Utility::Str::contains(item.getName(), "Big Key");}).size();
              }

              if (numOverworldLocations < neededOverworldLocations)
              {
                  ErrorLog::getInstance().log("Total Overworld Progression Locations: " + std::to_string(numOverworldLocations));
                  ErrorLog::getInstance().log("Number of Overworld Progress Items: " + std::to_string(neededOverworldLocations));
                  ErrorLog::getInstance().log("Please select more locations for overworld small/big keys to appear.");
                  return 1;
              }
          }
      }

      // Randomize entrances before placing items
      LOG_TO_DEBUG("Randomizing Entrances");
      if (const EntranceShuffleError entranceErr = randomizeEntrances(worlds); entranceErr != EntranceShuffleError::NONE)
      {
          LOG_TO_DEBUG("Entrance randomization unsuccessful. Error Code: " + errorToName(entranceErr));
          // Return early for errors which can't be resolved by re-shuffling
          if (entranceErr == EntranceShuffleError::BAD_ENTRANCE_SHUFFLE_TABLE_ENTRY || entranceErr == EntranceShuffleError::BAD_LINKS_SPAWN || entranceErr == EntranceShuffleError::PLANDOMIZER_ERROR)
          {
              ErrorLog::getInstance().log("Entrance shuffle error: " + errorToName(entranceErr));
              return 1;
          }

          buildRetryCount--;
          if (buildRetryCount == 0)
          {
              ErrorLog::getInstance().log("Build retry count exceeded. Error: " + errorToName(entranceErr));
              if (entranceErr == EntranceShuffleError::NOT_ENOUGH_SPHERE_ZERO_LOCATIONS)
              {
                  ErrorLog::getInstance().log("Please enable more sphere 0 locations or start with more items");
              }
              return 1;
          }

          continue;
      }

      // Now that all entrances have been randomized, we can flatten the world's
      // logic requirements for some useful operations later on
      // Determine race mode dungeons after entrance randomizer to ensure we pick
      // dungeons which can be properly reached depending on any entrance rando settings
      for (auto& world : worlds)
      {
          world.flattenLogicRequirements();
          WORLD_LOADING_ERROR_CHECK(world.setDungeonLocations(worlds));
          WORLD_LOADING_ERROR_CHECK(world.determineRaceModeDungeons(worlds));
      }

      // Retry the main fill algorithm a couple times incase it completely fails.
      int totalFillAttempts = 10;
      FillError fillError = FillError::NONE;
      #ifndef LOGIC_TESTS
          const std::string message = std::string("Filling World") + (worlds.size() > 1 ? "s" : "") + (fillAttemptCount++ > 0 ? " (Attempt " + std::to_string(fillAttemptCount) + ")" : "");
          Utility::platformLog(message);
          UPDATE_DIALOG_VALUE(10);
          UPDATE_DIALOG_LABEL(message.c_str());
      #endif
      while (totalFillAttempts > 0)
      {
          totalFillAttempts--;
          fillError = fill(worlds);
          if (fillError == FillError::NONE || fillError == FillError::NOT_ENOUGH_PROGRESSION_LOCATIONS || fillError == FillError::PLANDOMIZER_ERROR) {
              break;
          }
          LOG_TO_DEBUG("Fill attempt failed completely. Error: " + errorToName(fillError) + ". Will retry " + std::to_string(totalFillAttempts) + " more times");
          clearWorlds(worlds);
      }

      if (fillError != FillError::NONE)
      {
          #ifdef ENABLE_DEBUG
              if (fillError == FillError::GAME_NOT_BEATABLE)
              {
                  generatePlaythrough(worlds);
              }
              for (World& world : worlds) {
                  world.dumpWorldGraph("World" + std::to_string(world.getWorldId()));
              }
          #endif

          buildRetryCount--;
          if (buildRetryCount == 0)
          {
              ErrorLog::getInstance().log("Ran out of retries on fill algorithm.");
              return 1;
          }

          continue;
      }

      break;
  }

  #ifndef LOGIC_TESTS
      Utility::platformLog("Generating Playthrough");
      UPDATE_DIALOG_VALUE(15);
      UPDATE_DIALOG_LABEL("Generating Playthrough");
  #endif
  generatePlaythrough(worlds);

  #ifndef LOGIC_TESTS
      Utility::platformLog("Generating Hints");
      UPDATE_DIALOG_VALUE(20);
      UPDATE_DIALOG_LABEL("Generating Hints");
  #endif
  if (const HintError err = generateHints(worlds); err != HintError::NONE)
  {
      return 1;
  }

  return 0;
}
