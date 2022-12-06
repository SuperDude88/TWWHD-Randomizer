//A binary format used to store information for the game's charts

#pragma once

#include <fstream>
#include <string>
#include <vector>
#include <array>

#include <logic/GameItem.hpp>
#include <filetypes/baseFiletype.hpp>



enum struct [[nodiscard]] ChartError
{
	NONE = 0,
	COULD_NOT_OPEN,
	INVALID_NUMBER,
	REACHED_EOF,
	UNKNOWN,
	COUNT
};

class ChartPos {
public:
	uint16_t tex_x_offset = 0;
	uint16_t tex_y_offset = 0;
	uint16_t salvage_x_pos = 0;
	uint16_t salvage_y_pos = 0;

	ChartError read(std::istream& in);
	void save_changes(std::ostream& out);
};

class Chart {
private:
	uint8_t island_number = 0;
	GameItem item_name = GameItem::INVALID;

public:		
	uint8_t texture_id = 0;
	uint8_t owned_chart_index_plus_1 = 0;
	uint8_t number = 0;
	uint8_t type = 0;

	int8_t sector_x = 0;
	int8_t sector_y = 0;

	std::array<ChartPos, 4> possible_positions = {};

	ChartError read(std::istream& in);
	void save_changes(std::ostream& out);

	uint8_t getIslandNumber() const;
	ChartError setIslandNumber(const uint8_t num);
	GameItem getItem() const;
};

namespace FileTypes {

	const char* ChartErrorGetName(ChartError err);

	class ChartList : public FileType {
	private:
		uint32_t num_charts = 0;

		void initNew() override {};
	public:

		std::vector<Chart> charts;

		ChartError loadFromBinary(std::istream& in);
		ChartError loadFromFile(const std::string& filePath);
		ChartError writeToStream(std::ostream& out);
		ChartError writeToFile(const std::string& filePath);
		Chart& find_chart_by_chart_number(const uint8_t chart_number);
		Chart& find_chart_for_island_number(const uint8_t island_number);
	};
}
