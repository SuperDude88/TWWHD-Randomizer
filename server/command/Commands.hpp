#pragma once

#include <string>
#include <cstdint>
#include <iostream>

namespace Commands {
    enum struct CommandError
    {
        NONE = 0,
        CANNOT_OPEN_FILE,
        EOF_REACHED,
        RPX_OPERATION_FAILED,
        YAZ0_OPERATION_FAILED,
        SARC_OPERATION_FAILED,
        BFRES_OPERATION_FAILED,
        UNKNOWN,
        COUNT
    };

    const char* getErrorName(CommandError err);

    CommandError getBinaryData(const std::string& filePath, size_t offset, size_t length, char* dataOut);
    CommandError replaceBinaryData(const std::string& filePath, size_t offset, std::string replaceWith);
    CommandError convertRPXToELF(const std::string& rpxPath, const std::string& outPath);
    CommandError convertELFToRPX(const std::string& elfPath, const std::string& outPath);
    CommandError yaz0Decompress(std::istream& in, std::ostream& out);
    CommandError yaz0Compress(std::istream& in, std::ostream& out);
}
