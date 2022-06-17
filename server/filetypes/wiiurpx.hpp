#pragma once

#include "../filetypes/shared/elf_structs.hpp"


#define SHF_RPL_ZLIB	    0x08000000

#define CHUNK 16384
#define LEVEL 6



enum struct [[nodiscard]] RPXError {
	NONE = 0,
	COULD_NOT_OPEN,
	NOT_RPX,
	UNKNOWN_E_TYPE,
	ZLIB_ERROR,
	REACHED_EOF,
	UNKNOWN,
	COUNT
};

struct Rpl_Fileinfo {
	uint32_t magic_version;
	uint32_t mRegBytes_Text;
	uint32_t mRegBytes_TextAlign;
	uint32_t mRegBytes_Data;
	uint32_t mRegBytes_DataAlign;
	uint32_t mRegBytes_LoaderInfo;
	uint32_t mRegBytes_LoaderInfoAlign;
	uint32_t mRegBytes_Temp;
	uint32_t mTrampAdj;
	uint32_t mSDABase;
	uint32_t mSDA2Base;
	uint32_t mSizeCoreStacks;
	uint32_t mSrcFileNameOffset;
	uint32_t mFlags;
	uint32_t mSysHeapBytes;
	uint32_t mTagsOffset;
};

namespace FileTypes {
	const char* RPXErrorGetName(RPXError err);

    RPXError rpx_decompress(std::istream& in, std::ostream& out);
    RPXError rpx_compress(std::istream& in, std::ostream& out);
}
