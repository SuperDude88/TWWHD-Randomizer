//RPX files are game executables that run on the Wii U
//They are very similar to ELF binaries, but only use section headers (program headers are ignored)
//There are some sections with RPX metadata, and most sections are compressed with ZLIB

#pragma once

#include <cstdint>
#include <fstream>



enum struct [[nodiscard]] RPXError {
    NONE = 0,
    COULD_NOT_OPEN,
    NOT_RPX,
    UNEXPECTED_VALUE,
    ZLIB_ERROR,
    REACHED_EOF,
    UNKNOWN,
    COUNT
};

namespace FileTypes {
    const char* RPXErrorGetName(RPXError err);

    RPXError rpx_decompress(std::istream& in, std::ostream& out);
    RPXError rpx_compress(std::istream& in, std::ostream& out);
}
