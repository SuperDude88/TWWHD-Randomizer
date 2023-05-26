
#pragma once

#include <cstdint>
#include <string>

std::string roomIndexToIslandName(const uint8_t& startingIslandRoomIndex);
uint8_t islandNameToRoomIndex(const std::string& islandArea);
