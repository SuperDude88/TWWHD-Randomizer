#define Structs_H
#pragma once

#include <map>
#include <cstdint>
#include <string>
#include <vector>
#include "json.hpp"
#include <unordered_set>

struct ACTR {
	char name[8];
	uint32_t params;
	float x_pos;
	float y_pos;
	float z_pos;
	uint16_t aux_params_1;
	uint16_t y_rot;
	uint16_t aux_params_2;
	uint16_t enemy_number;
};

struct SCOB {
	char name[8];
	uint32_t params;
	float x_pos;
	float y_pos;
	float z_pos;
	uint16_t aux_params_1;
	uint16_t y_rot;
	uint16_t aux_params_2;
	uint16_t enemy_number;
	uint8_t scale_x;
	uint8_t scale_y;
	uint8_t scale_z;
	uint8_t padding;
};

struct Location {
	std::string Name;
	std::string Item;
	std::vector<std::string> Category;
	nlohmann::json Needs;
	std::string Path;
	std::string Type;
	std::vector<std::string> Offsets;
	std::string Extra;
};

struct LocationLists {
	std::vector<Location> ProgressLocations;
	std::vector<Location> NonprogressLocations;
	std::vector<Location> PlacedLocations;
	std::vector<Location> UnplacedLocations;
};