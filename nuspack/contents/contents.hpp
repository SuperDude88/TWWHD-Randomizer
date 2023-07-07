#pragma once

#include <fstream>
#include <vector>
#include <variant>
#include <sstream>
#include <list>
#include <cstdint>
#include <filesystem>

#include <nuspack/crypto/Encryption.hpp>
#include <nuspack/packaging/ContentDetails.hpp>
#include <nuspack/types.hpp>

class FSTEntry;
class FSTEntries;

class Content {
public:
    enum Type : uint16_t {
        ENCRYPTED = 0x1,
        HASHED = 0x2,
        CONTENT = 0x2000
    };

    static constexpr uint32_t ALIGNMENT_IN_CONTENT = 0x20;
    static constexpr uint32_t PAD_LEN = 0x8000;

    uint32_t id = 0;
    uint16_t index = 0;
    uint16_t type = Type::CONTENT | Type::ENCRYPTED;
    uint64_t size = 0;
    SHA1_t hash{0};

    uint64_t curFileOffset;
    uint16_t entriesFlags;
    std::vector<FSTEntry*> entries;

    uint32_t groupID;
    uint64_t parentTitleID;
    bool isFSTContent;

    inline bool isHashed() const {
        return (type & Type::HASHED) != 0;
    };
    
    inline void ResetFileOffset() {
        curFileOffset = 0;
    }

    inline bool operator==(const Content& other) const {
        return id == other.id;
    }
    inline bool operator==(const Content* other) const {
        return id == other->id;
    }

    Content() = default;
    Content(const uint32_t& ID_, const uint16_t& index_, const uint16_t& entriesFlags_, const uint32_t& groupID_, const uint64_t& parentTitleID_, const bool& isHashed_, const bool& isFSTContent_) :
        id(ID_),
        index(index_),
        entriesFlags(entriesFlags_),
        groupID(groupID_),
        parentTitleID(parentTitleID_),
        isFSTContent(isFSTContent_)
    {
        if (isHashed_) type |= Type::HASHED;
    }

    uint64_t GetOffsetForFileAndIncrease(const FSTEntry& entry);
    void Update(const std::vector<FSTEntry*>& entries);
    void PackContentToFile(const std::filesystem::path&  outputDir, Encryption& encryption);
    uint64_t writeFSTContentHeader(std::ostream& out, const uint64_t& oldOffset);
    void writeToStream(std::ostream& out);

private:
    std::variant<std::stringstream, std::ifstream> PackDecrypted();
    uint64_t PackEncrypted(std::istream& input, std::ostream& output, ContentHashes& hashes, Encryption& encryption);
};

class Contents {
public:
    Contents()
    {
        ContentDetails details(false, 0, 0, 0);
        GetNewContent(details, true);
    }

    inline Content& GetFSTContent() { return contents.front(); }
    inline uint16_t GetContentCount() { return static_cast<uint16_t>(contents.size()); }
    Content* GetNewContent(const ContentDetails& details, const bool& isFSTContent = false);
    void DeleteContent(const size_t& idx);
    void ResetFileOffsets();
    void Update(const FSTEntries& entries);
    void PackContents(const std::filesystem::path& out, Encryption& encryption);
    void writeFSTContentHeader(std::ostream& out);
    void writeToStream(std::ostream& out);

private:
    std::list<Content> contents;
};
