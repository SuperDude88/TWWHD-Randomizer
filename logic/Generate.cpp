
#include "Generate.hpp"
#include "World.hpp"
#include "ItemPool.hpp"
#include "Fill.hpp"
#include "SpoilerLog.hpp"
#include "Random.hpp"
#include "../server/command/Log.hpp"
#include "EntranceShuffle.hpp"
#include <string>
#include <unordered_set>
#include <unordered_map>
#include <fstream>
#include <iostream>
#include "../server/command/Log.hpp"

int generateWorlds(WorldPool& worlds, std::vector<Settings>& settingsVector, const int seed)
{

  Random_Init(seed);

  // Build worlds on a per-world basis incase we ever support different world graphs
  // per player
  std::cout << "Building World" << (worlds.size() > 1 ? "s" : "") << std::endl;
  int buildRetryCount = 10;
  EntranceShuffleError entranceErr;
  while (buildRetryCount > 0)
  {
      for (size_t i = 0; i < worlds.size(); i++)
      {
          DebugLog::getInstance().log("Building World " + std::to_string(i));
          worlds[i] = World();
          worlds[i].setWorldId(i);
          worlds[i].setSettings(settingsVector[i]);
          if (worlds[i].loadWorld("./logic/data/world.yaml", "./logic/data/macros.yaml", "./logic/data/location_data.yaml"))
          {
              return 1;
          }
          worlds[i].determineChartMappings();
          worlds[i].determineProgressionLocations();
          worlds[i].determineRaceModeDungeons();
          worlds[i].setItemPools();
      }

      // Randomize entrances before placing items
      DebugLog::getInstance().log("Randomizing Entrances");
      entranceErr = randomizeEntrances(worlds);
      if (entranceErr != EntranceShuffleError::NONE)
      {
          DebugLog::getInstance().log("Entrance randomization unsuccessful. Error Code: " + errorToName(entranceErr));
          buildRetryCount--;
          continue;
      }
      break;
  }

  if (buildRetryCount == 0)
  {
      std::cout << "Build retry count exceeded. Error: " << errorToName(entranceErr) << std::endl;
      return 1;
  }

  // Retry the main fill algorithm a couple times incase it completely fails.
  int totalFillAttempts = 5;
  FillError fillError;
  std::cout << "Filling World" << (worlds.size() > 1 ? "s" : "") << std::endl;
  while (totalFillAttempts > 0)
  {
      totalFillAttempts--;
      fillError = fill(worlds);
      if (fillError == FillError::NONE || fillError == FillError::NOT_ENOUGH_PROGRESSION_LOCATIONS) {
          break;
      }
      DebugLog::getInstance().log("Fill attempt failed completely. Will retry " + std::to_string(totalFillAttempts) + " more times");
      clearWorlds(worlds);
  }

  if (fillError != FillError::NONE)
  {
      std::cout << "Fill Unsuccessful. Error Code: " << errorToName(fillError) << std::endl;
      #ifdef ENABLE_DEBUG
          if (fillError == FillError::GAME_NOT_BEATABLE)
          {
              generatePlaythrough(worlds);
              generateSpoilerLog(worlds);
          }
          for (World& world : worlds) {
              world.dumpWorldGraph("World" + std::to_string(world.getWorldId()));
          }
      #endif
      return 1;
  }

  std::cout << "Generating Playthrough" << std::endl;
  generatePlaythrough(worlds);

  return 0;
}
