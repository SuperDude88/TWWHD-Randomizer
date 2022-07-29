//Format is related to JAudio (an audio library for GameCube games)
//BDT archives store sequenced audio in .bms format
//This isn't used in rando at the moment, but could be useful for audio rando later
//Also provides another form of documentation/code others can use

#pragma once

#include <vector>
#include <string>



enum struct [[nodiscard]] BDTError {
	NONE = 0,
	COULD_NOT_OPEN,
	REACHED_EOF,
    UNEXPECTED_VALUE,
	UNKNOWN,
	COUNT
};

namespace FileTypes {

	const char* BDTErrorGetName(BDTError err);

	class BDTFile {
	public:
		struct FileSpec {
		    uint32_t offset;
		    uint32_t size;
		};
		
        std::vector<std::string> files;

		BDTError loadFromBinary(std::istream& in);
		BDTError loadFromFile(const std::string& filePath);
		BDTError writeToStream(std::ostream& out);
		BDTError writeToFile(const std::string& filePath);
	private:
		uint32_t numFiles;
		std::vector<FileSpec> fileInfo;
	};
}

