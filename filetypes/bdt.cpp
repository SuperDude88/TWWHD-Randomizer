#include "bdt.hpp"

#include <array>
#include <algorithm>

#include <utility/endian.hpp>
#include <utility/common.hpp>
#include <utility/file.hpp>
#include <command/Log.hpp>

using eType = Utility::Endian::Type;



namespace FileTypes {

	const char* BDTErrorGetName(BDTError err) {
		switch (err) {
			case BDTError::NONE:
				return "NONE";
			case BDTError::COULD_NOT_OPEN:
				return "COULD_NOT_OPEN";
			case BDTError::REACHED_EOF:
				return "REACHED_EOF";
			case BDTError::UNEXPECTED_VALUE:
				return "UNEXPECTED_VALUE";
			default:
				return "UNKNOWN";
		}
	}

	BDTError BDTFile::loadFromBinary(std::istream& in) {
		if (!in.read(reinterpret_cast<char*>(&numFiles), sizeof(numFiles))) {
			LOG_ERR_AND_RETURN(BDTError::REACHED_EOF);
		}
		Utility::Endian::toPlatform_inplace(eType::Big, numFiles);

		LOG_AND_RETURN_IF_ERR(readPadding<BDTError>(in, 0x1C, "\x00"));

        fileInfo.reserve(numFiles);
		for (uint32_t i = 0; i < numFiles; i++) {
			FileSpec& spec = fileInfo.emplace_back();

			if (!in.read(reinterpret_cast<char*>(&spec.offset), sizeof(spec.offset))) {
			    LOG_ERR_AND_RETURN(BDTError::REACHED_EOF);
            }
			if (!in.read(reinterpret_cast<char*>(&spec.size), sizeof(spec.size))) {
			    LOG_ERR_AND_RETURN(BDTError::REACHED_EOF);
            }
		    Utility::Endian::toPlatform_inplace(eType::Big, spec.offset);
		    Utility::Endian::toPlatform_inplace(eType::Big, spec.size);
		}

        for(const FileSpec& spec : fileInfo) {
            std::string& fileData = files.emplace_back(spec.size, '\0');

            in.seekg(spec.offset, std::ios::beg);
			if (!in.read(&fileData[0], spec.size)) {
			    LOG_ERR_AND_RETURN(BDTError::REACHED_EOF);
            }
        }

		return BDTError::NONE;
	}

	BDTError BDTFile::loadFromFile(const std::string& filePath) {
		std::ifstream file(filePath, std::ios::binary);
		if (!file.is_open()) {
			LOG_ERR_AND_RETURN(BDTError::COULD_NOT_OPEN);
		}
		return loadFromBinary(file);
	}

	BDTError BDTFile::writeToStream(std::ostream& out) {
		numFiles = files.size();
        fileInfo.resize(numFiles);

		Utility::Endian::toPlatform_inplace(eType::Big, numFiles);
		out.write(reinterpret_cast<const char*>(&numFiles), sizeof(numFiles));
        padToLen(out, 0x20);

        Utility::seek(out, 0x20 + (0x8 * files.size()), std::ios::beg);
        std::streamoff offset = out.tellp();
		for (uint32_t i = 0; i < files.size(); i++) {
            fileInfo[i].offset = static_cast<uint32_t>(offset);

			out.write(&files[i][0], files[i].size());
            padToLen(out, 4); //Not sure if this is correct, just a guess

            offset = out.tellp();
            fileInfo[i].size = offset - fileInfo[i].offset;
		}

        out.seekp(0x20, std::ios::beg);
        for(FileSpec& spec : fileInfo) {
            Utility::Endian::toPlatform_inplace(eType::Big, spec.offset);
            Utility::Endian::toPlatform_inplace(eType::Big, spec.size);
            out.write(reinterpret_cast<const char*>(&spec.offset), sizeof(spec.offset));
            out.write(reinterpret_cast<const char*>(&spec.size), sizeof(spec.size));
        }

		return BDTError::NONE;
	}

	BDTError BDTFile::writeToFile(const std::string& filePath) {
		std::ofstream file(filePath, std::ios::binary);
		if (!file.is_open()) {
			LOG_ERR_AND_RETURN(BDTError::COULD_NOT_OPEN);
		}
		return writeToStream(file);
	}
}
