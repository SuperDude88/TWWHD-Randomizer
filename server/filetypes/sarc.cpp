#include "sarc.hpp"
#include "../utility/byteswap.hpp"
#include <fstream>
#include <cstring>
#include <algorithm>
#include <iterator>

constexpr uint32_t MAX_SFAT_STRING_LEN = 256;

inline bool attrToStringTableOffset(uint32_t attr, uint32_t& offset)
{
    if ((attr & 0x01000000) == 0) return false;
    offset = (attr & 0xFFFF) << 2;
    return true;
}

uint32_t SFATNameHash(const char* name, int length, uint32_t key)
{
	uint32_t result = 0;
	for(int i = 0; i < length; i++)
	{
        char c = name[i];
        // early terminate if we hit null
        if (c == '\0') return result;
		result = c + result * key;
	}
	return result;
}

SARCError readSARCHeader(std::istream& sarc, SARCHeader& hdr)
{
    // magicSARC
    if(!sarc.read(hdr.magicSARC, 4)) return SARCError::REACHED_EOF;
    if(std::strncmp(hdr.magicSARC, "SARC", 4) != 0)
    {
        return SARCError::NOT_SARC;
    }
    // headerLength_0x14
    if(!sarc.read(reinterpret_cast<char*>(&hdr.headerLength_0x14), sizeof(hdr.headerLength_0x14)))
    {
        return SARCError::REACHED_EOF;
    }
    Utility::byteswap_inplace(hdr.headerLength_0x14);
    if(hdr.headerLength_0x14 != 0x14) return SARCError::NOT_SARC;
    // byteOrderMarker
    if(!sarc.read(reinterpret_cast<char*>(&hdr.byteOrderMarker), sizeof(hdr.byteOrderMarker)))
    {
        return SARCError::REACHED_EOF;
    }
    Utility::byteswap_inplace(hdr.byteOrderMarker);
    // fileSize
    if(!sarc.read(reinterpret_cast<char*>(&hdr.fileSize), sizeof(hdr.fileSize)))
    {
        return SARCError::REACHED_EOF; 
    }
    Utility::byteswap_inplace(hdr.fileSize);
    // dataStartOffset
    if(!sarc.read(reinterpret_cast<char*>(&hdr.dataStartOffset), sizeof(hdr.dataStartOffset)))
    {
        return SARCError::REACHED_EOF; 
    }
    Utility::byteswap_inplace(hdr.dataStartOffset);
    // versionNumber_0x0100
    if(!sarc.read(reinterpret_cast<char*>(&hdr.versionNumber_0x0100), sizeof(hdr.versionNumber_0x0100)))
    {
        return SARCError::REACHED_EOF; 
    }
    Utility::byteswap_inplace(hdr.versionNumber_0x0100);
    if(hdr.versionNumber_0x0100 != 0x0100) return SARCError::NOT_SARC;
    // _unused
    if(!sarc.read(hdr._unused, sizeof(hdr._unused))) return SARCError::REACHED_EOF;
    return SARCError::NONE;
}

void writeSARCHeader(std::ostream& out, const SARCHeader& hdr)
{
    out.write(hdr.magicSARC, 4);
    auto headerLength = Utility::byteswap(hdr.headerLength_0x14);
    out.write(reinterpret_cast<char*>(&headerLength), 2);
    auto byteOrderMarker = Utility::byteswap(hdr.byteOrderMarker);
    out.write(reinterpret_cast<char*>(&byteOrderMarker), 2);
    auto fileSize = Utility::byteswap(hdr.fileSize);
    out.write(reinterpret_cast<char*>(&fileSize), 4);
    auto dataStartOffset = Utility::byteswap(hdr.dataStartOffset);
    out.write(reinterpret_cast<char*>(&dataStartOffset), 4);
    auto versionNumber = Utility::byteswap(hdr.versionNumber_0x0100);
    out.write(reinterpret_cast<char*>(&versionNumber), 2);
    out.write(hdr._unused, 2);
}

SARCError readSFATHeader(std::istream& sfat, SFATHeader& hdr)
{
    // magicSFAT
    if(!sfat.read(hdr.magicSFAT, sizeof(hdr.magicSFAT))) return SARCError::REACHED_EOF;
    if(std::strncmp(hdr.magicSFAT, "SFAT", 4) != 0)
    {
        return SARCError::NOT_SARC;
    }
    // headerLength
    if(!sfat.read(reinterpret_cast<char*>(&hdr.headerLength_0xC), sizeof(hdr.headerLength_0xC)))
    {
        return SARCError::REACHED_EOF;
    }
    Utility::byteswap_inplace(hdr.headerLength_0xC);
    if(hdr.headerLength_0xC != 0xC) return SARCError::NOT_SARC;
    // nodeCount
    if(!sfat.read(reinterpret_cast<char*>(&hdr.nodeCount), sizeof(hdr.nodeCount)))
    {
        return SARCError::REACHED_EOF;
    }
    Utility::byteswap_inplace(hdr.nodeCount);
    // hashKey_0x65
    if(!sfat.read(reinterpret_cast<char*>(&hdr.hashKey_0x65), sizeof(hdr.hashKey_0x65)))
    {
        return SARCError::REACHED_EOF;
    }
    Utility::byteswap_inplace(hdr.hashKey_0x65);
    if(hdr.hashKey_0x65 != 0x65) return SARCError::NOT_SARC;
    return SARCError::NONE;
}

void writeSFATHeader(std::ostream& out, const SFATHeader& hdr)
{
    out.write(hdr.magicSFAT, 4);
    auto headerLength = Utility::byteswap(hdr.headerLength_0xC);
    out.write(reinterpret_cast<char*>(&headerLength), 2);
    auto nodeCount = Utility::byteswap(hdr.nodeCount);
    out.write(reinterpret_cast<char*>(&nodeCount), 2);
    auto hashKey = Utility::byteswap(hdr.hashKey_0x65);
    out.write(reinterpret_cast<char*>(&hashKey), 4);
}

SARCError readSFATNode(std::istream& sfat, SFATNode& node)
{
    // fileNameHash
    if(!sfat.read(reinterpret_cast<char*>(&node.fileNameHash), sizeof(node.fileNameHash))) 
    {
        return SARCError::REACHED_EOF;
    }
    Utility::byteswap_inplace(node.fileNameHash);
    // fileAttributes
    if(!sfat.read(reinterpret_cast<char*>(&node.fileAttributes), sizeof(node.fileAttributes)))
    {
        return SARCError::REACHED_EOF;
    }
    Utility::byteswap_inplace(node.fileAttributes);
    // nodeDataOffset
    if(!sfat.read(reinterpret_cast<char*>(&node.nodeDataOffset), sizeof(node.nodeDataOffset)))
    {
        return SARCError::REACHED_EOF;
    }
    Utility::byteswap_inplace(node.nodeDataOffset);
    // nodeDataEnd
    if(!sfat.read(reinterpret_cast<char*>(&node.nodeDataEnd), sizeof(node.nodeDataEnd)))
    {
        return SARCError::REACHED_EOF;
    }
    Utility::byteswap_inplace(node.nodeDataEnd);
    // data end should be after offset
    if(node.nodeDataEnd <= node.nodeDataOffset)
    {
        return SARCError::NOT_SARC;
    }
    return SARCError::NONE;
}

void writeSFATNode(std::ostream& out, const SFATNode& node)
{
    auto fileNameHash = Utility::byteswap(node.fileNameHash);
    out.write(reinterpret_cast<char*>(&fileNameHash), 4);
    auto fileAttributes = Utility::byteswap(node.fileAttributes);
    out.write(reinterpret_cast<char*>(&fileAttributes), 4);
    auto nodeDataOffset = Utility::byteswap(node.nodeDataOffset);
    out.write(reinterpret_cast<char*>(&nodeDataOffset), 4);
    auto nodeDataEnd = Utility::byteswap(node.nodeDataEnd);
    out.write(reinterpret_cast<char*>(&nodeDataEnd), 4);
}

SARCError readSFATFileNameTableHeader(std::istream& sfat, SFATFileNameTableHeader& hdr)
{
    // magicSFNT
    if(!sfat.read(hdr.magicSFNT, sizeof(hdr.magicSFNT))) return SARCError::REACHED_EOF;
    if(strncmp(hdr.magicSFNT, "SFNT", 4) != 0) return SARCError::NOT_SARC;
    // headerLength_0x8
    if(!sfat.read(reinterpret_cast<char*>(&hdr.headerLength_0x8), sizeof(hdr.headerLength_0x8)))
    {
        return SARCError::REACHED_EOF;
    }
    Utility::byteswap_inplace(hdr.headerLength_0x8);
    if(hdr.headerLength_0x8 != 0x8) return SARCError::NOT_SARC;
    // _unused
    if(!sfat.read(hdr._unused, sizeof(hdr._unused))) return SARCError::REACHED_EOF;
    return SARCError::NONE;
}

void writeSFATFileNameTableHeader(std::ostream& out, const SFATFileNameTableHeader& hdr)
{
    out.write(hdr.magicSFNT, 4);
    auto headerLength = Utility::byteswap(hdr.headerLength_0x8);
    out.write(reinterpret_cast<char*>(&headerLength), 2);
    out.write(hdr._unused, 2);
}

SARCError readSFATStringTable(std::istream& sfat, std::vector<StringTableEntry>& out, uint32_t tableOffset, uint32_t dataOffset)
{
    char buffer[MAX_SFAT_STRING_LEN];
    while(sfat.tellg() < dataOffset)
    {
        uint32_t stringOffset = static_cast<uint32_t>(sfat.tellg()) - tableOffset;
        if(!sfat.getline(buffer, MAX_SFAT_STRING_LEN, '\0'))
        {
            return SARCError::STRING_TOO_LONG;
        }
        // n.b. getline() extracts and counts the delimiter character, but 
        // doesn't put it in buffer. A null character is also always added
        // so in our case we do gcount() - 1 to not include the null
        std::string str(buffer, sfat.gcount() - 1);
        out.emplace_back(StringTableEntry{stringOffset, str});
        char next = 0;
        auto end = sfat.tellg();
        end += 8;
        // continue until non-null character, as strings may be 
        // byte aligned with zero pads, but only go as far as 8 bytes
        while(sfat.tellg() < end && (next = sfat.get()) == '\0');
        if(next != '\0')
        {
            sfat.unget();
        }
    }
    return SARCError::NONE;
}

namespace FileTypes {
    const char* SARCErrorGetName(SARCError err)
    {
        switch(err)
        {
        case SARCError::NONE:
            return "NONE";
        case SARCError::COULD_NOT_OPEN:
            return "COULD_NOT_OPEN";
        case SARCError::NOT_SARC:
            return "NOT_SARC";
        case SARCError::REACHED_EOF:
            return "REACHED_EOF";
        case SARCError::STRING_TOO_LONG:
            return "STRING_TOO_LONG";
        case SARCError::BAD_NODE_ATTR:
            return "BAD_NODE_ATTR";
        case SARCError::STRING_NOT_FOUND:
            return "STRING_NOT_FOUND";
        case SARCError::FILENAME_HASH_MISMATCH:
            return "FILENAME_HASH_MISMATCH";
        case SARCError::HEADER_DATA_NOT_LOADED:
            return "HEADER_DATA_NOT_LOADED";
        case SARCError::FILE_DATA_NOT_LOADED:
            return "FILE_DATA_NOT_LOADED";
        case SARCError::SARC_NOT_EMPTY:
            return "SARC_NOT_EMPTY";
        case SARCError::SARC_IS_EMTPY:
            return "SARC_IS_EMPTY";
        case SARCError::UNKNOWN:
            return "UNKNOWN";
        case SARCError::COUNT:
            return "COUNT";
        default:
            return "UNKNOWN";
        }
    }

    SARCFile::SARCFile()
    {

    }

    void SARCFile::initNew()
    {
        isEmpty = false;
        // init sarc header
        std::memcpy(sarcHeader.magicSARC, "SARC", 4);
        sarcHeader.headerLength_0x14 = 0x14;
        sarcHeader.byteOrderMarker = 0xFEFF;
        sarcHeader.fileSize = 0;
        sarcHeader.dataStartOffset = 0;
        sarcHeader.versionNumber_0x0100 = 0x0100;
        std::memset(sarcHeader._unused, '\0', 2);
        // init SFAT Header
        std::memcpy(sfatHeader.magicSFAT, "SFAT", 4);
        sfatHeader.headerLength_0xC = 0xC;
        sfatHeader.nodeCount = 0;
        sfatHeader.hashKey_0x65 = 0x65;
        // init SFNT header
        std::memcpy(sfntHeader.magicSFNT, "SFNT", 4);
        sfntHeader.headerLength_0x8 = 0x8;
        std::memset(sfntHeader._unused, '\0', 2);
    }

    SARCFile SARCFile::createNew(const std::string& filename)
    {
        SARCFile newSARC{};
        newSARC.initNew();
        return newSARC;
    }

    SARCError SARCFile::loadFromBinary(std::istream& sarc, bool headerOnly)
    {
        SARCError err = SARCError::NONE;
        if((err = readSARCHeader(sarc, sarcHeader)) != SARCError::NONE)
        {
            return err;
        }
        // read SARC File Allocation Table
        if((err = readSFATHeader(sarc, sfatHeader)) != SARCError::NONE)
        {
            return err;
        }
        // read SFAT nodes
        nodes.resize(sfatHeader.nodeCount);
        for (int nodeIdx = 0; nodeIdx < sfatHeader.nodeCount; nodeIdx++)
        {
            if((err = readSFATNode(sarc, nodes[nodeIdx])) != SARCError::NONE)
            {
                return err;
            }
        }
        if((err = readSFATFileNameTableHeader(sarc, sfntHeader)) != SARCError::NONE)
        {
            return err;
        }
        uint32_t stringTableStartOffset = sarc.tellg();
        uint32_t fileDataStartOffset = sarcHeader.dataStartOffset;
        if((err = readSFATStringTable(sarc, stringTable, stringTableStartOffset, fileDataStartOffset)) != SARCError::NONE)
        {
            return err;
        }

        for(const auto& node : nodes)
        {
            SARCFileSpec spec;
            uint32_t strOffset;
            if (!attrToStringTableOffset(node.fileAttributes, strOffset))
            {
                return SARCError::BAD_NODE_ATTR;
            }
            // linear search for string table entry with matching offset
            auto match = std::find_if(
                stringTable.begin(), 
                stringTable.end(), 
                [&](const StringTableEntry& e){return e.offset == strOffset;}
            );
            if (match == stringTable.end())
            {
                return SARCError::STRING_NOT_FOUND;
            }
            auto hash = SFATNameHash(match->str.data(), match->str.size(), sfatHeader.hashKey_0x65);
            if (hash != node.fileNameHash)
            {
                return SARCError::FILENAME_HASH_MISMATCH;
            }
            spec.fileName = match->str;
            spec.fileOffset = node.nodeDataOffset;
            spec.fileLength = node.nodeDataEnd - node.nodeDataOffset;
            files.emplace_back(spec);
        }

        // sort file specs by offset (may already be sorted by nintendo, but they
        // don't HAVE to be in theory)
        std::sort(
            files.begin(), 
            files.end(), 
            [](const SARCFileSpec& l, const SARCFileSpec& r) {return l.fileOffset < r.fileOffset;}
        );

        sarc.seekg(sarcHeader.dataStartOffset);
        if(headerOnly)
        {
            // exit early case for only reading header
            isEmpty = false;
            return SARCError::NONE;
        }

        uint32_t remainingData = sarcHeader.fileSize - sarcHeader.dataStartOffset;
        fileData.resize(remainingData);
        if(!sarc.read(&fileData[0], remainingData))
        {
            return SARCError::REACHED_EOF;
        }
        auto bytesRead = sarc.gcount();
        if(bytesRead != remainingData)
        {
            return SARCError::REACHED_EOF;
        }

        isEmpty = false;
        return SARCError::NONE;
    }

    SARCError SARCFile::loadFromFile(const std::string& filePath, bool headerOnly)
    {
        std::ifstream file(filePath, std::ios::binary);
        if(!file.is_open())
        {
            return SARCError::COULD_NOT_OPEN;
        }
        return loadFromBinary(file, headerOnly);
    }

    std::vector<SARCFileSpec> SARCFile::getFileList()
    {
        return files;
    }

    SARCError SARCFile::readFile(const SARCFileSpec& file, std::string& dataOut, uint32_t offset, uint32_t bytes)
    {
        if (isEmpty) return SARCError::HEADER_DATA_NOT_LOADED;
        if (fileData.size() == 0) return SARCError::FILE_DATA_NOT_LOADED;
        if (bytes == 0)
        {
            bytes = file.fileLength;
        }
        uint32_t totalOffset = file.fileOffset + offset;
        uint32_t endOffset = totalOffset + bytes;
        if (endOffset > fileData.size() || bytes > file.fileLength)
        {
            return SARCError::REACHED_EOF;
        }
        dataOut.append(fileData, totalOffset, bytes);
        return SARCError::NONE;
    }

    SARCError SARCFile::patchFile(const SARCFileSpec& file, const std::string& patch, uint32_t offset)
    {
        return SARCError::UNKNOWN;
    }

    SARCError SARCFile::insertIntoFile(const SARCFileSpec& file, const std::string& data, uint32_t offset)
    {
        return SARCError::UNKNOWN; 
    }

    SARCError SARCFile::appendToFile(const SARCFileSpec& file, const std::string& data)
    {
        return SARCError::UNKNOWN;
    }

    SARCError SARCFile::writeToStream(std::ostream& out, uint32_t minDataOffset)
    {
        if (isEmpty) return SARCError::HEADER_DATA_NOT_LOADED;
        // determine current output file size and start of data segment
        uint32_t fileSize = 0, dataStartOffset = 0;
        dataStartOffset += sarcHeader.headerLength_0x14;
        dataStartOffset += sfatHeader.headerLength_0xC;
        dataStartOffset += sfatHeader.nodeCount * sizeof(SFATNode);
        dataStartOffset += sfntHeader.headerLength_0x8;
        // nintendo seems to pad file offsets to nearest 0x2000, so we can't just
        // sum filesizes
        for (const auto& fileSpec : files)
        {
            uint32_t filenameLen = fileSpec.fileName.size() + 1;
            filenameLen = Utility::byte_align<uint32_t>(filenameLen);
            dataStartOffset += filenameLen;
        }

        // nintendo seems to like enforcing a minimum data offset;
        // possibly to make computing the string table stuff simpler
        if (dataStartOffset < minDataOffset)
        {
            dataStartOffset = minDataOffset;
        }
        fileSize = dataStartOffset;
        const auto& lastFile = files.back();
        fileSize += lastFile.fileOffset + lastFile.fileLength;

        sarcHeader.fileSize = fileSize;
        sarcHeader.dataStartOffset = dataStartOffset;

        writeSARCHeader(out, sarcHeader);
        writeSFATHeader(out, sfatHeader);

        for(const auto& node : nodes)
        {
            writeSFATNode(out, node);
        }

        writeSFATFileNameTableHeader(out, sfntHeader);
        // write out file names
        for (const auto& fileSpec : files)
        {
            std::string byteAligned{fileSpec.fileName};
            // add null terminator so calc is correct
            byteAligned.append(1, '\0');
            uint32_t toAppend = Utility::byte_align_offset<uint32_t>(byteAligned.size());
            byteAligned.append(toAppend, '\0');
            if(!out.write(byteAligned.data(), byteAligned.size()))
            {
                return SARCError::REACHED_EOF;
            }
        }

        uint32_t padSize = dataStartOffset;
        padSize -= out.tellp();
        std::fill_n(std::ostream_iterator<char>(out), padSize, '\0');
        if(!out.write(fileData.data(), fileData.size()))
        {
            return SARCError::REACHED_EOF;
        }

        return SARCError::NONE;
    }

    SARCError SARCFile::writeToFile(const std::string& outFilePath)
    {
        std::ofstream outFile(outFilePath, std::ios::binary);
        if(!outFile.is_open())
        {
            return SARCError::COULD_NOT_OPEN;
        }
        return writeToStream(outFile);
    }

    uint32_t SARCFile::insertIntoStringList(std::string str)
    {
        StringTableEntry newEntry;
        uint16_t endOfPrev = 0;
        newEntry.str = str;
        if (!stringTable.empty())
        {
            auto maxOffEntryIt = std::max_element(
                stringTable.begin(),
                stringTable.end(),
                [](const StringTableEntry& l, const StringTableEntry& r) {return l.offset < r.offset; }
            );
            const auto& maxOffEntry = *maxOffEntryIt;
            endOfPrev = maxOffEntry.offset;
            endOfPrev += maxOffEntry.str.length();
        }
        // add whatever was needed before for 4 byte alignment
        endOfPrev += Utility::byte_align_offset<uint32_t>(endOfPrev);
        newEntry.offset = endOfPrev;
        stringTable.push_back(newEntry);
        return newEntry.offset;
    }

    SARCError SARCFile::extractToDir(const std::string& dirPath)
    {
        if (isEmpty) return SARCError::HEADER_DATA_NOT_LOADED;
        if (fileData.size() == 0) return SARCError::FILE_DATA_NOT_LOADED;
        for (const auto& file : files)
        {
            // test on msvc for portability
            std::ofstream outFile(dirPath + '/' + file.fileName, std::ios::binary);
            if (!outFile.is_open())
            {
                return SARCError::COULD_NOT_OPEN;
            }
            outFile.write(&fileData[file.fileOffset], file.fileLength);
        }
        return SARCError::NONE;
    }

    // note: filename must not be null terminated (I mean, it is a std::string)
    SARCError SARCFile::addFile(const std::string& addFileName, std::istream& inputData)
    {
        SFATNode newNode{};
        SARCFileSpec newSpec{};
        uint32_t fileLength = 0, dataPadSize = 0;

        if (isEmpty)
        {
            return SARCError::SARC_IS_EMTPY;
        }

        sfatHeader.nodeCount += 1;
        newNode.fileNameHash = SFATNameHash(addFileName.data(), addFileName.length(), sfatHeader.hashKey_0x65);
        auto newEntryOffset = insertIntoStringList(addFileName);
        // offset is divided by 4 in file attributes
        newEntryOffset /= 4;
        newNode.fileAttributes = 0x01000000 | newEntryOffset;
        if (sfatHeader.nodeCount == 1)
        {
            newNode.nodeDataOffset = 0;
            dataPadSize = 0;
        }
        else
        {
            // we guarantee the file spec list is sorted during reading
            const auto& trailingFile = files.back();
            newNode.nodeDataOffset = trailingFile.fileOffset + trailingFile.fileLength;
            // byte align, although the "standard" is non-specific
            newNode.nodeDataOffset = Utility::byte_align<uint32_t>(newNode.nodeDataOffset, 0x2000);
            // will need to pad out file data to the offset point to copy file data
            dataPadSize = newNode.nodeDataOffset - fileData.size();
        }
        // append padding zeros to data
        fileData.append(dataPadSize, '\0');

        if (!inputData.seekg(0, std::ios::end))
        {
            return SARCError::REACHED_EOF;
        }
        fileLength = inputData.tellg();
        if (!inputData.seekg(0, std::ios::beg))
        {
            return SARCError::REACHED_EOF;
        }
        newNode.nodeDataEnd = newNode.nodeDataOffset + fileLength;
        nodes.push_back(newNode);

        // see notes of https://en.cppreference.com/w/cpp/container/vector/resize if 
        // this becomes a bottleneck
        fileData.resize(newNode.nodeDataEnd);
        if (!inputData.read(&fileData[newNode.nodeDataOffset], fileLength))
        {
            return SARCError::REACHED_EOF;
        }

        newSpec.fileName = addFileName;
        newSpec.fileOffset = newNode.nodeDataOffset;
        newSpec.fileLength = fileLength;
        files.push_back(newSpec);
        return SARCError::NONE;
    }

    SARCError SARCFile::removeFile(const std::string& fileName)
    {
        return SARCError::UNKNOWN;
    }
}
