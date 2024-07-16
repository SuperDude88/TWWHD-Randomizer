#include "fst.hpp"

#include <utility/endian.hpp>
#include <utility/file.hpp>
#include <command/Log.hpp>

using eType = Utility::Endian::Type;

#define WIIU_BLOCK_SIZE 0x8000



namespace FileTypes {

    const char* FSTErrorGetName(FSTError err) {
        switch (err) {
            case FSTError::NONE:
                return "NONE";
            case FSTError::COULD_NOT_OPEN:
                return "COULD_NOT_OPEN";
            case FSTError::REACHED_EOF:
                return "REACHED_EOF";
            case FSTError::NOT_FST:
                return "NOT_FST";
            case FSTError::UNKNOWN_ENTRY_TYPE:
                return "UNKNOWN_ENTRY_TYPE";
            case FSTError::UNEXPECTED_VALUE:
                return "UNEXPECTED_VALUE";
            default:
                return "UNKNOWN";
        }
    }

    void FSTFile::Update() {
        contents.ResetFileOffsets();
        entries.Update();
        contents.Update(entries);
        entries.GetRootEntry().root_entryCount = entries.GetFSTEntryCount();
        header.contentNum = contents.GetContentCount();
    }

    FSTError FSTFile::writeToStream(std::ostream& out) {
        const uint32_t size = Utility::Endian::toPlatform(eType::Big, header.headerSize_0x20);
        const uint32_t count = Utility::Endian::toPlatform(eType::Big, header.contentNum);

        out.write(header.magicFST, sizeof(header.magicFST));
        out.write(reinterpret_cast<const char*>(&size), sizeof(size));
        out.write(reinterpret_cast<const char*>(&count), sizeof(count));
        out.write(reinterpret_cast<const char*>(&header.hashDisabled), sizeof(header.hashDisabled));
        out.write(reinterpret_cast<const char*>(&header.padding_0x00), sizeof(header.padding_0x00));
        contents.writeFSTContentHeader(out);
        entries.writeToStream(out);

        const std::streamoff& strStart = out.tellp();
        for(const auto& [offset, string] : entries.strings.getStrings()) {
            Utility::seek(out, strStart + offset, std::ios::beg);
            out.write(&string[0], string.size());
        }

        return FSTError::NONE;
    }

    FSTError FSTFile::writeToFile(const fspath& filePath) {
        std::ofstream file(filePath, std::ios::binary);
        if (!file.is_open()) {
            LOG_ERR_AND_RETURN(FSTError::COULD_NOT_OPEN);
        }
        return writeToStream(file);
    }
}
