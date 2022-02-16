
#include "Generate.hpp"
#include "World.hpp"
#include "ItemPool.hpp"
#include "Fill.hpp"
#include "SpoilerLog.hpp"
#include "Random.hpp"
#include "Debug.hpp"
#include <string>
#include <unordered_set>
#include <unordered_map>
#include <fstream>
#include <iostream>

int generateWorlds(WorldPool& worlds, std::vector<Settings>& settingsVector, const int seed)
{

  Random_Init(seed);

  // Build worlds on a per-world basis incase we ever support different world graphs
  // per player
  std::cout << "Building World" << (worlds.size() > 1 ? "s" : "") << std::endl;
  for (size_t i = 0; i < worlds.size(); i++)
  {
      debugLog("Building World " + std::to_string(i));
      worlds[i].setWorldId(i);
      worlds[i].setSettings(settingsVector[i]);
      if (worlds[i].loadWorld("../world.json", "../Macros.json", "../location_data.json"))
      {
          return 1;
      }
      worlds[i].determineChartMappings();
      worlds[i].determineProgressionLocations();
      worlds[i].determineRaceModeDungeons();
      worlds[i].setItemPools();
      // worlds[i].randomizeEntrances()
  }

  FillError fillError = fill(worlds);
  if (fillError == FillError::NONE) {
      std::cout << "Fill Successful" << std::endl;
  } else {
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

  // Dump world graphs if debugging
  #ifdef ENABLE_DEBUG
      for (World& world : worlds) {
          world.dumpWorldGraph("World" + std::to_string(world.getWorldId()));
      }
  #endif

  std::cout << "Generating Playthrough" << std::endl;
  generatePlaythrough(worlds);

  return 0;
}
