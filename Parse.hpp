#define Parse_H
#pragma once

#include <vector>
#include "Structs.hpp"

std::vector<Location> ParseLocations(std::string LocationsPath);

std::vector<Macro> ParseMacros(std::string MacrosPath);