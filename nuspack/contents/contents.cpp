#include "contents.hpp"

#include <cstdio>

#include <nuspack/fst/FSTEntries.hpp>
#include <utility/endian.hpp>
#include <utility/file.hpp>
#include <utility/math.hpp>

using eType = Utility::Endian::Type;



uint64_t Content::GetOffsetForFileAndIncrease(const FSTEntry& entry) {
    const uint64_t oldOffset = curFileOffset;
    curFileOffset += roundUp(std::get<FSTEntry::FileEntry>(entry.entry).fileSize, ALIGNMENT_IN_CONTENT);

    return oldOffset;
}

void Content::Update(const std::vector<FSTEntry*>& entries) {
    if(entries.size() > 0) {
        this->entries = entries;
    }
}

std::string Content::PackDecrypted()
{
    std::string tmpPath(TEMP_DIR "00000000.dec");
    tmpPath += '\0';
    std::snprintf(&tmpPath[0], tmpPath.size(), TEMP_DIR "%08X.dec", id);
    std::ofstream output(tmpPath, std::ios::binary);
    
    for (FSTEntry* pEntry : entries)
    {
        if (pEntry->isFile())
        {
            const FSTEntry::FileEntry& entry = std::get<FSTEntry::FileEntry>(pEntry->entry);
            if (output.tellp() != entry.fileOffset)
            {
                //Console.WriteLine("FAILED");
            }

            std::ifstream input(pEntry->path, std::ios::binary);
            output << input.rdbuf();

            uint64_t alignedFileSize = roundUp(entry.fileSize, ALIGNMENT_IN_CONTENT);
            //cur_offset += alignedFileSize;

            uint64_t padding = alignedFileSize - entry.fileSize;
            Utility::seek(output, padding, std::ios::cur);
        }
        else
        {
            //Console.WriteLine($"[{cnt_file}/{totalCount}] Wrote folder: \"{entry.filename}\"");
        }
    }

    return tmpPath;
}

void Content::PackContentToFile(const std::filesystem::path& outputDir, Encryption& encryption) {
    //At first we need to create the decrypted file.
    const std::string& decryptedFile = PackDecrypted();

    //Calculates the hashes for the decrypted content. If the content is not hashed,
    //only the hash of the decrypted file will be calculated
    std::ifstream file(decryptedFile, std::ios::binary);
    ContentHashes contentHashes(file, isHashed());

    if (contentHashes.h3Hashes.size() > 0)
    {
        std::string h3Path((outputDir / "00000000.h3").string());
        h3Path += '\0';
        std::snprintf(&h3Path[outputDir.string().size() + 1], h3Path.size() - outputDir.string().size() - 1, "%08X.h3", id);

        std::ofstream hashOut(h3Path, std::ios::binary);
        contentHashes.SaveH3ToFile(hashOut);
    }
    hash = contentHashes.TMDHash;
    
    std::string outputFilePath((outputDir / "00000000.app").string());
    outputFilePath += '\0';
    std::snprintf(&outputFilePath[outputDir.string().size() + 1], outputFilePath.size() - outputDir.string().size() - 1, "%08X.app", id);

    std::ofstream output(outputFilePath, std::ios::binary);
    file.clear();
    size = PackEncrypted(file, output, contentHashes, encryption);
}

uint64_t Content::PackEncrypted(std::istream& input, std::ostream& output, ContentHashes& hashes, Encryption& encryption) {
    if (isHashed())
    {
        const uint64_t size = input.seekg(0, std::ios::end).tellg();
        input.seekg(0, std::ios::beg);
        encryption.EncryptFileHashed(input, output, size, *this, hashes);
    }
    else
    {
        encryption.EncryptFileWithPadding(input, id, output, PAD_LEN);
    }

    return output.tellp(); //should always be at the end?
}

uint64_t Content::writeFSTContentHeader(std::ostream& out, const uint64_t& oldOffset) {
    uint8_t unknown;
    uint64_t content_offset = oldOffset;
    uint64_t fst_content_size = size / PAD_LEN;
    uint64_t fst_content_size_written = fst_content_size;

    if (isHashed())
    {
        unknown = 2;
        fst_content_size_written -= ((fst_content_size / 64) + 1) * 2;
        if (fst_content_size_written < 0) fst_content_size_written = 0;
    }
    else
    {
        unknown = 1;
    }

    if (isFSTContent)
    {
        unknown = 0;
        if (fst_content_size == 1) fst_content_size = 0;

        content_offset += fst_content_size + 2;
    }
    else
    {
        content_offset += fst_content_size;
    }

    const uint32_t offs = Utility::Endian::toPlatform(eType::Big, static_cast<uint32_t>(oldOffset));
    const uint32_t written = Utility::Endian::toPlatform(eType::Big, static_cast<uint32_t>(fst_content_size_written));
    const uint64_t tID = Utility::Endian::toPlatform(eType::Big, parentTitleID);
    const uint32_t gID = Utility::Endian::toPlatform(eType::Big, groupID);

    const std::streamoff& start = out.tellp();
    out.write(reinterpret_cast<const char*>(&offs), sizeof(offs));
    out.write(reinterpret_cast<const char*>(&written), sizeof(written));
    out.write(reinterpret_cast<const char*>(&tID), sizeof(tID));
    out.write(reinterpret_cast<const char*>(&gID), sizeof(gID));
    out.write(reinterpret_cast<const char*>(&unknown), sizeof(unknown));
    Utility::seek(out, start + 0x20, std::ios::beg); //adds padding

    return content_offset;
}

void Content::writeToStream(std::ostream& out) {
    const auto ID_ = Utility::Endian::toPlatform(eType::Big, id);
    const auto idx_ = Utility::Endian::toPlatform(eType::Big, index);
    const auto type_ = Utility::Endian::toPlatform(eType::Big, type);
    const auto size_ = Utility::Endian::toPlatform(eType::Big, size);

    out.write(reinterpret_cast<const char*>(&ID_), sizeof(ID_));
    out.write(reinterpret_cast<const char*>(&idx_), sizeof(idx_));
    out.write(reinterpret_cast<const char*>(&type_), sizeof(type_));
    out.write(reinterpret_cast<const char*>(&size_), sizeof(size_));
    out.write(reinterpret_cast<const char*>(&hash[0]), sizeof(hash));
    Utility::seek(out, 0xC, std::ios::cur); //padding
}



std::list<Content>::iterator Contents::GetNewContent(const ContentDetails& details, const bool& isFSTContent) {
    contents.emplace_back(contents.size(), contents.size(), details.entriesFlag, details.groupID, details.parentID, details.isHashed, isFSTContent);

    std::list<Content>::iterator it = contents.begin();
    std::advance(it, contents.size() - 1);
    return it;
}

void Contents::DeleteContent(const size_t& idx) {
    std::list<Content>::iterator it = contents.begin();
    std::advance(it, idx);
    contents.erase(it);
}

void Contents::ResetFileOffsets() {
    for(Content& content : contents) {
        content.ResetFileOffset();
    }
}

void Contents::Update(const FSTEntries& entries) {
    for(Content& content : contents) {
        content.Update(entries.GetFSTEntriesByContent(content));
    }
}

void Contents::PackContents(const std::filesystem::path& out, Encryption& encryption) {
    for(Content& content : contents) {
        if(!content.isFSTContent) {
            content.PackContentToFile(out, encryption);
        }
    }
}

void Contents::writeFSTContentHeader(std::ostream& out) {
    uint64_t contentOffset = 0;
    for(Content& content : contents) {
        contentOffset = content.writeFSTContentHeader(out, contentOffset);
    }
}

void Contents::writeToStream(std::ostream& out) {
    std::vector<Content*> sortedContents(contents.size()); //sorted by index
    for(Content& content : contents) {
        sortedContents[content.index] = &content;
    }

    for(Content* content : sortedContents) {
        content->writeToStream(out);
    }
}
