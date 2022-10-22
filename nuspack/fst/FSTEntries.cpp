#include "FSTEntries.hpp"

#include <nuspack/contents/contents.hpp>
#include <nuspack/packaging/fst.hpp>
#include <utility/endian.hpp>
#include <utility/file.hpp>

using eType = Utility::Endian::Type;



void FSTEntry::setContent(Content* content_) {
	content = content_;
	flags = content_->entriesFlags;
}

Content* FSTEntry::getContent() {
	return content;
}

std::vector<FSTEntry*> FSTEntry::GetFSTEntriesByContent(const Content& content_) const {
    std::vector<FSTEntry*> entries;
    if (this->content == nullptr)
    {
        if (isDir())
        {
            //Console.Error.WriteLine($"The folder \"{filename}\" is empty. Please add a dummy file to it.");
        }
        else
        {
            //Console.Error.WriteLine($"The file \"{filename}\" is not assigned to any content (.app).");
            //Console.Error.WriteLine("Please delete it or write a corresponding content rule.");
        }
        //Environment.Exit(0);
    }
    else if (this->content == content_)
    {
        entries.push_back(const_cast<FSTEntry*>(this));
    }

    for(const auto& child : children) {
        const auto& childEntries = child.GetFSTEntriesByContent(content_);
        entries.insert(entries.end(), childEntries.begin(), childEntries.end());
    }

    return entries;
}

uint32_t FSTEntry::GetEntryCount() const {
    uint32_t ret = 1;

    for(const auto& child : children) {
        ret += child.GetEntryCount();
    }

    return ret;
}

std::vector<FSTEntry*> FSTEntry::GetDirChildren() {
    std::vector<FSTEntry*> ret;
    for(FSTEntry& child : children) {
        if(child.isDir()) {
            ret.push_back(&child);
        }
    }

    return ret;
}

std::vector<FSTEntry*> FSTEntry::GetFileChildren() {
    std::vector<FSTEntry*> ret;
    for(FSTEntry& child : children) {
        if(child.isFile()) {
            ret.push_back(&child);
        }
    }

    return ret;
}

void FSTEntry::Update(uint64_t& curEntryOffset) {
    nameOffset = FileTypes::FSTFile::AddString(name);
    entryOffset = curEntryOffset;
    curEntryOffset++;

    if (isDir() && !isRoot) {
        DirEntry& dirEnt = std::get<DirEntry>(entry);
        dirEnt.parentOffset = parent->entryOffset;
    }

    if (content != nullptr && !isDir()) {
        FileEntry& fileEnt = std::get<FileEntry>(entry);
        fileEnt.fileOffset = content->GetOffsetForFileAndIncrease(*this);
    }

    for (FSTEntry& entry : children)
    {
        entry.Update(curEntryOffset);
    }
}

FSTEntry* FSTEntry::UpdateDirRefs() {
    if (!isDir()) return nullptr;

    DirEntry& dirEnt = std::get<DirEntry>(entry);
    if (parent != nullptr) dirEnt.parentOffset = parent->entryOffset;

    FSTEntry* result = nullptr;

    std::vector<FSTEntry*> dirChildren = GetDirChildren();
    size_t i = 0;
    for (FSTEntry* child : dirChildren)
    {
        DirEntry& cur_dir = std::get<DirEntry>(child->entry);
        if (dirChildren.size() > i + 1) cur_dir.nextOffset = dirChildren[i + 1]->entryOffset;

        FSTEntry* cur_result = dirChildren[i]->UpdateDirRefs();

        if (cur_result != nullptr)
        {
            FSTEntry* cur_foo = cur_result->parent;
            while (std::get<DirEntry>(cur_foo->entry).nextOffset == 0)
            {
                cur_foo = cur_foo->parent;
            }
            std::get<DirEntry>(cur_result->entry).nextOffset = std::get<DirEntry>(cur_foo->entry).nextOffset;
        }

        if (dirChildren.size() > i) result = child;
        
        i++;
    }

    return result;
}

void FSTEntry::writeToStream(std::ostream& out) {
    if (isRoot)
    {
        out.write("\x01", 1);
        Utility::seek(out, 7, std::ios::cur);

        const uint32_t count = Utility::Endian::toPlatform(eType::Big, root_entryCount);
        out.write(reinterpret_cast<const char*>(&count), sizeof(count));
        Utility::seek(out, 4, std::ios::cur);
    }
    else
    {
        const uint8_t type = typeAsByte();
        out.write(reinterpret_cast<const char*>(&type), sizeof(type));

        uint32_t nameoff = 0;
        if(Utility::Endian::isBE()) {
            nameoff = (nameOffset & 0x00FFFFFF) << 8;
        }
        else {
            nameoff = Utility::Endian::byteswap24(nameOffset);
        }
        out.write(reinterpret_cast<const char*>(&nameoff), 3); // We need to write a 24bit int (big endian)

        if (isDir())
        {
            const DirEntry& ent = std::get<DirEntry>(entry);
            const uint32_t parent = Utility::Endian::toPlatform(eType::Big, ent.parentOffset);
            const uint32_t next = Utility::Endian::toPlatform(eType::Big, ent.nextOffset);
            out.write(reinterpret_cast<const char*>(&parent), sizeof(parent));
            out.write(reinterpret_cast<const char*>(&next), sizeof(next));
        }
        else
        {
            const FileEntry& ent = std::get<FileEntry>(entry);
            const uint32_t offs = Utility::Endian::toPlatform(eType::Big, ent.fileOffset >> 5);
            const uint32_t size = Utility::Endian::toPlatform(eType::Big, ent.fileSize);
            out.write(reinterpret_cast<const char*>(&offs), sizeof(offs));
            out.write(reinterpret_cast<const char*>(&size), sizeof(size));
        }

        const uint16_t flag = Utility::Endian::toPlatform(eType::Big, flags);
        const uint16_t id = Utility::Endian::toPlatform(eType::Big, static_cast<uint16_t>(content->id));
        out.write(reinterpret_cast<const char*>(&flag), sizeof(flag));
        out.write(reinterpret_cast<const char*>(&id), sizeof(id));
    }

    for (FSTEntry& entry : children)
    {
        entry.writeToStream(out);
    }
}



void FSTEntries::Update() {
    for(FSTEntry& entry : entries)
    {
        entry.Update(curEntryOffset);
    }
    UpdateDirRefs();
}

void FSTEntries::UpdateDirRefs() {
    if (entries.size() == 0) return;

    FSTEntry::DirEntry root = entries.front().entry.emplace<FSTEntry::DirEntry>();
    root.parentOffset = 0;
    root.nextOffset = curEntryOffset;
    FSTEntry* lastdir = entries.front().UpdateDirRefs();
    if (lastdir != nullptr)
    {
        std::get<FSTEntry::DirEntry>(lastdir->entry).nextOffset = curEntryOffset;
    }
}

std::vector<FSTEntry*> FSTEntries::GetFSTEntriesByContent(const Content& content) const {
    std::vector<FSTEntry*> ret;

    for(const FSTEntry& entry : entries) {
        const std::vector<FSTEntry*>& temp = entry.GetFSTEntriesByContent(content);
        ret.insert(ret.end(), temp.begin(), temp.end());
    }

    return ret;
}

uint32_t FSTEntries::GetFSTEntryCount() {
    uint32_t ret = 0;

    for(const FSTEntry& entry : entries) {
        ret += entry.GetEntryCount();
    }

    return ret;
}

void FSTEntries::writeToStream(std::ostream& out) {
    for(FSTEntry& entry : entries) {
        entry.writeToStream(out);
    }
}
