#pragma once

#include <fstream>
#include <vector>
#include <string>
#include <unordered_map>


#define DEFAULT_LAYER 255 //Helps check for default layer (layer 0/NULL is a valid layer, 255 is not)

using namespace std::literals::string_literals; //probably not great in header



enum struct [[nodiscard]] DZXError
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

struct ChunkEntry {
	std::string data;
};

class Chunk {
public:
	unsigned int offset = 0;

	std::string type = "\0\0\0\0"s;
	unsigned int layer = DEFAULT_LAYER; //Uses 255 to signify default layer, default for chunks that don't use it
	uint32_t num_entries = 0;
	uint32_t first_entry_offset = 0;
	int entry_size = 0;
	std::vector<ChunkEntry> entries;

	DZXError read(std::istream& data, const unsigned int offset);
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
		std::vector<ChunkEntry*> entries_by_type(const std::string& chunk_type); //return vector of pointers so we can edit the chunk data
		std::vector<ChunkEntry*> entries_by_type_and_layer(const std::string& chunk_type, const unsigned int layer);
		ChunkEntry& add_entity(const std::string&, const unsigned int layer = DEFAULT_LAYER);
		void remove_entity(ChunkEntry* entity);
		DZXError writeToStream(std::ostream& out);
		DZXError writeToFile(const std::string& outFilePath);
	private:
		void initNew();
	};
}
