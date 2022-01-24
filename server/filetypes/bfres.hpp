#pragma once

#include <cstdint>
#include <cstring>
#include <string>
#include <vector>
#include <variant>
#include <fstream>
#include <algorithm>
#include <filesystem>

#include "../utility/byteswap.hpp"



enum struct FRESError
{
    NONE = 0,
    COULD_NOT_OPEN,
    NOT_FRES,
    REACHED_EOF,
    STRING_TOO_LONG,
    STRING_NOT_FOUND,
    HEADER_DATA_NOT_LOADED,
    FILE_DATA_NOT_LOADED,
    FRES_NOT_EMPTY,
    FRES_IS_EMTPY,
    GROUP_IS_EMPTY,
    UNKNOWN,
    COUNT
};

//Only embedded files are currently implemented

struct EmbeddedFileSpec {
    int location; //Used to deal with relative offsets, not a part of the file data
    int32_t dataOffset;
    uint32_t fileLength;
};

struct FRESHeader
{
    char magicFRES[4];
    uint8_t version[4];
    uint16_t byteOrderMarker;
    uint16_t headerLength_0x10;
    uint32_t fileSize;
    uint32_t fileAlignment;
    int32_t nameOffset;
    int32_t stringTableSize;
    int32_t stringTableOffset;
    int32_t groupOffsets[12];
    uint16_t groupCounts[12];
    uint32_t userPointer;
    std::vector<EmbeddedFileSpec> embeddedFiles; //Mostly for ease of editing, aren't directly related to the main header
};

struct GroupEntry
{
    int location; //Used to deal with relative offsets, not a part of the file data
    uint32_t searchValue;
    uint16_t leftIndex;
    uint16_t rightIndex;
    int32_t namePointer;
    int32_t dataPointer;
};

struct GroupHeader
{
    uint32_t groupLength;
    int32_t entryCount;
    std::vector<GroupEntry> entries;
};

struct IndexGroup
{
    GroupHeader header;
    std::vector<GroupEntry> entries;
};

struct FRESFileSpec
{
    /// name of the file from the string table
    std::string fileName;
    /// offset relative to start of FRES file
    uint32_t fileOffset;
    /// length of file past offset
    uint32_t fileLength;
};

struct FRESStringTableEntry
{
    uint32_t length;
    std::string str;
};

namespace FileTypes
{
    const char* FRESErrorGetName(FRESError err);


    class resFile
    {
    public:
        FRESHeader fresHeader;
        std::vector<FRESFileSpec> files;

        resFile();
        //static FRESFile createNew(const std::string& filename); //Needs more complete implementation to be usable
        FRESError loadFromBinary(std::istream& bfres); //Only does embedded files for now
        FRESError loadFromFile(const std::string& filePath);
        /*FRESError readFile(const FRESFileSpec& file, std::string& dataOut, uint32_t offset = 0, uint32_t bytes = 0);
        FRESError patchFile(const FRESFileSpec& file, const std::string& patch, uint32_t offset);
        FRESError insertIntoFile(const FRESFileSpec& file, const std::string& data, uint32_t offset);
        FRESError appendToFile(const FRESFileSpec& file, const std::string& data);*/ //Not sure these are needed, will implement if a use comes up
        FRESError replaceEmbeddedFile(unsigned int fileIndex, const std::string& newFile);
        FRESError replaceEmbeddedFile(std::string& fileName, const std::string& newFile);
        FRESError replaceFromDir(std::string& dirPath);
        FRESError extractToDir(const std::string& dirPath); //Only does embedded files for now
        FRESError writeToStream(std::ostream& out);
        FRESError writeToFile(const std::string& outFilePath);
        //FRESError addFile(const std::string& fileName, std::istream& fileData); //Shifts too many offsets and data pieces for the (current) partial implementation
        //FRESError removeFile(const std::string& fileName); //Shifts too many offsets and data pieces for the (current) partial implementation
    private:
        //void initNew(); //Needs a more complete implementation to work
        //uint32_t insertIntoStringList(std::string str); //Needs a more complete implementation to work
        std::string fileData;
    };
}
