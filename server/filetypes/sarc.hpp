#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <iostream>

constexpr uint32_t NINTENDO_MIN_OFFSET = 8192;

struct SARCHeader
{
    char magicSARC[4];
    uint16_t headerLength_0x14;
    uint16_t byteOrderMarker;
    uint32_t fileSize;
    uint32_t dataStartOffset;
    uint16_t versionNumber_0x0100;
    char _unused[2];
};

struct SFATHeader
{
    char magicSFAT[4];
    uint16_t headerLength_0xC;
    uint16_t nodeCount;
    uint32_t hashKey_0x65;
};

struct SFATNode
{
    uint32_t fileNameHash;
    // also contains name as offset into string table in ls half word
    uint32_t fileAttributes;
    uint32_t nodeDataOffset;
    uint32_t nodeDataEnd;
};

// N.B. strings in this table are 4-byte aligned null terminated
struct SFATFileNameTableHeader
{
    char magicSFNT[4];
    uint16_t headerLength_0x8;
    char _unused[2];
};

struct SARCFileSpec
{
    /// name of the file from the SARC string table
    std::string fileName;
    /// ofset relative to start of SARC file
    uint32_t fileOffset;
    /// length of file past offset
    uint32_t fileLength;
};

enum struct SARCError
{
    NONE = 0,
    COULD_NOT_OPEN,
    NOT_SARC,
    REACHED_EOF,
    STRING_TOO_LONG,
    BAD_NODE_ATTR,
    STRING_NOT_FOUND,
    FILENAME_HASH_MISMATCH,
    HEADER_DATA_NOT_LOADED,
    FILE_DATA_NOT_LOADED,
    SARC_NOT_EMPTY,
    SARC_IS_EMTPY,
    UNKNOWN,
    COUNT
};

struct StringTableEntry 
{
    uint32_t offset;
    std::string str;
};

namespace FileTypes
{
    const char* SARCErrorGetName(SARCError err);

    /**
     * @brief Read out the file information from a sarc file contained in the given stream.
     * 
     * @param sarc a binary istream of the SARC content. Assumes the stream is currently
     * at the beginning of the file. After the execution of this function, the sarc stream
     * is guaranteed to be at the start of the data section, on success.
     * @param fileList an output argument that will be populated with the list of files 
     * contained in the given SARC archive. 
     * @param dataStartOffset an output argument that will hold the offset to the start of the 
     * data section in bytes.
     * @return SARCError an error enum value if an error ocurred, otherwise SARCError::NONE
     */
    SARCError SARCGetFileList(std::istream& sarc, 
                              std::vector<SARCFileSpec>& fileList, 
                              uint32_t& dataStartOffset);

    /**
     * @brief Read a single file from the data region of the given SARC stream
     * 
     * @param src an istream starting from the data section of a SARC file
     * @param spec the file specification to read
     * @param out an ostream to write the read data into
     * @return SARCError an error enum value if an error ocurred, otherwise SARCError::NONE
     */
    SARCError SARCReadFile(std::istream& src, const SARCFileSpec& spec, std::ostream& out);

    class SARCFile 
    {
    public:
        SARCFile();
        // initialize a new SARC file, must be empty 
        static SARCFile createNew(const std::string& filename);
        SARCError loadFromBinary(std::istream& sarc, bool headerOnly = false);
        SARCError loadFromFile(const std::string& filePath, bool headerOnly = false);
        std::vector<SARCFileSpec> getFileList();
        SARCError readFile(const SARCFileSpec& file, std::string& dataOut, uint32_t offset = 0, uint32_t bytes = 0);
        SARCError patchFile(const SARCFileSpec& file, const std::string& patch, uint32_t offset);
        SARCError insertIntoFile(const SARCFileSpec& file, const std::string& data, uint32_t offset);
        SARCError appendToFile(const SARCFileSpec& file, const std::string& data);
        SARCError writeToStream(std::ostream& out, uint32_t minDataOffset = NINTENDO_MIN_OFFSET);
        SARCError writeToFile(const std::string& outFilePath);
        SARCError extractToDir(const std::string& dirPath);
        SARCError addFile(const std::string& fileName, std::istream& fileData);
        SARCError removeFile(const std::string& fileName);
    private:
        void initNew();
        uint32_t insertIntoStringList(std::string str);

        SARCHeader sarcHeader{};
        SFATHeader sfatHeader{};
        std::vector<SFATNode> nodes{};
        SFATFileNameTableHeader sfntHeader{};
        std::vector<StringTableEntry> stringTable{};
        bool isEmpty = true;
        std::string fileData{};
        std::vector<SARCFileSpec> files{};
    };
}
