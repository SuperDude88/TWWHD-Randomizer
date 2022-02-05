#include "Commands.hpp"

#include "../filetypes/wiiurpx.hpp"
#include "../filetypes/yaz0.hpp"

namespace Commands {
    const char* getErrorName(CommandError err)
    {
        switch(err)
        {
        case CommandError::NONE:
            return "NONE";
        case CommandError::CANNOT_OPEN_FILE:
            return "CANNOT_OPEN_FILE";
        case CommandError::EOF_REACHED:
            return "EOF_REACHED";
        case CommandError::RPX_OPERATION_FAILED:
            return "RPX_OPERATION_FAILED";
        case CommandError::YAZ0_OPERATION_FAILED:
            return "YAZ0_OPERATION_FAILED";
        case CommandError::SARC_OPERATION_FAILED:
            return "SARC_OPERATION_FAILED";
        case CommandError::BFRES_OPERATION_FAILED:
            return "BFRES_OPERATION_FAILED";
        case CommandError::UNKNOWN:
            return "UNKNOWN";
        case CommandError::COUNT:
            return "COUNT";
        default:
            return "ILLEGAL";
        }
    }

    CommandError getBinaryData(const std::string& filePath, size_t offset, size_t length, char* dataOut)
    {
        std::ifstream file(filePath.c_str(), std::ios::binary);
        if (!file.is_open()) return CommandError::CANNOT_OPEN_FILE;
        
        if (!file.seekg(offset))
        {
            return CommandError::EOF_REACHED;
        }

        if(!file.read(dataOut, length))
        {
            return CommandError::EOF_REACHED;
        }
        return CommandError::NONE;
    }

    CommandError replaceBinaryData(const std::string& filePath, size_t offset, std::string replaceWith)
    {
        auto mode = std::ios::binary | std::ios::in | std::ios::out;
        std::ofstream file(filePath.c_str(), mode);
        if (!file.is_open()) return CommandError::CANNOT_OPEN_FILE;
        if (!file.seekp(offset)) return CommandError::EOF_REACHED;
        if(!file.write(replaceWith.data(), replaceWith.size())) return CommandError::EOF_REACHED;
        return CommandError::NONE;
    }

    CommandError convertRPXToELF(const std::string& rpxPath, const std::string& outPath)
    {
        std::ifstream rpxFile(rpxPath, std::ios::binary);
        if(!rpxFile.is_open()) return CommandError::CANNOT_OPEN_FILE;
        std::ofstream elfFile(outPath, std::ios::binary);
        if(!elfFile.is_open()) return CommandError::CANNOT_OPEN_FILE;
        if(RPXError err = FileTypes::rpx_decompress(rpxFile, elfFile); err != RPXError::NONE)
        {
            return CommandError::RPX_OPERATION_FAILED;
        }
        return CommandError::NONE;
    }

    CommandError convertELFToRPX(const std::string& elfPath, const std::string& outPath)
    {
        std::ifstream elfFile(elfPath, std::ios::binary);
        if(!elfFile.is_open()) return CommandError::CANNOT_OPEN_FILE;
        std::ofstream rpxFile(outPath, std::ios::binary);
        if(!rpxFile.is_open()) return CommandError::CANNOT_OPEN_FILE;
        if(RPXError err = FileTypes::rpx_decompress(elfFile, rpxFile); err != RPXError::NONE)
        {
            return CommandError::RPX_OPERATION_FAILED;
        }
        return CommandError::NONE;
    }

    CommandError yaz0Decompress(std::istream& in, std::ostream& out)
    {
        if(YAZ0Error err = FileTypes::yaz0Decode(in, out); err != YAZ0Error::NONE)
        {
            return CommandError::YAZ0_OPERATION_FAILED;
        }
        return CommandError::NONE;
    }

    CommandError yaz0Compress(std::istream& in, std::ostream& out)
    {
        if(YAZ0Error err = FileTypes::yaz0Encode(in, out); err != YAZ0Error::NONE)
        {
            return CommandError::YAZ0_OPERATION_FAILED;
        }
        return CommandError::NONE;
    }
}
