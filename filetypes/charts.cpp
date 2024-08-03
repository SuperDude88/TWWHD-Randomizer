#include "charts.hpp"

#include <algorithm>

#include <utility/endian.hpp>
#include <command/Log.hpp>

using eType = Utility::Endian::Type;

static constexpr std::array<GameItem, 49> island_num_to_item = {
    GameItem::TreasureChart25, // Sector 1 Forsaken Fortress
    GameItem::TreasureChart7,  // Sector 2 Star Island
    GameItem::TreasureChart24, // etc...
    GameItem::TreasureChart42,
    GameItem::TreasureChart11,
    GameItem::TreasureChart45,
    GameItem::TreasureChart13,
    GameItem::TreasureChart41,
    GameItem::TreasureChart29,
    GameItem::TreasureChart22,
    GameItem::TreasureChart18,
    GameItem::TreasureChart30,
    GameItem::TreasureChart39,
    GameItem::TreasureChart19,
    GameItem::TreasureChart8,
    GameItem::TreasureChart2,
    GameItem::TreasureChart10,
    GameItem::TreasureChart26,
    GameItem::TreasureChart3,
    GameItem::TreasureChart37,
    GameItem::TreasureChart27,
    GameItem::TreasureChart38,
    GameItem::TriforceChart1,
    GameItem::TreasureChart21,
    GameItem::TreasureChart6,
    GameItem::TreasureChart14,
    GameItem::TreasureChart34,
    GameItem::TreasureChart5,
    GameItem::TreasureChart28,
    GameItem::TreasureChart35,
    GameItem::TriforceChart2,
    GameItem::TreasureChart44,
    GameItem::TreasureChart1,
    GameItem::TreasureChart20,
    GameItem::TreasureChart36,
    GameItem::TreasureChart23,
    GameItem::TreasureChart12,
    GameItem::TreasureChart16,
    GameItem::TreasureChart4,
    GameItem::TreasureChart17,
    GameItem::TreasureChart31,
    GameItem::TriforceChart3,
    GameItem::TreasureChart9,
    GameItem::TreasureChart43,
    GameItem::TreasureChart40,
    GameItem::TreasureChart46,
    GameItem::TreasureChart15,
    GameItem::TreasureChart32,
    GameItem::TreasureChart33 // Sector 49 Five Star Isles
};

ChartError ChartPos::read(std::istream& in) {
	if (!in.read(reinterpret_cast<char*>(&tex_x_offset), sizeof(tex_x_offset))) {
		LOG_ERR_AND_RETURN(ChartError::REACHED_EOF);
	}
	if (!in.read(reinterpret_cast<char*>(&tex_y_offset), sizeof(tex_y_offset))) {
		LOG_ERR_AND_RETURN(ChartError::REACHED_EOF);
	}
	if (!in.read(reinterpret_cast<char*>(&salvage_x_pos), sizeof(salvage_x_pos))) {
		LOG_ERR_AND_RETURN(ChartError::REACHED_EOF);
	}
	if (!in.read(reinterpret_cast<char*>(&salvage_y_pos), sizeof(salvage_y_pos))) {
		LOG_ERR_AND_RETURN(ChartError::REACHED_EOF);
	}

	Utility::Endian::toPlatform_inplace(eType::Big, tex_x_offset);
	Utility::Endian::toPlatform_inplace(eType::Big, tex_y_offset);
	Utility::Endian::toPlatform_inplace(eType::Big, salvage_x_pos);
	Utility::Endian::toPlatform_inplace(eType::Big, salvage_y_pos);

	return ChartError::NONE;
}

void ChartPos::save_changes(std::ostream& out) const {
	const uint16_t tex_x = Utility::Endian::toPlatform(eType::Big, tex_x_offset);
	const uint16_t tex_y = Utility::Endian::toPlatform(eType::Big, tex_y_offset);
	const uint16_t salvage_x = Utility::Endian::toPlatform(eType::Big, salvage_x_pos);
	const uint16_t salvage_y = Utility::Endian::toPlatform(eType::Big, salvage_y_pos);

	out.write(reinterpret_cast<const char*>(&tex_x), sizeof(tex_x));
	out.write(reinterpret_cast<const char*>(&tex_y), sizeof(tex_y));
	out.write(reinterpret_cast<const char*>(&salvage_x), sizeof(salvage_x));
	out.write(reinterpret_cast<const char*>(&salvage_y), sizeof(salvage_y));
}



ChartError Chart::read(std::istream& in) {
	if (!in.read(reinterpret_cast<char*>(&texture_id), sizeof(texture_id))) {
		LOG_ERR_AND_RETURN(ChartError::REACHED_EOF);
	}
	if (!in.read(reinterpret_cast<char*>(&owned_chart_index_plus_1), sizeof(owned_chart_index_plus_1))) {
		LOG_ERR_AND_RETURN(ChartError::REACHED_EOF);
	}
	if (!in.read(reinterpret_cast<char*>(&number), sizeof(number))) {
		LOG_ERR_AND_RETURN(ChartError::REACHED_EOF);
	}
	if (!in.read(reinterpret_cast<char*>(&type), sizeof(type))) {
		LOG_ERR_AND_RETURN(ChartError::REACHED_EOF);
	}
	if (!in.read(reinterpret_cast<char*>(&sector_x), sizeof(sector_x))) {
		LOG_ERR_AND_RETURN(ChartError::REACHED_EOF);
	}
	if (!in.read(reinterpret_cast<char*>(&sector_y), sizeof(sector_y))) {
		LOG_ERR_AND_RETURN(ChartError::REACHED_EOF);
	}

	for (unsigned int i = 0; i < 4; i++) {
		ChartPos pos;
		LOG_AND_RETURN_IF_ERR(pos.read(in));
		possible_positions[i] = pos;
	}

	if(number >= 1 && number <= 49) item_name = island_num_to_item[getIslandNumber() - 1];

	return ChartError::NONE;
}

void Chart::save_changes(std::ostream& out) {
	out.write(reinterpret_cast<const char*>(&texture_id), sizeof(texture_id));
	out.write(reinterpret_cast<const char*>(&owned_chart_index_plus_1), sizeof(owned_chart_index_plus_1));
	out.write(reinterpret_cast<const char*>(&number), sizeof(number));
	out.write(reinterpret_cast<const char*>(&type), sizeof(type));

	out.write(reinterpret_cast<const char*>(&sector_x), sizeof(sector_x));
	out.write(reinterpret_cast<const char*>(&sector_y), sizeof(sector_y));

	for (ChartPos& pos : possible_positions) {
		pos.save_changes(out);
	}
}

uint8_t Chart::getIslandNumber() const {
	return (sector_x + 3) + ((sector_y + 3) * 7) + 1;
}

ChartError Chart::setIslandNumber(uint8_t islandNum) {
	if (islandNum < 1 || islandNum > 49) {
		LOG_ERR_AND_RETURN(ChartError::INVALID_NUMBER);
	}

	const uint8_t island_index = islandNum - 1;
	sector_x = (island_index % 7) - 3;
	sector_y = (island_index / 7) - 3;

	return ChartError::NONE;
}

GameItem Chart::getItem() const {
	if (number == 0 || number > 49) {
		return GameItem::INVALID;
	}
	return item_name;
}



namespace FileTypes {

	const char* ChartErrorGetName(const ChartError err) {
		switch (err) {
			case ChartError::NONE:
				return "NONE";
			case ChartError::COULD_NOT_OPEN:
				return "COULD_NOT_OPEN";
			case ChartError::INVALID_NUMBER:
				return "INVALID_NUMBER";
			case ChartError::REACHED_EOF:
				return "REACHED_EOF";
			default:
				return "UNKNOWN";
		}
	}

	ChartError ChartList::loadFromBinary(std::istream& in) {
		if (!in.read(reinterpret_cast<char*>(&num_charts), sizeof(num_charts))) {
			LOG_ERR_AND_RETURN(ChartError::REACHED_EOF);
		}

		Utility::Endian::toPlatform_inplace(eType::Big, num_charts);

		for (uint32_t i = 0; i < num_charts; i++) {
			Chart chart;
			LOG_AND_RETURN_IF_ERR(chart.read(in));
			charts.push_back(chart);
		}

		return ChartError::NONE;
	}

	ChartError ChartList::loadFromFile(const fspath& filePath) {
		std::ifstream file(filePath, std::ios::binary);
		if (!file.is_open()) {
			LOG_ERR_AND_RETURN(ChartError::COULD_NOT_OPEN);
		}
		return loadFromBinary(file);
	}

	ChartError ChartList::writeToStream(std::ostream& out) {
		num_charts = charts.size();
		Utility::Endian::toPlatform_inplace(eType::Big, num_charts);
		out.write(reinterpret_cast<const char*>(&num_charts), sizeof(num_charts));

		for (Chart& chart : charts) {
			chart.save_changes(out);
		}

		return ChartError::NONE;
	}

	ChartError ChartList::writeToFile(const fspath& filePath) {
		std::ofstream file(filePath, std::ios::binary);
		if (!file.is_open()) {
			LOG_ERR_AND_RETURN(ChartError::COULD_NOT_OPEN);
		}
		return writeToStream(file);
	}
}
