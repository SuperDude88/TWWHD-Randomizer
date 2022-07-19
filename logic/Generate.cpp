
#include "Generate.hpp"
#include "World.hpp"
#include "ItemPool.hpp"
#include "Fill.hpp"
#include "SpoilerLog.hpp"
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
          #ifdef ENABLE_DEBUG
              DebugLog::getInstance().log("Building World " + std::to_string(i));
          #endif
          worlds[i] = World();
          worlds[i].setWorldId(i);
          worlds[i].setSettings(settingsVector[i]);
          worlds[i].resolveRandomSettings();
          if (worlds[i].loadWorld("./logic/data/world.yaml", "./logic/data/macros.yaml", "./logic/data/location_data.yaml"))
          {
              return 1;
          }
          worlds[i].determineChartMappings();
          worlds[i].determineProgressionLocations();
          if (worlds[i].determineRaceModeDungeons() != World::WorldLoadingError::NONE)
          {
              return 1;
          }
          worlds[i].setItemPools();
      }

      // Randomize entrances before placing items
      #ifdef ENABLE_DEBUG
          DebugLog::getInstance().log("Randomizing Entrances");
      #endif
      entranceErr = randomizeEntrances(worlds);
      if (entranceErr != EntranceShuffleError::NONE)
      {
          #ifdef ENABLE_DEBUG
              DebugLog::getInstance().log("Entrance randomization unsuccessful. Error Code: " + errorToName(entranceErr));
          #endif
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
      #ifdef ENABLE_DEBUG
          DebugLog::getInstance().log("Fill attempt failed completely. Will retry " + std::to_string(totalFillAttempts) + " more times");
      #endif
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

  #ifdef ENABLE_TIMING
      auto stop = std::chrono::high_resolution_clock::now();
      auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
      auto seconds = static_cast<double>(duration.count()) / 1000000.0;
      Utility::platformLog(std::string("Generating and Filling worlds took ") + std::to_string(seconds) + " seconds\n");
  #endif
  return 0;
}
