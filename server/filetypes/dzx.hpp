#pragma once

#include<cstring>
#include<fstream>
#include<vector>
#include<string>
#include<unordered_map>
#include"../utility/byteswap.hpp"

#define default_layer 255 //Helps check for default layer (layer 0/NULL is a valid layer, 255 is not)

enum struct DZXError
{
	NONE = 0,
	COULD_NOT_OPEN,
	UNKNOWN_CHUNK,
	UNKNOWN_LAYER_CHAR,
	CHUNK_NO_ENTRIES,
	NO_CHUNKS,
	REACHED_EOF,
    HEADER_DATA_NOT_LOADED,
    FILE_DATA_NOT_LOADED,
    UNKNOWN,
    COUNT
};

static const std::unordered_map<std::string, int> size_by_type{ {"SCOB", 0x24}, {"ACTR", 0x20}, {"TRES", 0x20}, {"PLYR", 0x20}, {"SCLS", 0xC}, {"STAG", 0x14}, {"FILI", 0x8}, {"SHIP", 0x10}, {"RTBL", 0x4}, {"RTBL_SubEntry", 0x8}, {"RTBL_AdjacentRoom", 0x1}, {"RPAT", 0xC}, {"RPPN", 0x10}, {"TGOB", 0x20}, {"TGSC", 0x24}, {"DOOR", 0x24}, {"TGDR", 0x24}, {"EVNT", 0x18}, {"2DMA", 0x38}, {"MULT", 0xC}, {"FLOR", 0x14}, {"LBNK", 0x1}, {"SOND", 0x1C}, {"RCAM", 0x14}, {"RARO", 0x14}, {"DMAP", 0x10}, {"EnvR", 0x8}, {"Colo", 0xC}, {"Pale", 0x6C}, {"Virt", 0x38}, {"LGHT", 0x1C}, {"LGTV", 0x1C}, {"MECO", 0x2}, {"MEMA", 0x4}, {"PATH", 0xC}, {"PPNT", 0x10}, {"CAMR", 0x14}, {"AROB", 0x14}};

static const std::unordered_map<char, int> layer_char_to_layer_index{ {'0', 0}, {'1', 1}, {'2', 2}, {'3', 3}, {'4', 4}, {'5', 5}, {'6', 6}, {'7', 7}, {'8', 8}, {'9', 9}, {'a', 10}, {'b', 11} };

struct ChunkEntry {
	std::string data;
};

class Chunk {
public:
	int offset = 0;
	char type[5] = "\0\0\0\0";
	int layer = default_layer; //Uses 255 to signify default layer, default for chunks that don't use it
	uint32_t num_entries = 0;
	uint32_t first_entry_offset = 0;
	int entry_size = 0;
	std::vector<ChunkEntry> entries;

	DZXError read(std::istream& data, int offset);
	DZXError save_changes(std::ostream& out);
};

namespace FileTypes {

	const char* DZXErrorGetName(DZXError err);

	class DZXFile {
	public:
		uint32_t num_chunks;
		std::vector<Chunk> chunks;

		DZXFile();
		static DZXFile createNew(const std::string& filename);
		DZXError loadFromBinary(std::istream& dzx);
		DZXError loadFromFile(const std::string& filePath);
		std::vector<ChunkEntry*> entries_by_type(std::string chunk_type); //return vector of pointers so we can edit the chunk data
		std::vector<ChunkEntry*> entries_by_type_and_layer(std::string chunk_type, int layer);
		ChunkEntry& add_entity(const char chunk_type[4], int layer = default_layer);
		void remove_entity(ChunkEntry* entity);
		DZXError writeToStream(std::ostream& out);
		DZXError writeToFile(const std::string& outFilePath);
	private:
		void initNew();
	};
}
