
#include "Generate.hpp"
#include "World.hpp"
#include "ItemPool.hpp"
#include "Fill.hpp"
#include "SpoilerLog.hpp"
#include "Hints.hpp"
#include "../seedgen/random.hpp"
#include "../server/command/Log.hpp"
#include "../server/utility/platform.hpp"
#include "EntranceShuffle.hpp"
#include <string>
#include <unordered_set>
#include <unordered_map>
#include <fstream>
#include <filesystem>
#include <chrono>

int generateWorlds(WorldPool& worlds, std::vector<Settings>& settingsVector)
{
  #ifdef ENABLE_TIMING
      auto start = std::chrono::high_resolution_clock::now();
  #endif
  // Build worlds on a per-world basis incase we ever support different world graphs
  // per player
  #ifndef MASS_TESTING
      Utility::platformLog(std::string("Building World") + (worlds.size() > 1 ? "s\n" : "\n"));
  #endif
  int buildRetryCount = 10;
  EntranceShuffleError entranceErr = EntranceShuffleError::NONE;
  while (buildRetryCount > 0)
  {
      for (size_t i = 0; i < worlds.size(); i++)
      {
          LOG_TO_DEBUG("Building World " + std::to_string(i));
          worlds[i] = World();
          worlds[i].setWorldId(i);
          worlds[i].setSettings(settingsVector[i]);
          worlds[i].resolveRandomSettings();
          if (worlds[i].loadWorld("./logic/data/world.yaml", "./logic/data/macros.yaml", "./logic/data/location_data.yaml", "./logic/data/item_data.yaml"))
          {
              return 1;
          }
          worlds[i].determineChartMappings();
          worlds[i].determineProgressionLocations();
          if (worlds[i].setItemPools() != World::WorldLoadingError::NONE)
          {
              ErrorLog::getInstance().log(worlds[i].getLastErrorDetails());
              return 1;
          }
          if (worlds[i].getSettings().plandomizer)
          {
              if (worlds[i].loadPlandomizer() != World::WorldLoadingError::NONE)
              {
                  Utility::platformLog(worlds[i].getLastErrorDetails());
                  return 1;
              }
          }
          if (worlds[i].determineRaceModeDungeons() != World::WorldLoadingError::NONE)
          {
              return 1;
          }
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
      break;
  }

  if (buildRetryCount == 0)
  {
      ErrorLog::getInstance().log("Build retry count exceeded. Error: " + errorToName(entranceErr));
      return 1;
  }

  // Retry the main fill algorithm a couple times incase it completely fails.
  int totalFillAttempts = 5;
  FillError fillError = FillError::NONE;
  #ifndef MASS_TESTING
      Utility::platformLog(std::string("Filling World") + (worlds.size() > 1 ? "s\n" : "\n"));
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
  }

  if (fillError != FillError::NONE)
  {
      ErrorLog::getInstance().log(std::string("Fill Unsuccessful. Error Code: ") + errorToName(fillError));
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

  #ifndef MASS_TESTING
      Utility::platformLog("Generating Playthrough\n");
  #endif
  generatePlaythrough(worlds);

  #ifndef MASS_TESTING
      Utility::platformLog("Generating Hints\n");
  #endif
  auto hintError = generateHints(worlds);
  if (hintError != HintError::NONE)
  {
      return 1;
  }

  #ifdef ENABLE_TIMING
      auto stop = std::chrono::high_resolution_clock::now();
      auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
      auto seconds = static_cast<double>(duration.count()) / 1000000.0;
      Utility::platformLog(std::string("Building and Filling took ") + std::to_string(seconds) + " seconds\n");
  #endif
  return 0;
}
