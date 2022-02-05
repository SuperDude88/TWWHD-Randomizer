#include "sarc.hpp"

#include <cstring>
#include <algorithm>

#include "../utility/byteswap.hpp"
#include "../utility/common.hpp"

const std::unordered_map<std::string, uint32_t> alignments = {
	{"bflan", 0x00000004},
	{"bflyt", 0x00000004},
	{"szs", 0x00002000},
	{"sarc", 0x00002000},
	{"bfres", 0x00002000}, //seems to be 0x2000 usually
	{"sharcfb", 0x00002000}, //seems to be 0x2000 usually, sometimes 0x100 in .sarc files?
};


uint32_t calculateHash(const std::string& name, uint32_t multiplier) {
	uint32_t hash = 0;
	for (const int8_t byte : name) {
		if (byte == 0x00) break; //string is null-terminated
		hash = hash * multiplier + byte;
	}

	return hash;
}

uint32_t getAlignment(const std::string& fileExt, const file& file) {
	if (alignments.find(fileExt) != alignments.end()) {
		return alignments.at(fileExt);
	}
	else if (fileExt == "bflim" && file.data.substr(file.data.size() - 0x28, 4) == "FLIM") {
		uint16_t alignment = *(uint16_t*)&file.data[(file.data.size() - 8)];
		Utility::byteswap_inplace(alignment);
		return alignment;
	}
	else {
		return 0;
	}
}

namespace FileTypes{

	const char* SARCErrorGetName(SARCError err) {
		switch (err) {
		case SARCError::NONE:
			return "NONE";
		case SARCError::COULD_NOT_OPEN:
			return "COULD_NOT_OPEN";
		case SARCError::NOT_SARC:
			return "NOT_SARC";
		case SARCError::UNKNOWN_VERSION:
			return "UNKNOWN_VERSION";
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
		case SARCError::UNEXPECTED_VALUE:
			return "UNEXPECTED_VALUE";
		default:
			return "UNKNOWN";
		}
	}

	SARCFile::SARCFile() {
		
	}

	void SARCFile::initNew() {
		memcpy(&header.magicSARC, "SARC", 4);
		header.headerSize_0x14 = 0x14;
		header.byteOrderMarker = 0xFEFF;
		header.fileSize = 0;
		header.dataOffset = 0;
		header.version_0x0100 = 0x0100;
		header.padding_0x00[0] = 0x00;
		header.padding_0x00[1] = 0x00;

		fileTable.offset = 0;
		memcpy(&fileTable.magicSFAT, "SFAT", 4);
		fileTable.headerSize_0xC = 0xC;
		fileTable.numFiles = 0;
		fileTable.hashKey_0x65 = 0x00000065;
		fileTable.nodes = {};

		nameTable.offset = 0;
		memcpy(&nameTable.magicSFNT, "SFNT", 4);
		nameTable.headerSize_0x8 = 0x8;
		nameTable.padding_0x00[0] = 0x00;
		nameTable.padding_0x00[1] = 0x00;
		nameTable.filenames = {};

		file_index_by_name = {};
		files = {};
	}

	SARCFile SARCFile::createNew(const std::string& filename) {
		SARCFile newSARC{};
		newSARC.initNew();
		return newSARC;
	}

	SARCError SARCFile::loadFromBinary(std::istream& sarc) {
		if (!sarc.read(header.magicSARC, 4)) return SARCError::REACHED_EOF;
		if (!sarc.read(reinterpret_cast<char*>(&header.headerSize_0x14), sizeof(header.headerSize_0x14))) return SARCError::REACHED_EOF;
		if (!sarc.read(reinterpret_cast<char*>(&header.byteOrderMarker), sizeof(header.byteOrderMarker))) return SARCError::REACHED_EOF;
		if (!sarc.read(reinterpret_cast<char*>(&header.fileSize), sizeof(header.fileSize))) return SARCError::REACHED_EOF;
		if (!sarc.read(reinterpret_cast<char*>(&header.dataOffset), sizeof(header.dataOffset))) return SARCError::REACHED_EOF;
		if (!sarc.read(reinterpret_cast<char*>(&header.version_0x0100), sizeof(header.version_0x0100))) return SARCError::REACHED_EOF;
		if (!sarc.read(reinterpret_cast<char*>(&header.padding_0x00), sizeof(header.padding_0x00))) return SARCError::REACHED_EOF;

		Utility::byteswap_inplace(header.headerSize_0x14);
		Utility::byteswap_inplace(header.byteOrderMarker);
		Utility::byteswap_inplace(header.fileSize);
		Utility::byteswap_inplace(header.dataOffset);
		Utility::byteswap_inplace(header.version_0x0100);

		if (std::strncmp("SARC", header.magicSARC, 4) != 0) return SARCError::NOT_SARC;
		if (header.headerSize_0x14 != 0x0014) return SARCError::UNEXPECTED_VALUE;
		if (header.byteOrderMarker != 0xFEFF) return SARCError::UNEXPECTED_VALUE;
		if (header.version_0x0100 != 0x0100) return SARCError::UNKNOWN_VERSION;
		if (header.padding_0x00[0] != 0x00 || header.padding_0x00[1] != 0x00) return SARCError::UNEXPECTED_VALUE;

		fileTable.offset = sarc.tellg();
		if (!sarc.read(fileTable.magicSFAT, 4)) return SARCError::REACHED_EOF;
		if (!sarc.read(reinterpret_cast<char*>(&fileTable.headerSize_0xC), sizeof(fileTable.headerSize_0xC))) return SARCError::REACHED_EOF;
		if (!sarc.read(reinterpret_cast<char*>(&fileTable.numFiles), sizeof(fileTable.numFiles))) return SARCError::REACHED_EOF;
		if (!sarc.read(reinterpret_cast<char*>(&fileTable.hashKey_0x65), sizeof(fileTable.hashKey_0x65))) return SARCError::REACHED_EOF;

		Utility::byteswap_inplace(fileTable.headerSize_0xC);
		Utility::byteswap_inplace(fileTable.numFiles);
		Utility::byteswap_inplace(fileTable.hashKey_0x65);

		if (std::strncmp("SFAT", fileTable.magicSFAT, 4)) return SARCError::UNEXPECTED_VALUE;
		if (fileTable.headerSize_0xC != 0xC) return SARCError::UNEXPECTED_VALUE;
		if (fileTable.hashKey_0x65 != 0x65) return SARCError::UNEXPECTED_VALUE;

		for (uint16_t i = 0; i < fileTable.numFiles; i++) {
			SFATNode& node = fileTable.nodes.emplace_back();
			if (!sarc.read(reinterpret_cast<char*>(&node.nameHash), sizeof(node.nameHash))) return SARCError::REACHED_EOF;
			if (!sarc.read(reinterpret_cast<char*>(&node.attributes), sizeof(node.attributes))) return SARCError::REACHED_EOF;
			if (!sarc.read(reinterpret_cast<char*>(&node.dataStart), sizeof(node.dataStart))) return SARCError::REACHED_EOF;
			if (!sarc.read(reinterpret_cast<char*>(&node.dataEnd), sizeof(node.dataEnd))) return SARCError::REACHED_EOF;

			Utility::byteswap_inplace(node.nameHash);
			Utility::byteswap_inplace(node.attributes);
			Utility::byteswap_inplace(node.dataStart);
			Utility::byteswap_inplace(node.dataEnd);
		}

		nameTable.offset = sarc.tellg();
		if (!sarc.read(nameTable.magicSFNT, 4)) return SARCError::REACHED_EOF;
		if (!sarc.read(reinterpret_cast<char*>(&nameTable.headerSize_0x8), sizeof(nameTable.headerSize_0x8))) return SARCError::REACHED_EOF;
		if (!sarc.read(reinterpret_cast<char*>(&nameTable.padding_0x00), sizeof(nameTable.padding_0x00))) return SARCError::REACHED_EOF;

		Utility::byteswap_inplace(nameTable.headerSize_0x8);

		if (std::strncmp("SFNT", nameTable.magicSFNT, 4)) return SARCError::UNEXPECTED_VALUE;
		if (nameTable.headerSize_0x8 != 0x8) return SARCError::UNEXPECTED_VALUE;
		if (nameTable.padding_0x00[0] != 0x00 || nameTable.padding_0x00[1] != 0x00) return SARCError::UNEXPECTED_VALUE;

		for (const SFATNode& node : fileTable.nodes) {
			if ((node.attributes & 0xFF000000) >> 24 != 0x01) return SARCError::BAD_NODE_ATTR;
			uint32_t nameOffset = (node.attributes & 0x00FFFFFF) * 4;
			const std::string name = readNullTerminatedStr(sarc, nameTable.offset + 0x8 + nameOffset);
			if (name.empty()) return SARCError::REACHED_EOF;
			nameTable.filenames.push_back(name);

			uint32_t hash = calculateHash(name, fileTable.hashKey_0x65);
			if (hash != node.nameHash) return SARCError::FILENAME_HASH_MISMATCH;

			file& fileEntry = files.emplace_back();
			file_index_by_name[name] = files.size() - 1;
			fileEntry.name = name;
			fileEntry.data.resize(node.dataEnd - node.dataStart);
			sarc.seekg(header.dataOffset + node.dataStart, std::ios::beg);
			if (!sarc.read(&fileEntry.data[0], fileEntry.data.size())) return SARCError::REACHED_EOF;
		}

		return SARCError::NONE;
	}

	SARCError SARCFile::loadFromFile(const std::string& filePath) {
		std::ifstream file(filePath, std::ios::binary);
		if (!file.is_open()) {
			return SARCError::COULD_NOT_OPEN;
		}
		return loadFromBinary(file);
	}

	const file& SARCFile::getFile(const std::string& filename) {
		if (file_index_by_name.count(filename) == 0) {
			file blankFile;
			return blankFile;
		}
		return files[file_index_by_name.at(filename)];
	}

	SARCError SARCFile::writeToStream(std::ostream& out) {
		{
			uint16_t headerSize_BE = Utility::byteswap(header.headerSize_0x14);
			uint16_t byteOrderMarker_BE = Utility::byteswap(header.byteOrderMarker);
			uint32_t dataOffset_BE = Utility::byteswap(header.dataOffset);
			uint16_t version_BE = Utility::byteswap(header.version_0x0100);

			out.write(header.magicSARC, 4);
			out.write(reinterpret_cast<char*>(&headerSize_BE), sizeof(headerSize_BE));
			out.write(reinterpret_cast<char*>(&byteOrderMarker_BE), sizeof(byteOrderMarker_BE));
			out.seekp(4, std::ios::cur); //skip over filesize for now
			out.write(reinterpret_cast<char*>(&dataOffset_BE), sizeof(dataOffset_BE));
			out.write(reinterpret_cast<char*>(&version_BE), sizeof(version_BE));
			out.write(reinterpret_cast<char*>(&header.padding_0x00), sizeof(header.padding_0x00));
		}

		{
			uint16_t headerSize_BE = Utility::byteswap(fileTable.headerSize_0xC);
			uint16_t numFiles_BE = Utility::byteswap(fileTable.numFiles);
			uint32_t hashKey_BE = Utility::byteswap(fileTable.hashKey_0x65);

			out.write(fileTable.magicSFAT, 4);
			out.write(reinterpret_cast<char*>(&headerSize_BE), sizeof(headerSize_BE));
			out.write(reinterpret_cast<char*>(&numFiles_BE), sizeof(numFiles_BE));
			out.write(reinterpret_cast<char*>(&hashKey_BE), sizeof(hashKey_BE));

			for (const SFATNode& node : fileTable.nodes) {
				uint32_t nameHash_BE = Utility::byteswap(node.nameHash);
				uint32_t attributes_BE = Utility::byteswap(node.attributes);
				uint32_t dataStart_BE = Utility::byteswap(node.dataStart);
				uint32_t dataEnd_BE = Utility::byteswap(node.dataEnd);

				out.write(reinterpret_cast<char*>(&nameHash_BE), sizeof(nameHash_BE));
				out.write(reinterpret_cast<char*>(&attributes_BE), sizeof(attributes_BE));
				out.write(reinterpret_cast<char*>(&dataStart_BE), sizeof(dataStart_BE));
				out.write(reinterpret_cast<char*>(&dataEnd_BE), sizeof(dataEnd_BE));
			}
		}

		{
			uint16_t headerSize_BE = Utility::byteswap(nameTable.headerSize_0x8);

			out.write(nameTable.magicSFNT, 4);
			out.write(reinterpret_cast<char*>(&headerSize_BE), sizeof(headerSize_BE));
			out.write(reinterpret_cast<char*>(&nameTable.padding_0x00), sizeof(nameTable.padding_0x00));

			for (const std::string& filename : nameTable.filenames) {
				out.write(&filename[0], filename.length());
				padToLen(out, 4);
			}
		}

		out.seekp(header.dataOffset, std::ios::beg);
		for (unsigned int i = 0; i < files.size(); i++) {
			out.seekp(header.dataOffset + fileTable.nodes[i].dataStart, std::ios::beg);
			out.write(&files[i].data[0], files[i].data.size());
		}

		header.fileSize = out.tellp();
		out.seekp(8, std::ios::beg);
		uint32_t fileSize_BE = Utility::byteswap(header.fileSize);
		out.write(reinterpret_cast<char*>(&fileSize_BE), sizeof(fileSize_BE));

		return SARCError::NONE;
	}

	SARCError SARCFile::writeToFile(const std::string& outFilePath) {
		std::ofstream outFile(outFilePath, std::ios::binary);
		if (!outFile.is_open()) {
			return SARCError::COULD_NOT_OPEN;
		}
		return writeToStream(outFile);
	}

	SARCError SARCFile::extractToDir(const std::string& dirPath) {
		for (const file& file : files)
		{
			std::filesystem::path path = dirPath + '/' + file.name;
			std::filesystem::create_directories(path.parent_path()); //handle any folder structure stuff contained in the SARC
			std::ofstream outFile(dirPath + '/' + file.name, std::ios::binary);
			if (!outFile.is_open())
			{
				return SARCError::COULD_NOT_OPEN;
			}
			outFile.write(&file.data[0], file.data.size());
		}
		return SARCError::NONE;
	}

	SARCError SARCFile::rebuildFromDir(const std::string& dirPath) {
		//rebuild using the original filename list (so extraneous unpacked stuff isnt added accidentally)

		uint32_t curDataOffset = 0;
		for (unsigned int i = 0; i < nameTable.filenames.size(); i++) {
			const std::string& filename = nameTable.filenames[i];
			std::filesystem::path absPath = dirPath + '/' + filename;
			uint32_t fileSize = std::filesystem::file_size(absPath);
			SFATNode& node = fileTable.nodes[i];

			file& entry = files[i];
			entry.name = filename;
			entry.data.resize(fileSize);

			std::ifstream inFile(absPath.string(), std::ios::binary);
			if (!inFile.read(&entry.data[0], fileSize)) return SARCError::REACHED_EOF;

			//silly alignment stuff
			std::string filetype = filename.substr(filename.find(".") + 1);
			filetype.pop_back();
			uint32_t alignment = getAlignment(filetype, entry);
			if (alignment != 0) {
				unsigned int padLen = alignment - (curDataOffset % alignment);
				if (padLen == alignment) padLen = 0;
				node.dataStart = curDataOffset + padLen;
			}
			else {
				node.dataStart = curDataOffset;
			}

			node.dataEnd = node.dataStart + entry.data.size();
			curDataOffset = node.dataEnd;

			file_index_by_name[filename] = i;
		}

		return SARCError::NONE;
	}

	SARCError SARCFile::buildFromDir(const std::string& dirPath) { //needs some implementation updates to work completely from a new sarc
		fileTable.numFiles = 0;
		fileTable.nodes.clear();
		nameTable.filenames.clear();
		files.clear();

		uint32_t curDataOffset = 0x14 + 0xC + 0x8; //header sizes
		uint32_t curNameOffset = 0;
		for (const auto& path : std::filesystem::recursive_directory_iterator(dirPath)) {
			if (path.is_regular_file()) {
				std::filesystem::path absPath = path.path();
				std::string filename = absPath.string().substr(absPath.string().find(dirPath) + dirPath.size() + 1); //could use std::filesystem::relative(absPath, dirPath).string(), but gives undefined reference errors with devkitPro stuff
				// ^ include everything after the dirPath, and exclude 1 extra character for the path separator connecting them
				filename += '\0'; //add null terminator
#ifdef _WIN32
				std::replace(filename.begin(), filename.end(), '\\', '/');
#endif

				uint32_t fileSize = std::filesystem::file_size(absPath);
				SFATNode& node = fileTable.nodes.emplace_back();
				node.nameHash = calculateHash(filename, fileTable.hashKey_0x65);

				file& entry = files.emplace_back();
				entry.name = filename;
				entry.data.resize(fileSize);

				curDataOffset += 0x10 + filename.size(); //add node + filename length
				unsigned int numPaddingBytes = 4 - (filename.size() % 4);
				if (numPaddingBytes == 4) numPaddingBytes = 0;
				curDataOffset += numPaddingBytes; //add name padding

				std::ifstream inFile(absPath.string(), std::ios::binary);
				if (!inFile.read(&entry.data[0], fileSize)) return SARCError::REACHED_EOF;
			}
		}

		unsigned int numPaddingBytes = 0x100 - (curDataOffset % 0x100);
		if (numPaddingBytes == 0x100) numPaddingBytes = 0;
		curDataOffset += numPaddingBytes; //pad to nearest 0x100, maybe not needed but haven't seen something to disprove it

		header.dataOffset = curDataOffset;

		std::sort(files.begin(), files.end(), [&](const file& a, const file& b) {return calculateHash(a.name, fileTable.hashKey_0x65) < calculateHash(b.name, fileTable.hashKey_0x65); });
		std::sort(fileTable.nodes.begin(), fileTable.nodes.end(), [&](const SFATNode& a, const SFATNode& b) {return a.nameHash < b.nameHash; });
		fileTable.numFiles = files.size();

		curDataOffset = 0;

		for (unsigned int i = 0; i < files.size(); i++) {
			const file& entry = files[i];
			SFATNode& node = fileTable.nodes[i];

			//silly alignment stuff
			const std::string& filename = entry.name;
			std::string filetype = filename.substr(filename.find(".") + 1);
			filetype.pop_back();
			uint32_t alignment = getAlignment(filetype, entry);
			if (alignment != 0) {
				unsigned int padLen = alignment - (curDataOffset % alignment);
				if (padLen == alignment) padLen = 0;
				node.dataStart = curDataOffset + padLen;
			}
			else {
				node.dataStart = curDataOffset;
			}

			node.dataEnd = node.dataStart + entry.data.size();
			curDataOffset = node.dataEnd;

			node.attributes = 0x01000000 | ((curNameOffset / 4) & 0x00FFFFFF);
			curNameOffset += filename.size();
			numPaddingBytes = 4 - (curNameOffset % 4);
			if (numPaddingBytes == 4) numPaddingBytes = 0;
			curNameOffset += numPaddingBytes;
			nameTable.filenames.push_back(filename);
			file_index_by_name[filename] = i;
		}

		header.fileSize = header.dataOffset + fileTable.nodes.back().dataEnd;

		return SARCError::NONE;
	}
}
