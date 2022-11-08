#include "tmd.hpp"

#include <algorithm>

#include <libs/hashing/sha256.h>
#include <utility/endian.hpp>
#include <utility/file.hpp>
#include <utility/common.hpp>
#include <command/Log.hpp>

using eType = Utility::Endian::Type;

#define WIIU_BLOCK_SIZE 0x8000

static SHA256 sha256;



namespace FileTypes {

	const char* TMDErrorGetName(TMDError err) {
		switch (err) {
			case TMDError::NONE:
				return "NONE";
			case TMDError::COULD_NOT_OPEN:
				return "COULD_NOT_OPEN";
			case TMDError::REACHED_EOF:
				return "REACHED_EOF";
			case TMDError::UNEXPECTED_VALUE:
				return "UNEXPECTED_VALUE";
			default:
				return "UNKNOWN";
		}
	}

	void TMDFile::update(const AppInfo& info) {
		header.groupID = info.groupID;
		header.systemVersion = info.OSVersion;
		header.appType = info.appType;
		header.titleVersion = info.titleVer;
		header.contentCount = contents.GetContentCount();
	
		ContentInfo firstContentInfo(header.contentCount);
        contentInfo = firstContentInfo;
	}

	TMDError TMDFile::writeToStream(std::ostream& out) {
		Utility::Endian::toPlatform_inplace(eType::Big, header.signatureType);
		Utility::Endian::toPlatform_inplace(eType::Big, header.systemVersion);
		const uint64_t tid = Utility::Endian::toPlatform(eType::Big, ticket.titleID);
		Utility::Endian::toPlatform_inplace(eType::Big, header.titleType);
		Utility::Endian::toPlatform_inplace(eType::Big, header.groupID);
		Utility::Endian::toPlatform_inplace(eType::Big, header.appType);
		Utility::Endian::toPlatform_inplace(eType::Big, header.accessRights);
		Utility::Endian::toPlatform_inplace(eType::Big, header.titleVersion);
		Utility::Endian::toPlatform_inplace(eType::Big, header.contentCount);
		Utility::Endian::toPlatform_inplace(eType::Big, header.bootIndex);

		out.write(reinterpret_cast<const char*>(&header.signatureType), 4);
		out.write(reinterpret_cast<const char*>(&header.signature), sizeof(header.signature));
		out.write(reinterpret_cast<const char*>(&header.padding_0x00), sizeof(header.padding_0x00));
		out.write(reinterpret_cast<const char*>(&header.issuer), sizeof(header.issuer));
		out.write(reinterpret_cast<const char*>(&header.version), sizeof(header.version));
		out.write(reinterpret_cast<const char*>(&header.caCrlVersion), sizeof(header.caCrlVersion));
		out.write(reinterpret_cast<const char*>(&header.signerCrlVersion), sizeof(header.signerCrlVersion));
		out.write(reinterpret_cast<const char*>(&header.pad_0x00), sizeof(header.pad_0x00));
		out.write(reinterpret_cast<const char*>(&header.systemVersion), sizeof(header.systemVersion));
		out.write(reinterpret_cast<const char*>(&tid), sizeof(tid));
		out.write(reinterpret_cast<const char*>(&header.titleType), sizeof(header.titleType));
		out.write(reinterpret_cast<const char*>(&header.groupID), sizeof(header.groupID));
		out.write(reinterpret_cast<const char*>(&header.appType), sizeof(header.appType));
		out.write(reinterpret_cast<const char*>(&header.reserved), sizeof(header.reserved));
		out.write(reinterpret_cast<const char*>(&header.accessRights), sizeof(header.accessRights));
		out.write(reinterpret_cast<const char*>(&header.titleVersion), sizeof(header.titleVersion));
		out.write(reinterpret_cast<const char*>(&header.contentCount), sizeof(header.contentCount));
		out.write(reinterpret_cast<const char*>(&header.bootIndex), sizeof(header.bootIndex));
		out.write(reinterpret_cast<const char*>(&header.padding2_0x00), sizeof(header.padding2_0x00));
		out.write(reinterpret_cast<const char*>(&header.SHA2), sizeof(header.SHA2));

		contentInfo.writeToStream(out);
		Utility::seek(out, (0x40 * sizeof(contentInfo)) - sizeof(contentInfo), std::ios::cur);

		contents.writeToStream(out);

		return TMDError::NONE;
	}

	TMDError TMDFile::writeToFile(const std::string& filePath) {
		std::ofstream file(filePath, std::ios::binary);
		if (!file.is_open()) {
			LOG_ERR_AND_RETURN(TMDError::COULD_NOT_OPEN);
		}
		return writeToStream(file);
	}

	Encryption TMDFile::getEncryption() {
		IV iv{0};
		const uint64_t tID = Utility::Endian::toPlatform(eType::Big, ticket.titleID);
		std::memcpy(iv.data(), &tID, sizeof(tID));

		return Encryption(ticket.decryptedKey, iv);
	}

	void TMDFile::UpdateContentInfoHash() {
		std::stringstream info;
		contentInfo.writeToStream(info);
		padToLen(info, 0x24 * 64);
		
		std::string hash = sha256(info.str());
		std::copy(hash.begin(), hash.begin() + 0x20, header.SHA2.data());
	}
}
