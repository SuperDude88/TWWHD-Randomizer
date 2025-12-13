// Filesystem table for Wii U disk images/downloaded content
// Describes folders and files in the title, necessary for building a NUS package
// Likely an evolution of the Wii format

#pragma once

#include <fstream>

#include <utility/path.hpp>
#include <nuspack/fst/FSTEntries.hpp>
#include <nuspack/contents/contents.hpp>



enum struct [[nodiscard]] FSTError {
    NONE = 0,
    COULD_NOT_OPEN,
    REACHED_EOF,
    NOT_FST,
    UNEXPECTED_VALUE,
    UNKNOWN_ENTRY_TYPE,
    UNKNOWN,
    COUNT
};

struct FSTHeader {
    char magicFST[4] = "FST";
    uint32_t headerSize_0x20 = 0x20;
    uint32_t contentNum = 0;
    uint8_t hashDisabled = 0;
    uint8_t padding_0x00[0x13] = {0};
};

namespace FileTypes {

    const char* FSTErrorGetName(FSTError err);

    class FSTFile {
    public:
        Contents& contents;
        FSTEntries entries;
        FSTEntry& root;

        FSTFile(Contents& contents_) :
            contents(contents_),
            root(entries.GetRootEntry())
        {}
        
        void Update();
        FSTError writeToStream(std::ostream& out);
        FSTError writeToFile(const fspath& filePath);

    private:
        FSTHeader header;
    };
}
