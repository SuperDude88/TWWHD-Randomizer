
#include "Generate.hpp"

#include <string>
#include <unordered_set>
#include <unordered_map>
#include <fstream>
#include <filesystem>
#include <chrono>

#include <logic/Plandomizer.hpp>
#include <logic/World.hpp>
#include <logic/ItemPool.hpp>
#include <logic/Fill.hpp>
#include <logic/SpoilerLog.hpp>
#include <logic/Hints.hpp>
#include <logic/EntranceShuffle.hpp>
#include <seedgen/random.hpp>
#include <command/Log.hpp>
#include <utility/platform.hpp>

#include <gui/update_dialog_header.hpp>

#define WORLD_LOADING_ERROR_CHECK(err) if (err != World::WorldLoadingError::NONE) {ErrorLog::getInstance().log(worlds[i].getLastErrorDetails()); return 1;}

int generateWorlds(WorldPool& worlds, std::vector<Settings>& settingsVector)
{
  #ifdef ENABLE_TIMING
      ScopedTimer<std::chrono::high_resolution_clock, "Building and Filling took "> timer;
  #endif
  // Build worlds on a per-world basis incase we ever support different world graphs
  // per player
  #ifndef MASS_TESTING
      Utility::platformLog(std::string("Building World") + (worlds.size() > 1 ? "s\n" : "\n"));
      UPDATE_DIALOG_LABEL("Building World");
  #endif
  int buildRetryCount = 10;
  EntranceShuffleError entranceErr = EntranceShuffleError::NONE;
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
      std::string plandoFilepath = "";
      bool usePlando = false;
      for (auto& world : worlds)
      {
          if (world.getSettings().plandomizer)
          {
             usePlando = true;
             #ifdef DEVKITPRO
                 plandoFilepath = APP_SAVE_PATH "plandomizer.yaml"; //can't bundle in the romfs, put it in the save directory instead
             #else
                 plandoFilepath = world.getSettings().plandomizerFile;
             #endif
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
      for (size_t i = 0; i < worlds.size(); i++)
      {
          worlds[i].resolveRandomSettings();
          if (worlds[i].loadWorld(DATA_PATH "logic/data/world.yaml", DATA_PATH "logic/data/macros.yaml", DATA_PATH "logic/data/location_data.yaml", DATA_PATH "logic/data/item_data.yaml", DATA_PATH "logic/data/area_names.yaml"))
          {
              return 1;
          }
          worlds[i].determineChartMappings();
          WORLD_LOADING_ERROR_CHECK(worlds[i].determineProgressionLocations());
          WORLD_LOADING_ERROR_CHECK(worlds[i].setItemPools());
          WORLD_LOADING_ERROR_CHECK(worlds[i].determineRaceModeDungeons());
      }

      // Vanilla items must be placed before plandomizer items so that players
      // don't plandomize a different item into a known vanilla location
      placeVanillaItems(worlds);

      // Process Plandomized locations now after building each world
      for (size_t i = 0; i < worlds.size(); i++)
      {
          WORLD_LOADING_ERROR_CHECK(worlds[i].processPlandomizerLocations(worlds));
      }

      // Randomize entrances before placing items
      LOG_TO_DEBUG("Randomizing Entrances");
      entranceErr = randomizeEntrances(worlds);
      if (entranceErr != EntranceShuffleError::NONE)
      {
          LOG_TO_DEBUG("Entrance randomization unsuccessful. Error Code: " + errorToName(entranceErr));
          if (entranceErr == EntranceShuffleError::BAD_ENTRANCE_SHUFFLE_TABLE_ENTRY || entranceErr == EntranceShuffleError::BAD_LINKS_SPAWN || entranceErr == EntranceShuffleError::PLANDOMIZER_ERROR)
          {
              ErrorLog::getInstance().log("Error Code: " + errorToName(entranceErr));
              return 1;
          }
          buildRetryCount--;
          continue;
      }

      if (buildRetryCount == 0 && entranceErr != EntranceShuffleError::NONE)
      {
          ErrorLog::getInstance().log("Build retry count exceeded. Error: " + errorToName(entranceErr));
          return 1;
      }

      // Retry the main fill algorithm a couple times incase it completely fails.
      int totalFillAttempts = 5;
      FillError fillError = FillError::NONE;
      #ifndef MASS_TESTING
          std::string message = std::string("Filling World") + (worlds.size() > 1 ? "s" : "") + (fillAttemptCount++ > 0 ? " (Attempt " + std::to_string(fillAttemptCount) + ")" : "");
          Utility::platformLog(message + "\n");
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
          LOG_TO_DEBUG("Fill attempt failed completely. Will retry " + std::to_string(totalFillAttempts) + " more times");
          clearWorlds(worlds);
          if (totalFillAttempts == 0)
          {
              ErrorLog::getInstance().log("Ran out of retries on fill algorithm");
          }
      }

      // If we don't have enough locations available, but one of the worlds has race mode enabled,
      // then try rebuilding the world with different dungeons to increase the number of locations
      if (fillError == FillError::NOT_ENOUGH_PROGRESSION_LOCATIONS && std::any_of(worlds.begin(), worlds.end(), [](World& world){return world.getSettings().race_mode && world.getSettings().progression_dungeons;}))
      {
          continue;
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
          return 1;
      }

      break;
  }

  #ifndef MASS_TESTING
      Utility::platformLog("Generating Playthrough\n");
      UPDATE_DIALOG_VALUE(15);
      UPDATE_DIALOG_LABEL("Generating Playthrough");
  #endif
  generatePlaythrough(worlds);

  #ifndef MASS_TESTING
      Utility::platformLog("Generating Hints\n");
      UPDATE_DIALOG_VALUE(20);
      UPDATE_DIALOG_LABEL("Generating Hints");
  #endif
  auto hintError = generateHints(worlds);
  if (hintError != HintError::NONE)
  {
      return 1;
  }

  return 0;
}
