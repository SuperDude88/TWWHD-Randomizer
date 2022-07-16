#include "bfres.hpp"

#include <cstring>
#include <algorithm>
#include <filesystem>
#include <variant>

#include "../utility/endian.hpp"
#include "../utility/common.hpp"
#include "../command/Log.hpp"

#include "../utility/platform.hpp"

using eType = Utility::Endian::Type;

#define SUBFILE_ERR_CHECK(func) { \
    if(const auto error = func; error != decltype(error)::NONE) {\
        ErrorLog::getInstance().log(std::string("Encountered error on line " TOSTRING(__LINE__) " of ") + __FILENAME__); \
        return FRESError::SUBFILE_ERROR;  \
    } \
}

//This code should probably be optimized a bit, minimize loops etc, it runs quite often
namespace {
    FRESError readStringTableEntry(std::istream& bfres, const unsigned int offset, std::string& out) {
        bfres.seekg(offset - 4, std::ios::beg); //BFRES stores offsets to strings directly, but the 4 bytes preceding it store length which we want
        out.clear(); //make sure string is empty before reading
        
        uint32_t len;
        if (!bfres.read(reinterpret_cast<char*>(&len), sizeof(len))) {
            LOG_ERR_AND_RETURN(FRESError::REACHED_EOF);
        }
        Utility::Endian::toPlatform_inplace(eType::Big, len);

        if (out = readNullTerminatedStr(bfres, offset); out.empty()) {
    		LOG_ERR_AND_RETURN(FRESError::REACHED_EOF); //empty string means it could not read a character from file
    	}
        else {
            out.pop_back(); //remove null terminator because it breaks length
            if (len != out.length()) LOG_ERR_AND_RETURN(FRESError::STRING_LEN_MISMATCH);
    		return FRESError::NONE;
    	}
    }

    FRESError readFRESHeader(std::istream& bfres, FRESHeader& hdr) {
        // magicFRES
        if (!bfres.read(hdr.magicFRES, 4)) LOG_ERR_AND_RETURN(FRESError::REACHED_EOF);
        if (std::strncmp(hdr.magicFRES, "FRES", 4) != 0)
        {
            LOG_ERR_AND_RETURN(FRESError::NOT_FRES);
        }
        // version number, doesn't affect much
        if (!bfres.read(reinterpret_cast<char*>(&hdr.version), sizeof(hdr.version)))
        {
            LOG_ERR_AND_RETURN(FRESError::REACHED_EOF);
        }
        // byteOrderMarker
        if (!bfres.read(reinterpret_cast<char*>(&hdr.byteOrderMarker), sizeof(hdr.byteOrderMarker)))
        {
            LOG_ERR_AND_RETURN(FRESError::REACHED_EOF);
        }
        // headerLength_0x10
        if (!bfres.read(reinterpret_cast<char*>(&hdr.headerLength_0x10), sizeof(hdr.headerLength_0x10)))
        {
            LOG_ERR_AND_RETURN(FRESError::REACHED_EOF);
        }
        // fileSize
        if (!bfres.read(reinterpret_cast<char*>(&hdr.fileSize), sizeof(hdr.fileSize)))
        {
            LOG_ERR_AND_RETURN(FRESError::REACHED_EOF);
        }
        // fileAlignment
        if (!bfres.read(reinterpret_cast<char*>(&hdr.fileAlignment), sizeof(hdr.fileAlignment)))
        {
            LOG_ERR_AND_RETURN(FRESError::REACHED_EOF);
        }
        // nameOffset
        if (!bfres.read(reinterpret_cast<char*>(&hdr.nameOffset), sizeof(hdr.nameOffset)))
        {
            LOG_ERR_AND_RETURN(FRESError::REACHED_EOF);
        }
        // stringTableSize
        if (!bfres.read(reinterpret_cast<char*>(&hdr.stringTableSize), sizeof(hdr.stringTableSize)))
        {
            LOG_ERR_AND_RETURN(FRESError::REACHED_EOF);
        }
        // stringTableOffset
        if (!bfres.read(reinterpret_cast<char*>(&hdr.stringTableOffset), sizeof(hdr.stringTableOffset)))
        {
            LOG_ERR_AND_RETURN(FRESError::REACHED_EOF);
        }
        // Index group info
        for (int i = 0; i < 12; i++) {
            if (!bfres.read(reinterpret_cast<char*>(&hdr.groupOffsets[i]), sizeof(hdr.groupOffsets[i])))
            {
                LOG_ERR_AND_RETURN(FRESError::REACHED_EOF);
            }
            Utility::Endian::toPlatform_inplace(eType::Big, hdr.groupOffsets[i]);
            //calculate global offset
        }
        for (int i = 0; i < 12; i++) {
            if (!bfres.read(reinterpret_cast<char*>(&hdr.groupCounts[i]), sizeof(hdr.groupCounts[i])))
            {
                LOG_ERR_AND_RETURN(FRESError::REACHED_EOF);
            }
            Utility::Endian::toPlatform_inplace(eType::Big, hdr.groupCounts[i]);
        }
        // userPointer
        if (!bfres.read(reinterpret_cast<char*>(&hdr.userPointer), sizeof(hdr.userPointer)))
        {
            LOG_ERR_AND_RETURN(FRESError::REACHED_EOF);
        }
        Utility::Endian::toPlatform_inplace(eType::Big, hdr.byteOrderMarker);
        Utility::Endian::toPlatform_inplace(eType::Big, hdr.headerLength_0x10);
        Utility::Endian::toPlatform_inplace(eType::Big, hdr.fileSize);
        Utility::Endian::toPlatform_inplace(eType::Big, hdr.fileAlignment);
        Utility::Endian::toPlatform_inplace(eType::Big, hdr.nameOffset);
        Utility::Endian::toPlatform_inplace(eType::Big, hdr.stringTableSize);
        Utility::Endian::toPlatform_inplace(eType::Big, hdr.stringTableOffset);
        Utility::Endian::toPlatform_inplace(eType::Big, hdr.userPointer);
        
        return FRESError::NONE;
    }

    //Partial implementation, only process embedded files for now

    std::variant<FRESError, std::vector<FRESFileSpec>> readEmbedded(std::istream& bfres, FRESHeader& hdr) {
        if (hdr.groupCounts[11] != 0) {
            std::vector<FRESFileSpec> fileList;
            bfres.seekg(0x20 + (11 * 0x4) + hdr.groupOffsets[11], std::ios::beg);
            GroupHeader group;
            if (!bfres.read(reinterpret_cast<char*>(&group.groupLength), sizeof(group.groupLength))) {
                LOG_ERR_AND_RETURN(FRESError::REACHED_EOF);
            }
            if (!bfres.read(reinterpret_cast<char*>(&group.entryCount), sizeof(group.entryCount))) {
                LOG_ERR_AND_RETURN(FRESError::REACHED_EOF);
            }

            Utility::Endian::toPlatform_inplace(eType::Big, group.groupLength);
            Utility::Endian::toPlatform_inplace(eType::Big, group.entryCount);

            for (int32_t entry_index = 0; entry_index < group.entryCount; entry_index++) {
                GroupEntry entry;
                bfres.seekg(0x20 + 11 * 0x4 + hdr.groupOffsets[11] + 0x8 + 0x10 * (1 + entry_index), std::ios::beg);
                entry.location = bfres.tellg();
                if (!bfres.read(reinterpret_cast<char*>(&entry.searchValue), sizeof(entry.searchValue))) {
                    LOG_ERR_AND_RETURN(FRESError::REACHED_EOF);
                }
                if (!bfres.read(reinterpret_cast<char*>(&entry.leftIndex), sizeof(entry.leftIndex))) {
                    LOG_ERR_AND_RETURN(FRESError::REACHED_EOF);
                }
                if (!bfres.read(reinterpret_cast<char*>(&entry.rightIndex), sizeof(entry.rightIndex))) {
                    LOG_ERR_AND_RETURN(FRESError::REACHED_EOF);
                }
                if (!bfres.read(reinterpret_cast<char*>(&entry.namePointer), sizeof(entry.namePointer))) {
                    LOG_ERR_AND_RETURN(FRESError::REACHED_EOF);
                }
                if (!bfres.read(reinterpret_cast<char*>(&entry.dataPointer), sizeof(entry.dataPointer))) {
                    LOG_ERR_AND_RETURN(FRESError::REACHED_EOF);
                }

                Utility::Endian::toPlatform_inplace(eType::Big, entry.searchValue);
                Utility::Endian::toPlatform_inplace(eType::Big, entry.leftIndex);
                Utility::Endian::toPlatform_inplace(eType::Big, entry.rightIndex);
                Utility::Endian::toPlatform_inplace(eType::Big, entry.namePointer);
                Utility::Endian::toPlatform_inplace(eType::Big, entry.dataPointer);
                group.entries.push_back(entry);

                FRESFileSpec file;
                LOG_AND_RETURN_IF_ERR(readStringTableEntry(bfres, entry.location + 0x8 + entry.namePointer, file.fileName));

                EmbeddedFileSpec fileInfo;
                bfres.seekg(entry.location + 0xC + entry.dataPointer, std::ios::beg);
                fileInfo.location = bfres.tellg();
                if (!bfres.read(reinterpret_cast<char*>(&fileInfo.dataOffset), sizeof(fileInfo.dataOffset))) {
                    LOG_ERR_AND_RETURN(FRESError::REACHED_EOF);
                }
                if (!bfres.read(reinterpret_cast<char*>(&fileInfo.fileLength), sizeof(fileInfo.fileLength))) {
                    LOG_ERR_AND_RETURN(FRESError::REACHED_EOF);
                }
                Utility::Endian::toPlatform_inplace(eType::Big, fileInfo.dataOffset);
                Utility::Endian::toPlatform_inplace(eType::Big, fileInfo.fileLength);

                file.fileLength = fileInfo.fileLength;
                file.fileOffset = fileInfo.location + fileInfo.dataOffset;

                hdr.embeddedFiles.push_back(fileInfo); //Add the embedded files to the header

                fileList.push_back(file);
            }
            return fileList;
        }

        return FRESError::GROUP_IS_EMPTY; //Not sure the best way to handle this error, it isnt really an error but still useful info that doesnt fit elsewhere
    }

    FRESError writeFRESHeader(std::ostream& out, FRESHeader& hdr) {
        auto BOM = Utility::Endian::toPlatform(eType::Big, hdr.byteOrderMarker);
        auto headerLength = Utility::Endian::toPlatform(eType::Big, hdr.headerLength_0x10);
        auto fileSize = Utility::Endian::toPlatform(eType::Big, hdr.fileSize);
        auto Alignment = Utility::Endian::toPlatform(eType::Big, hdr.fileAlignment);
        auto nameOffset = Utility::Endian::toPlatform(eType::Big, hdr.nameOffset);
        auto stringTableSize = Utility::Endian::toPlatform(eType::Big, hdr.stringTableSize);
        auto stringTableOffset = Utility::Endian::toPlatform(eType::Big, hdr.stringTableOffset);
        auto userPointer = Utility::Endian::toPlatform(eType::Big, hdr.userPointer);
        out.write(hdr.magicFRES, 4);
        for (int i = 0; i < 4; i++) {
            out.write(reinterpret_cast<const char*>(&hdr.version[i]), 1);
        }
        out.write(reinterpret_cast<const char*>(&BOM), 2);
        out.write(reinterpret_cast<const char*>(&headerLength), 2);
        out.write(reinterpret_cast<const char*>(&fileSize), 4);
        out.write(reinterpret_cast<const char*>(&Alignment), 4);
        out.write(reinterpret_cast<const char*>(&nameOffset), 4);
        out.write(reinterpret_cast<const char*>(&stringTableSize), 4);
        out.write(reinterpret_cast<const char*>(&stringTableOffset), 4);
        for (int i = 0; i < 12; i++) {
            auto fileOffset = Utility::Endian::toPlatform(eType::Big, hdr.groupOffsets[i]);
            out.write(reinterpret_cast<const char*>(&fileOffset), 4);
        }
        for (int i = 0; i < 12; i++) {
            auto fileCount = Utility::Endian::toPlatform(eType::Big, hdr.groupCounts[i]);
            out.write(reinterpret_cast<const char*>(&fileCount), 2);
        }
        out.write(reinterpret_cast<const char*>(&userPointer), 4);

        for (const EmbeddedFileSpec& file : hdr.embeddedFiles) {
            auto offset = Utility::Endian::toPlatform(eType::Big, file.dataOffset);
            auto length = Utility::Endian::toPlatform(eType::Big, file.fileLength);
            out.write(reinterpret_cast<const char*>(&offset), 4);
            out.write(reinterpret_cast<const char*>(&length), 4);
        }

        return FRESError::NONE;
    }
}

namespace FileTypes {
    const char* FRESErrorGetName(FRESError err) {
        switch (err)
        {
        case FRESError::NONE:
            return "NONE";
        case FRESError::COULD_NOT_OPEN:
            return "COULD_NOT_OPEN";
        case FRESError::NOT_FRES:
            return "NOT_FRES";
        case FRESError::REACHED_EOF:
            return "REACHED_EOF";
        case FRESError::STRING_LEN_MISMATCH:
            return "STRING_LEN_MISMATCH";
        case FRESError::GROUP_IS_EMPTY:
            return "GROUP_IS_EMPTY";
        case FRESError::SUBFILE_ERROR:
            return "SUBFILE_ERROR";
        default:
            return "UNKNOWN";
        }
    }

    resFile::resFile() {
        
    }

    FRESError resFile::loadFromBinary(std::istream& bfres) {
        LOG_AND_RETURN_IF_ERR(readFRESHeader(bfres, fresHeader));

        std::variant<FRESError, std::vector<FRESFileSpec>> readFiles = readEmbedded(bfres, fresHeader);
        if (readFiles.index() == 0) {
            if (std::get<FRESError>(readFiles) != FRESError::GROUP_IS_EMPTY) { //ignore the group is empty error since we can still continue execution
                LOG_ERR_AND_RETURN(std::get<FRESError>(readFiles));
            }
        }
        files.insert(files.end(), std::get<std::vector<FRESFileSpec>>(readFiles).begin(), std::get<std::vector<FRESFileSpec>>(readFiles).end());

        // sort file specs by offset (may already be sorted by nintendo, but they don't HAVE to be in theory)
        std::sort(files.begin(), files.end(), [](const FRESFileSpec& a, const FRESFileSpec& b) {return a.fileOffset < b.fileOffset; });

        bfres.seekg(0x6C, std::ios::beg);
        fileData.resize(fresHeader.fileSize - 0x6C); //Exclude header size since it doesn't get included in this data
        if (!bfres.read(&fileData[0], fresHeader.fileSize - 0x6C)) {
            LOG_ERR_AND_RETURN(FRESError::REACHED_EOF);
        }

        bfres.seekg(fresHeader.groupOffsets[1] + 0x24, std::ios::beg);
        GroupHeader group;
        if (!bfres.read(reinterpret_cast<char*>(&group.groupLength), sizeof(group.groupLength))) {
            LOG_ERR_AND_RETURN(FRESError::REACHED_EOF);
        }
        if (!bfres.read(reinterpret_cast<char*>(&group.entryCount), sizeof(group.entryCount))) {
            LOG_ERR_AND_RETURN(FRESError::REACHED_EOF);
        }

        Utility::Endian::toPlatform_inplace(eType::Big, group.groupLength);
        Utility::Endian::toPlatform_inplace(eType::Big, group.entryCount);

        bfres.seekg(0x10, std::ios::cur); //skip root entry

        for (int32_t entry_index = 0; entry_index < group.entryCount; entry_index++) {
            GroupEntry entry;
            entry.location = bfres.tellg();
            if (!bfres.read(reinterpret_cast<char*>(&entry.searchValue), sizeof(entry.searchValue))) {
                LOG_ERR_AND_RETURN(FRESError::REACHED_EOF);
            }
            if (!bfres.read(reinterpret_cast<char*>(&entry.leftIndex), sizeof(entry.leftIndex))) {
                LOG_ERR_AND_RETURN(FRESError::REACHED_EOF);
            }
            if (!bfres.read(reinterpret_cast<char*>(&entry.rightIndex), sizeof(entry.rightIndex))) {
                LOG_ERR_AND_RETURN(FRESError::REACHED_EOF);
            }
            if (!bfres.read(reinterpret_cast<char*>(&entry.namePointer), sizeof(entry.namePointer))) {
                LOG_ERR_AND_RETURN(FRESError::REACHED_EOF);
            }
            if (!bfres.read(reinterpret_cast<char*>(&entry.dataPointer), sizeof(entry.dataPointer))) {
                LOG_ERR_AND_RETURN(FRESError::REACHED_EOF);
            }
            Utility::Endian::toPlatform_inplace(eType::Big, entry.searchValue);
            Utility::Endian::toPlatform_inplace(eType::Big, entry.leftIndex);
            Utility::Endian::toPlatform_inplace(eType::Big, entry.rightIndex);
            Utility::Endian::toPlatform_inplace(eType::Big, entry.namePointer);
            Utility::Endian::toPlatform_inplace(eType::Big, entry.dataPointer);
            group.entries.push_back(entry);
        }

        for(const GroupEntry& texEntry : group.entries) {
            bfres.seekg(texEntry.location + texEntry.dataPointer + 0xC, std::ios::beg);
            Subfiles::FTEXFile& tex = textures.emplace_back();
            SUBFILE_ERR_CHECK(tex.loadFromBinary(bfres));
        }

        return FRESError::NONE;
    }

    FRESError resFile::loadFromFile(const std::string& filePath) {
        std::ifstream file(filePath, std::ios::binary);
        if (!file.is_open()) {
            LOG_ERR_AND_RETURN(FRESError::COULD_NOT_OPEN);
        }
        return loadFromBinary(file);
    }

    FRESError resFile::replaceEmbeddedFile(const unsigned int fileIndex, const std::string& newFile) {
        const uint32_t originalLen = fresHeader.embeddedFiles[fileIndex].fileLength;

        std::ifstream inFile(newFile, std::ios::binary);
        if (!inFile.is_open()) {
            LOG_ERR_AND_RETURN(FRESError::COULD_NOT_OPEN);
        }
        std::string inData;
        inFile.seekg(0, std::ios::end);
        inData.resize(inFile.tellg());
        inFile.seekg(0, std::ios::beg);
        if (!inFile.read(&inData[0], inData.size())) {
            LOG_ERR_AND_RETURN(FRESError::REACHED_EOF);
        }

        fresHeader.embeddedFiles[fileIndex].fileLength = inData.size();

        if (fileIndex != fresHeader.embeddedFiles.size() - 1) { //Check if it is the last embedded file
            for (unsigned int i = fileIndex + 1; i < fresHeader.embeddedFiles.size(); i++) {
                fresHeader.embeddedFiles[i].dataOffset = fresHeader.embeddedFiles[i].dataOffset + (inData.size() - originalLen); //Change offset based on how much the previous file shifted it
            }
        }

        fileData.replace(fresHeader.embeddedFiles[fileIndex].dataOffset + fresHeader.embeddedFiles[fileIndex].location - 0x6C, originalLen, inData); //Offset is relative to the location it's stored in the file, location is relative to file start so we take away the header size
        return FRESError::NONE;
    }

    FRESError resFile::replaceEmbeddedFile(const std::string& fileName, const std::string& newFile) {
        GroupHeader group;
        group.groupLength = *reinterpret_cast<int32_t*>(&fileData[0x20 + (11 * 0x4) + fresHeader.groupOffsets[11] - 0x6C]);
        group.entryCount = *reinterpret_cast<int32_t*>(&fileData[0x20 + (11 * 0x4) + fresHeader.groupOffsets[11] - 0x6C] + 4);

        Utility::Endian::toPlatform_inplace(eType::Big, group.groupLength);
        Utility::Endian::toPlatform_inplace(eType::Big, group.entryCount);

        int offset = 0x20 + (11 * 0x4) + fresHeader.groupOffsets[11] - 0x6C + 8;
        for (int entry_index = 0; entry_index <= group.entryCount; entry_index++) {
            GroupEntry entry;
            entry.searchValue = *reinterpret_cast<uint32_t*>(&fileData[offset]);
            entry.leftIndex = *reinterpret_cast<uint16_t*>(&fileData[offset + 4]);
            entry.rightIndex = *reinterpret_cast<uint16_t*>(&fileData[offset + 6]);
            entry.namePointer = *reinterpret_cast<int32_t*>(&fileData[offset + 8]);
            entry.dataPointer = *reinterpret_cast<int32_t*>(&fileData[offset + 12]);

            Utility::Endian::toPlatform_inplace(eType::Big, entry.searchValue);
            Utility::Endian::toPlatform_inplace(eType::Big, entry.leftIndex);
            Utility::Endian::toPlatform_inplace(eType::Big, entry.rightIndex);
            Utility::Endian::toPlatform_inplace(eType::Big, entry.namePointer);
            Utility::Endian::toPlatform_inplace(eType::Big, entry.dataPointer);
            group.entries.push_back(entry);
            offset += 0x10;
        }

        uint32_t searchVal = group.entries[0].searchValue;
        uint32_t nextSearchVal = group.entries[group.entries[0].leftIndex].searchValue;
        uint16_t entryIndex = group.entries[0].leftIndex;

        while (searchVal > nextSearchVal) {
            searchVal = nextSearchVal;
            uint32_t charpos = searchVal >> 3;
            uint32_t bitpos = searchVal & 0b111;
            uint32_t direction;

            if (charpos >= fileName.size()) {
                direction = 0;
            } else {
                direction = (fileName[charpos] >> bitpos) & 1U; //unsigned to resolve a compiler warning, makes no functional difference
            }

            if (direction == 0) {
                entryIndex = group.entries[entryIndex].leftIndex;
            }
            else {
                entryIndex = group.entries[entryIndex].rightIndex;
            }
            nextSearchVal = group.entries[entryIndex].searchValue;
        }

        LOG_AND_RETURN_IF_ERR(replaceEmbeddedFile(entryIndex - 1, newFile)); //Entry index includes the root entry, we don't need it
        return FRESError::NONE;
    }

    FRESError resFile::replaceFromDir(const std::string& dirPath) {
        for (const auto& file : files) {
            LOG_AND_RETURN_IF_ERR(replaceEmbeddedFile(file.fileName, dirPath + file.fileName));
        }
        return FRESError::NONE;
    }

    FRESError resFile::extractToDir(const std::string& dirPath) {
        for (const FRESFileSpec& file : files) {
            std::ofstream outFile(dirPath + '/' + file.fileName, std::ios::binary);
            if (!outFile.is_open()) {
                LOG_ERR_AND_RETURN(FRESError::COULD_NOT_OPEN);
            }
            outFile.write(&fileData[file.fileOffset - 0x6C], file.fileLength); //Offsets are relative to file start but the data doesnt include the header
        }
        return FRESError::NONE;
    }
    
    FRESError resFile::writeToStream(std::ostream& out) {
        LOG_AND_RETURN_IF_ERR(writeFRESHeader(out, fresHeader));
        out.write(&fileData[fresHeader.embeddedFiles.size() * 0x8], fileData.size() - (fresHeader.embeddedFiles.size() * 0x8)); //Skip over the embedded file info at the start of the file
        uint32_t fileLen = out.tellp();
        out.seekp(0xC, std::ios::beg);
        Utility::Endian::toPlatform_inplace(eType::Big, fileLen);
        out.write(reinterpret_cast<const char*>(&fileLen), sizeof(fileLen));

        for(Subfiles::FTEXFile& tex : textures) {
            out.seekp(tex.offset);
            SUBFILE_ERR_CHECK(tex.writeToStream(out));
        }

        return FRESError::NONE;
    }

    FRESError resFile::writeToFile(const std::string& outFilePath) {
        std::ofstream outFile(outFilePath, std::ios::binary);
        if (!outFile.is_open()) {
            LOG_ERR_AND_RETURN(FRESError::COULD_NOT_OPEN);
        }
        return writeToStream(outFile);
    }

}
