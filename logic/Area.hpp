
#pragma once

#include <string>

void storeNewAreaPrettyName(const std::string& area, const std::string& prettyName);
std::string areaToPrettyName(const std::string& area);
std::string prettyNameToArea(const std::string& prettyName);
std::string roomIndexToIslandName(const uint8_t& startingIslandRoomIndex);
uint8_t islandNameToRoomIndex(const std::string& islandArea);
