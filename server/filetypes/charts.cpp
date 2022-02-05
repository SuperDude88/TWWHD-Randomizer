#include "charts.hpp"

#include <cmath>
#include <algorithm>

#include "../utility/byteswap.hpp"



ChartError ChartPos::read(std::istream& in, const unsigned int offset) {
	this->offset = offset;
	in.seekg(offset, std::ios::beg);

	if (!in.read((char*)&tex_x_offset, sizeof(tex_x_offset))) {
		return ChartError::REACHED_EOF;
	}
	if (!in.read((char*)&tex_y_offset, sizeof(tex_y_offset))) {
		return ChartError::REACHED_EOF;
	}
	if (!in.read((char*)&salvage_x_pos, sizeof(salvage_x_pos))) {
		return ChartError::REACHED_EOF;
	}
	if (!in.read((char*)&salvage_y_pos, sizeof(salvage_y_pos))) {
		return ChartError::REACHED_EOF;
	}

	Utility::byteswap_inplace(tex_x_offset);
	Utility::byteswap_inplace(tex_y_offset);
	Utility::byteswap_inplace(salvage_x_pos);
	Utility::byteswap_inplace(salvage_y_pos);

	return ChartError::NONE;
}

void ChartPos::save_changes(std::ostream& out) {
	out.seekp(offset, std::ios::beg);

	uint16_t tex_x = Utility::byteswap(tex_x_offset);
	uint16_t tex_y = Utility::byteswap(tex_y_offset);
	uint16_t salvage_x = Utility::byteswap(salvage_x_pos);
	uint16_t salvage_y = Utility::byteswap(salvage_y_pos);

	out.write((char*)&tex_x, sizeof(tex_x));
	out.write((char*)&tex_y, sizeof(tex_y));
	out.write((char*)&salvage_x, sizeof(salvage_x));
	out.write((char*)&salvage_y, sizeof(salvage_y));

	return;
}



ChartError Chart::read(std::istream& in, const unsigned int offset) {
	this->offset = offset;
	in.seekg(offset, std::ios::beg);

	if (!in.read((char*)&texture_id, sizeof(texture_id))) {
		return ChartError::REACHED_EOF;
	}
	if (!in.read((char*)&owned_chart_index_plus_1, sizeof(owned_chart_index_plus_1))) {
		return ChartError::REACHED_EOF;
	}
	if (!in.read((char*)&number, sizeof(number))) {
		return ChartError::REACHED_EOF;
	}
	if (!in.read((char*)&type, sizeof(type))) {
		return ChartError::REACHED_EOF;
	}
	if (!in.read((char*)&sector_x, sizeof(sector_x))) {
		return ChartError::REACHED_EOF;
	}
	if (!in.read((char*)&sector_y, sizeof(sector_y))) {
		return ChartError::REACHED_EOF;
	}

	for (unsigned int i = 0; i < 4; i++) {
		ChartPos pos;
		ChartError err = pos.read(in, this->offset + 6 + (0x8 * i));
		if (err != ChartError::NONE) {
			return err;
		}
		possible_positions[i] = pos;
	}

	island_number = sector_x + 3 + (sector_y + 3) * 7 + 1;

	if (number < 1 || number > 49) {
		return ChartError::INVALID_NUMBER;
	}
	if (number <= 3) { //may be different on HD
		item_name = "Triforce Chart " + std::to_string(number);
	}
	else {
		item_name = "Treasure Chart " + std::to_string(number);
	}

	return ChartError::NONE;
}

void Chart::save_changes(std::ostream& out) {
	out.seekp(offset, std::ios::beg);

	out.write((char*)&texture_id, sizeof(texture_id));
	out.write((char*)&owned_chart_index_plus_1, sizeof(owned_chart_index_plus_1));
	out.write((char*)&number, sizeof(number));
	out.write((char*)&type, sizeof(type));

	out.write((char*)&sector_x, sizeof(sector_x));
	out.write((char*)&sector_y, sizeof(sector_y));

	for (ChartPos& pos : possible_positions) {
		pos.save_changes(out);
	}
}

uint8_t Chart::getIslandNumber() const {
	return island_number;
}

ChartError Chart::setIslandNumber(const uint8_t value) {
	if (number < 1 || number > 49) {
		return ChartError::INVALID_NUMBER;
	}

	uint8_t island_index = value - 1;
	sector_x = (island_index % 7) - 3;
	sector_y = floor(island_index / 7) - 3;

	island_number = sector_x + 3 + (sector_y + 3) * 7 + 1;

	return ChartError::NONE;
}

std::string Chart::getName() const {
	return item_name;
}

ChartError Chart::setName(const uint8_t value) {
	if (number < 1 || number > 49) {
		return ChartError::INVALID_NUMBER;
	}

	if (number <= 3) { //may be different on HD
		item_name = "Triforce Chart " + std::to_string(value);
	}
	else {
		item_name = "Treasure Chart " + std::to_string(value);
	}

	return ChartError::NONE;
}


namespace FileTypes {

	const char* ChartErrorGetName(ChartError err) {
		switch (err) {
			case ChartError::NONE:
				return "NONE";
			case ChartError::COULD_NOT_OPEN:
				return "COULD_NOT_OPEN";
			case ChartError::INVALID_NUMBER:
				return "INVALID_NUMBER";
			case ChartError::REACHED_EOF:
				return "REACHED_EOF";
			case ChartError::COUNT:
				return "COUNT";
			default:
				return "UNKNOWN";
		}
	}

	ChartError ChartList::loadFromBinary(std::istream& in) {
		if (!in.read((char*)&num_charts, sizeof(num_charts))) {
			return ChartError::REACHED_EOF;
		}

		Utility::byteswap_inplace(num_charts);

		int offset = 4;
		for (unsigned int i = 0; i < num_charts; i++) {
			Chart chart;
			if (ChartError err = chart.read(in, offset); err != ChartError::NONE) return err;
			charts.push_back(chart);
			offset += 0x26;
		}

		return ChartError::NONE;
	}

	ChartError ChartList::loadFromFile(const std::string& filePath) {
		std::ifstream file(filePath, std::ios::binary);
		if (!file.is_open()) {
			return ChartError::COULD_NOT_OPEN;
		}
		return loadFromBinary(file);
	}

	Chart& ChartList::find_chart_by_chart_number(const uint8_t chart_number) {
		auto it = std::find_if(charts.begin(), charts.end(), [chart_number](const Chart& chart) {return chart.number == chart_number; });
		return *it;
	}

	Chart& ChartList::find_chart_for_island_number(const uint8_t island_number) {
		auto it = std::find_if(charts.begin(), charts.end(), [island_number](const Chart& chart) {return chart.number == island_number && (chart.type == 0 || chart.type == 1 || chart.type == 2 || chart.type == 6); });
		return *it;
	}
}