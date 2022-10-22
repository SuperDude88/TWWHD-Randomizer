#pragma once

#include <string>
#include <list>
#include <vector>
#include <variant>
#include <filesystem>

class Content; //forward declare, circular include issue

class FSTEntry {
public:
	enum Type : uint8_t {
		FILE = 0x00,
		DIR = 0x01,
		WiiVC = 0x02,
		NOT_IN_NUS = 0x80
	};
    
	struct FileEntry {
		uint32_t fileOffset = 0;
		uint32_t fileSize = 0;
	};
	struct DirEntry {
		uint32_t parentOffset = 0;
		uint32_t nextOffset = 0;
	};

	std::string name;
	std::filesystem::path path;
    FSTEntry* parent = nullptr;
	std::list<FSTEntry> children;

	uint32_t nameOffset = 0;
	uint32_t entryOffset = 0;
    uint16_t flags = 0;

	std::variant<FileEntry, DirEntry> entry = DirEntry();
	
	bool isRoot = false;
    uint32_t root_entryCount = 0;
	
	inline bool isDir() const { return entry.index() == 1; }
	inline bool isFile() const { return !isDir(); }
	inline uint8_t typeAsByte() {
		uint8_t type = 0;
		if(isDir()) type |= Type::DIR;
		if(name.ends_with("nfs")) type |= Type::WiiVC;

		return type;
	}

	FSTEntry() = default;
	FSTEntry(const bool& root_) : 
		isRoot(root_)
	{};

	void setContent(Content* content_);
	Content* getContent();
	
	std::vector<FSTEntry*> GetFSTEntriesByContent(const Content& content_) const;
	uint32_t GetEntryCount() const;
	std::vector<FSTEntry*> GetDirChildren();
	std::vector<FSTEntry*> GetFileChildren();
	void Update(uint64_t& curEntryOffset);
	FSTEntry* UpdateDirRefs();
	void writeToStream(std::ostream& out);

private:
	Content* content;
};

class FSTEntries {
public:
    std::list<FSTEntry> entries;
	uint64_t curEntryOffset = 0;

	FSTEntries() :
		entries(1, FSTEntry(1)) //add root
	{}

	inline FSTEntry& GetRootEntry() { return entries.front(); }
	void Update();
	void UpdateDirRefs();
	std::vector<FSTEntry*> GetFSTEntriesByContent(const Content& content) const;
	uint32_t GetFSTEntryCount();
	void writeToStream(std::ostream& out);
};
