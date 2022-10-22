#include "dzx.hpp"


#include <cstring>
#include <cstdio>

#include <utility/endian.hpp>
#include <utility/common.hpp>
#include <utility/file.hpp>
#include <command/Log.hpp>

using eType = Utility::Endian::Type;

static const std::unordered_map<std::string, unsigned int> size_by_type{ {"SCOB", 0x24}, {"ACTR", 0x20}, {"TRES", 0x20}, {"PLYR", 0x20}, {"SCLS", 0xC}, {"STAG", 0x14}, {"FILI", 0x8}, {"SHIP", 0x10}, {"RTBL", 0x4}, {"RTBL_SubEntry", 0x8}, {"RTBL_AdjacentRoom", 0x1}, {"RPAT", 0xC}, {"RPPN", 0x10}, {"TGOB", 0x20}, {"TGSC", 0x24}, {"DOOR", 0x24}, {"TGDR", 0x24}, {"EVNT", 0x18}, {"2DMA", 0x38}, {"MULT", 0xC}, {"FLOR", 0x14}, {"LBNK", 0x1}, {"SOND", 0x1C}, {"RCAM", 0x14}, {"RARO", 0x14}, {"DMAP", 0x10}, {"EnvR", 0x8}, {"Colo", 0xC}, {"Pale", 0x6C}, {"Virt", 0x38}, {"LGHT", 0x1C}, {"LGTV", 0x1C}, {"MECO", 0x2}, {"MEMA", 0x4}, {"PATH", 0xC}, {"PPNT", 0x10}, {"CAMR", 0x14}, {"AROB", 0x14} };

static const std::unordered_map<char, unsigned int> layer_char_to_layer_index{ {'0', 0}, {'1', 1}, {'2', 2}, {'3', 3}, {'4', 4}, {'5', 5}, {'6', 6}, {'7', 7}, {'8', 8}, {'9', 9}, {'a', 10}, {'b', 11} };


DZXError Chunk::read(std::istream& file, const unsigned int offset) {
	file.seekg(offset, std::ios::beg);

	this->offset = offset;
	type.resize(4);
	if (!file.read(&type[0], 4)) {
		LOG_ERR_AND_RETURN(DZXError::REACHED_EOF);
	}
	if (!file.read(reinterpret_cast<char*>(&num_entries), 4)) {
		LOG_ERR_AND_RETURN(DZXError::REACHED_EOF);
	}
	if (!file.read(reinterpret_cast<char*>(&first_entry_offset), 4)) {
		LOG_ERR_AND_RETURN(DZXError::REACHED_EOF);
	}
	Utility::Endian::toPlatform_inplace(eType::Big, num_entries);
	Utility::Endian::toPlatform_inplace(eType::Big, first_entry_offset);
	
	if (std::strncmp("TRE", &type[0], 3) == 0) {
		char layer_char = type[3];
		layer = DEFAULT_LAYER;
		if (layer_char_to_layer_index.find(layer_char) != layer_char_to_layer_index.end()) {
			layer = layer_char_to_layer_index.at(layer_char);
		}
		else if (layer_char != 'S') { //Should always be 'S' if it is not a unique layer char
			LOG_ERR_AND_RETURN(DZXError::UNKNOWN_LAYER_CHAR);
		}
		type = "TRES";
	}
	else if (std::strncmp("ACT", &type[0], 3) == 0) {
		char layer_char = type[3];
		layer = DEFAULT_LAYER;
		if (layer_char_to_layer_index.find(layer_char) != layer_char_to_layer_index.end()) {
			layer = layer_char_to_layer_index.at(layer_char);
		}
		else if (layer_char != 'R') { //Should always be 'R' if it is not a unique layer char
			LOG_ERR_AND_RETURN(DZXError::UNKNOWN_LAYER_CHAR);
		}
		type = "ACTR";
	}
	else if (std::strncmp("SCO", &type[0], 3) == 0) {
		char layer_char = type[3];
		layer = DEFAULT_LAYER;
		if (layer_char_to_layer_index.find(layer_char) != layer_char_to_layer_index.end()) {
			layer = layer_char_to_layer_index.at(layer_char);
		}
		else if (layer_char != 'B') { //Should always be 'B' if it is not a unique layer char
			LOG_ERR_AND_RETURN(DZXError::UNKNOWN_LAYER_CHAR);
		}
		type = "SCOB";
	}

	if (size_by_type.count(type) == 0) {
		LOG_ERR_AND_RETURN(DZXError::UNKNOWN_CHUNK);
	}
	entry_size = size_by_type.at(type);

	file.seekg(first_entry_offset, std::ios::beg);
	if (std::strncmp("RTBL", &type[0], 4) == 0) { //RTBL has dynamic length based on the number of rooms, needs a special case
		for (unsigned int entry_index = 0; entry_index < num_entries; entry_index++) {
			ChunkEntry entry;
			file.seekg(entry_index * 0x4 + first_entry_offset, std::ios::beg);
			entry.data.resize(12);
			if (!file.read(&entry.data[0], 4)) {
				LOG_ERR_AND_RETURN(DZXError::REACHED_EOF);
			}
			uint32_t subentry_offset = *(reinterpret_cast<uint32_t*>(&entry.data[0]));
			Utility::Endian::toPlatform_inplace(eType::Big, subentry_offset);

			file.seekg(subentry_offset, std::ios::beg);
			if (!file.read(&entry.data[4], 8)) {
				LOG_ERR_AND_RETURN(DZXError::REACHED_EOF);
			}
			uint8_t num_rooms = *(reinterpret_cast<uint8_t*>(&entry.data[4]));
			uint32_t adjacent_rooms_offset = *(reinterpret_cast<uint32_t*>(&entry.data[8]));
			Utility::Endian::toPlatform_inplace(eType::Big, adjacent_rooms_offset);

			entry.data.resize(12 + num_rooms);
			file.seekg(adjacent_rooms_offset, std::ios::beg);
			if (!file.read(&entry.data[12], num_rooms)) { //read all the rooms into data at once
				LOG_ERR_AND_RETURN(DZXError::REACHED_EOF);
			}
			entry_size = entry.data.size();
			entries.push_back(entry);
		}
	}
	else {
		for (unsigned int entry_index = 0; entry_index < num_entries; entry_index++) {
			ChunkEntry entry;
			entry.data.resize(entry_size);
			if (!file.read(&entry.data[0], entry_size)) {
				LOG_ERR_AND_RETURN(DZXError::REACHED_EOF);
			}
			entries.push_back(entry);
		}
	}
	return DZXError::NONE;
}

DZXError Chunk::save_changes(std::ostream& out) {
	offset = out.tellp();

	num_entries = entries.size();
	//When nintendo deleted tuner triggers they left some empty chunks
	//if (num_entries == 0) {
	//	return DZXError::CHUNK_NO_ENTRIES;
	//}

	if(layer != DEFAULT_LAYER) {
		char buf[2];
		std::snprintf(buf, 2, "%x", layer);
		type[3] = buf[0];
	}
	out.write(&type[0], 4);
	const auto num_entries_byteswap = Utility::Endian::toPlatform(eType::Big, num_entries); //Need to use num_entries later, can't byteswap inplace
	out.write(reinterpret_cast<const char*>(&num_entries_byteswap), 4);
	Utility::seek(out, 4, std::ios::cur); //filled in later

	return DZXError::NONE;
}

namespace FileTypes {

	const char* DZXErrorGetName(DZXError err) {
		switch (err) {
			case DZXError::NONE:
				return "NONE";
			case DZXError::COULD_NOT_OPEN:
				return "COULD_NOT_OPEN";
			case DZXError::UNKNOWN_CHUNK:
				return "UNKNOWN_CHUNK";
			case DZXError::UNKNOWN_LAYER_CHAR:
				return "UNKNOWN_LAYER_CHAR";
			case DZXError::CHUNK_NO_ENTRIES:
				return "CHUNK_NO_ENTRIES";
			case DZXError::NO_CHUNKS:
				return "NO_CHUNKS";
			case DZXError::REACHED_EOF:
				return "REACHED_EOF";
			case DZXError::HEADER_DATA_NOT_LOADED:
				return "HEADER_DATA_NOT_LOADED";
			case DZXError::FILE_DATA_NOT_LOADED:
				return "FILE_DATA_NOT_LOADED";
			default:
				return "UNKNOWN";
		}
	}

	DZXFile::DZXFile() {
		
	}

	void DZXFile::initNew() {
		num_chunks = 0;
		chunks = {};
	}

	DZXFile DZXFile::createNew(const std::string& filename) {
		DZXFile newDZX{};
		newDZX.initNew();
		return newDZX;
	}

	DZXError DZXFile::loadFromBinary(std::istream& dzx) {

		if (!dzx.read(reinterpret_cast<char*>(&num_chunks), 4)) {
			LOG_ERR_AND_RETURN(DZXError::REACHED_EOF);
		}
		Utility::Endian::toPlatform_inplace(eType::Big, num_chunks);
		if (num_chunks == 0) {
			LOG_ERR_AND_RETURN(DZXError::NO_CHUNKS);
		}

		for (uint32_t chunk_index = 0; chunk_index < num_chunks; chunk_index++) {
			uint32_t offset = 4 + chunk_index * 0xC;
			Chunk chunk;
			LOG_AND_RETURN_IF_ERR(chunk.read(dzx, offset));
			chunks.push_back(chunk);
		}
		return DZXError::NONE;
	}

	DZXError DZXFile::loadFromFile(const std::string& filePath) {
		std::ifstream file(filePath, std::ios::binary);
		if (!file.is_open()) {
			LOG_ERR_AND_RETURN(DZXError::COULD_NOT_OPEN);
		}
		return loadFromBinary(file);
	}

	std::vector<ChunkEntry*> DZXFile::entries_by_type(const std::string& chunk_type) {
		std::vector<ChunkEntry*> entries;
		for (Chunk& chunk : chunks) {
			if (chunk_type == chunk.type) {
				for (ChunkEntry& entry : chunk.entries) {
					entries.push_back(&entry); //can't use insert because it needs to be converted to a pointer
				}
			}
		}
		return entries;
	}

	std::vector<ChunkEntry*> DZXFile::entries_by_type_and_layer(const std::string& chunk_type, const unsigned int layer) {
		std::vector<ChunkEntry*> entries;
		for (Chunk& chunk : chunks) {
			if (chunk_type == chunk.type && layer == chunk.layer) {
				for (ChunkEntry& entry : chunk.entries) {
					entries.push_back(&entry); //can't use insert because it needs to be converted to a pointer
				}
			}
		}
		return entries;
	}

	ChunkEntry& DZXFile::add_entity(const std::string& chunk_type, const unsigned int layer) {
		ChunkEntry entity;
		for (Chunk& chunk : chunks) {
			if (chunk_type == chunk.type && layer == chunk.layer) {
				chunk.entries.push_back(entity);
				return chunk.entries.back(); //return reference to the entity we added
			}
		}

		//if chunk does not already exist
		Chunk chunk;
		chunk.type = chunk_type;
		chunk.layer = layer;
		chunk.entry_size = size_by_type.at(chunk_type);
		chunk.entries.push_back(entity);
		chunks.push_back(chunk);
		return chunks.back().entries.back(); //return reference to the entity we added
	}

	void DZXFile::remove_entity(ChunkEntry* entity) {
		size_t len = entity->data.size();
		entity->data.clear();
		entity->data.resize(len, '\x00'); //clear entity, replace it with null data
		return;
	}

	DZXError DZXFile::writeToStream(std::ostream& out) {
		for (auto it = chunks.begin(); it != chunks.end(); it++) {
			if((*it).entries.size() == 0) it = chunks.erase(it); //remove empty chunks
		}
		num_chunks = chunks.size();
		if (num_chunks == 0) {
			LOG_ERR_AND_RETURN(DZXError::NO_CHUNKS);
		}
		Utility::Endian::toPlatform_inplace(eType::Big, num_chunks);
		out.write(reinterpret_cast<const char*>(&num_chunks), 4);

		for (Chunk& chunk : chunks) {
			LOG_AND_RETURN_IF_ERR(chunk.save_changes(out));
		}
		
		for (Chunk& chunk : chunks) {
			int padding_size = 4 - out.tellp() % 4;
			if (padding_size == 4) {
				padding_size = 0;
			}
			for (int byte = 0; byte < padding_size; byte++) { //pad to nearest 4 bytes
				out.write("\xFF", 1);
			}

			chunk.first_entry_offset = out.tellp();
			Utility::seek(out, chunk.offset + 8, std::ios::beg);
			const auto first_entry_offset = Utility::Endian::toPlatform(eType::Big, chunk.first_entry_offset);
			out.write(reinterpret_cast<const char*>(&first_entry_offset), 4);

			Utility::seek(out, chunk.first_entry_offset, std::ios::beg);
			if (chunk.entries.size() == 0) {
				LOG_ERR_AND_RETURN(DZXError::CHUNK_NO_ENTRIES);
			}
			if (std::strncmp("RTBL", &chunk.type[0], 4) == 0) { //RTBL has a dynamic length based on rooms, needs to be saved differently
				unsigned int rooms_offset = chunk.first_entry_offset + chunk.entries.size() * 0xC;
				for (unsigned int entry_index = 0; entry_index < chunk.entries.size(); entry_index++) {
					ChunkEntry& entry = chunk.entries[entry_index];
					uint32_t subentry_offset = chunk.first_entry_offset + entry_index * 0x8 + chunk.entries.size() * 0x4; //update the subentry's offset
					const auto subentry_offset_byteswap = Utility::Endian::toPlatform(eType::Big, subentry_offset);
					entry.data.replace(0, 4, reinterpret_cast<const char*>(&subentry_offset_byteswap), 4);
					Utility::seek(out, chunk.first_entry_offset + entry_index * 4, std::ios::beg);
					out.write(reinterpret_cast<const char*>(&subentry_offset_byteswap), 4);
					
					Utility::seek(out, subentry_offset, std::ios::beg);
					out.write(&entry.data[4], 4);
					entry.data.replace(8, 4, reinterpret_cast<char*>(&rooms_offset), 4);
					const auto rooms_offset_byteswap = Utility::Endian::toPlatform(eType::Big, rooms_offset);
					out.write(reinterpret_cast<const char*>(&rooms_offset_byteswap), 4);
					Utility::seek(out, rooms_offset, std::ios::beg);

					out.write(&entry.data[12], entry.data[4]);
					rooms_offset += entry.data[4]; //Add number of rooms (1 byte each) to the room offset for next iteration
				}

			}
			else {
				for (const ChunkEntry& entry : chunk.entries) {
					out.write(&entry.data[0], entry.data.size());
				}
			}
		}
		padToLen(out, 0x20, '\xFF');

		return DZXError::NONE;
	}

	DZXError DZXFile::writeToFile(const std::string& outFilePath) {
		std::ofstream outFile(outFilePath, std::ios::binary);
		if (!outFile.is_open()) {
			LOG_ERR_AND_RETURN(DZXError::COULD_NOT_OPEN);
		}
		return writeToStream(outFile);
	}
}
