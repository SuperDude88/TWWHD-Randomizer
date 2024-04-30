//Format is a part of NintendoWare::g3d (a model rendering library)
//BFRES files contain several subfiles with model, texture, and animation data
//They can also have embedded files of any type

#pragma once

#include <string>
#include <vector>
#include <filetypes/subfiles/bftex.hpp>
#include <filetypes/baseFiletype.hpp>



enum struct [[nodiscard]] FRESError
{
    NONE = 0,
    COULD_NOT_OPEN,
    NOT_FRES,
    REACHED_EOF,
    STRING_LEN_MISMATCH,
    GROUP_IS_EMPTY,
    SUBFILE_ERROR,
    UNKNOWN,
    COUNT
};

//Only embedded files are currently implemented

struct EmbeddedFileSpec {
    unsigned int location; //Used to deal with relative offsets, not a part of the file data
    
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
    unsigned int location; //Used to deal with relative offsets, not a part of the file data
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

namespace FileTypes
{
    const char* FRESErrorGetName(FRESError err);


    class resFile final : public FileType {
    public:
        struct FileSpec
        {
            // name of the file from the string table
            std::string fileName;
            // offset relative to start of FRES file
            uint32_t fileOffset;
            // length of file past offset
            uint32_t fileLength;
        };

        struct StringTableEntry
        {
            uint32_t length;
            std::string str;
        };

        FRESHeader fresHeader;
        std::vector<Subfiles::FTEXFile> textures;
        std::vector<FileSpec> files;
        std::string fileData;

        resFile();
        //static FRESFile createNew(); //Needs more complete implementation to be usable
        FRESError loadFromBinary(std::istream& bfres); //Only does embedded files for now
        FRESError loadFromFile(const std::string& filePath);
        FRESError replaceEmbeddedFile(const std::string& fileName, const std::string& newFilename);
        FRESError replaceEmbeddedFile(const std::string& fileName, std::stringstream& newData);
        FRESError replaceFromDir(const std::string& dirPath);
        FRESError extractToDir(const std::string& dirPath) const; //Only does embedded files for now
        FRESError writeToStream(std::ostream& out);
        FRESError writeToFile(const std::string& outFilePath);
    private:
        FRESError replaceEmbeddedFile(const unsigned int fileIndex, std::istream& newFile);
        FRESError replaceEmbeddedFile(const unsigned int fileIndex, std::stringstream& newFile);
        void initNew() override {} //Needs a more complete implementation to work
    };
}
