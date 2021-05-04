#define Location_H
#pragma once

#include "Structs.hpp"
#include <vector>

LocationLists FindPossibleProgressLocations(std::vector<Location> Locations, std::vector<std::string> settings);

int WriteLocations(LocationLists list, std::fstream fptr);