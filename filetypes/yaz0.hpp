//Format is part of sead (Nintendo's standard library)
//YAZ0 is a compression algorithm Nintendo has used across many games

#pragma once

#include <fstream>



enum struct [[nodiscard]] YAZ0Error {
	NONE = 0,
	COULD_NOT_OPEN,
	NOT_YAZ0,
	DATA_SIZE_0,
	REACHED_EOF,
	UNKNOWN,
	COUNT
};

namespace FileTypes {
	const char* YAZ0ErrorGetName(YAZ0Error err);

    YAZ0Error yaz0Encode(std::istream& in, std::ostream& out, uint32_t compressionLevel = 9);
    YAZ0Error yaz0Decode(std::istream& in, std::ostream& out);
    YAZ0Error yaz0Encode(std::stringstream& in, std::ostream& out, uint32_t compressionLevel = 9);
    YAZ0Error yaz0Decode(std::stringstream& in, std::ostream& out);
}
