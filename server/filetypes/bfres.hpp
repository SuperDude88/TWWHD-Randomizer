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

//This was for parsing the individual subfiles to extract them, but rando only needs embedded files currently
//Only embedded files will be implemented at first to simplify/accelerate dev
/*namespace BFRES_Subfiles {
    namespace FMDL {
        struct FMDLHeader {
            char magicFMDL[4];
            int32_t nameOffset;
            int32_t filePathOffset; //Looks to be stripped in some or all WWHD files
            int32_t FSKLOffset;
            int32_t FVTXArrayOffset;
            int32_t FSHPIndexGroup;
            int32_t FMATIndexGroup;
            int32_t UserIndexGroup;
            uint16_t FVTXCount;
            uint16_t FSHPCount;
            uint16_t FMATCount;
            uint16_t UserCount;
            uint32_t vertexCount;
            uint32_t userPointer;
        };
        struct FVTXHeader {
            char magicFVTX[4];
            uint8_t attributeCount;
            uint8_t bufferCount;
            uint16_t arrayIndex;
            uint32_t vertexCount;
            uint8_t skinCount;
            char padding[3];
            int32_t attributeArray;
            int32_t attributeIndexGroup;
            int32_t bufferArray;
            uint32_t userPointer;
        };
        struct FVTXAttributes {
            int32_t nameOffset;
            uint8_t bufferIndex;
            char padding;
            uint16_t bufferOffset;
            uint32_t dataFormat;
        };
        typedef uint8_t unorm_8;
        typedef uint8_t unorm_8_8[2];
        typedef uint16_t unorm_16_16[2];
        typedef uint8_t unorm_8_8_8_8[4];
        typedef uint8_t uint_8;
        typedef uint8_t uint_8_8[2];
        typedef uint8_t uint_8_8_8_8[4];
        typedef int8_t snorm_8;
        typedef int8_t snorm_8_8[2];
        typedef int16_t snorm_16_16[2];
        typedef int8_t snorm_8_8_8_8[4];
        typedef int10_t snorm_10_10_10_2[3];
        typedef int8_t sint_8;
        typedef int8_t sint_8_8[2];
        typedef int8_t sint_8_8_8_8[4];
        typedef float float_32;
        typedef _Float16 float_16_16[2]; //GCC might support this type, not sure for Visual Studio
        typedef float float_32_32[2];
        typedef _Float16 float_16_16_16_16[4]; //Same as last float16
        typedef float float_32_32_32[3];
        typedef float float_32_32_32_32[4];
        struct buffer {
            uint32_t dataPointer;
            uint32_t bufferSize;
            uint32_t bufferHandle;
            uint16_t stride;
            uint16_t count_0x1;
            uint32_t contextPointer;
            int32_t dataOffset;
        };
        struct FMAT {
            char magicFMAT[4];
            int32_t nameOffset;
            uint32_t flags;
            uint16_t sectionIndex;
            uint16_t renderInfParamCount;
            uint8_t texReferenceCount;
            uint8_t texSamplerCount;
            uint16_t materialParamCount;
            uint16_t volatileParamCount;
            uint16_t matParamLen;
            uint16_t rawParamLen;
            uint16_t userEntryCount;
            int32_t renderInfParamGroup;
            int32_t renderStateOffset;
            int32_t shaderAssignOffset;
            int32_t texReferenceOffset;
            int32_t texSamplerOffset;
            int32_t texSamplerGroup;
            int32_t matParamOffset;
            int32_t matParamGroup;
            int32_t matParamData;
            int32_t userGroup;
            int32_t volatileFlagsOffset;
            int32_t userPointer;
        };
    }
}
*/

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
        FRESError replaceEmbeddedFile(std::string fileName, const std::string& newFile);
        FRESError replaceFromDir(std::string dirPath);
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