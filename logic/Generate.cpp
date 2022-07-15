
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

int generateWorlds(WorldPool& worlds, std::vector<Settings>& settingsVector)
{
  // Build worlds on a per-world basis incase we ever support different world graphs
  // per player
  Utility::platformLog(std::string("Building World\n") + (worlds.size() > 1 ? "s" : ""));
  int buildRetryCount = 10;
  EntranceShuffleError entranceErr = EntranceShuffleError::NONE;
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
          World::WorldLoadingError err = World::WorldLoadingError::NONE;
          err = worlds[i].determineRaceModeDungeons();
          if (err != World::WorldLoadingError::NONE)
          {
              return 1;
          }
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
      ErrorLog::getInstance().log("Build retry count exceeded. Error: " + errorToName(entranceErr));
      return 1;
  }

  // Retry the main fill algorithm a couple times incase it completely fails.
  int totalFillAttempts = 5;
  FillError fillError = FillError::NONE;
  Utility::platformLog(std::string("Filling World\n") + (worlds.size() > 1 ? "s" : ""));
  while (totalFillAttempts > 0)
  {
      totalFillAttempts--;
      fillError = fill(worlds);
      if (fillError == FillError::NONE || fillError == FillError::NOT_ENOUGH_PROGRESSION_LOCATIONS || fillError == FillError::PLANDOMIZER_ERROR) {
          break;
      }
      DebugLog::getInstance().log("Fill attempt failed completely. Will retry " + std::to_string(totalFillAttempts) + " more times");
      clearWorlds(worlds);
  }

  if (fillError != FillError::NONE)
  {
      ErrorLog::getInstance().log(std::string("Fill Unsuccessful. Error Code: ") + errorToName(fillError));
      #ifdef ENABLE_DEBUG
          if (fillError == FillError::GAME_NOT_BEATABLE)
          {
              generatePlaythrough(worlds);
              //generateSpoilerLog(worlds, seed);
          }
          for (World& world : worlds) {
              world.dumpWorldGraph("World" + std::to_string(world.getWorldId()));
          }
      #endif
      return 1;
  }

  Utility::platformLog("Generating Playthrough\n");
  generatePlaythrough(worlds);

  return 0;
}
