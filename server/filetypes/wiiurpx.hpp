#pragma once

#include <fstream>

#define SHF_RPL_ZLIB	    0x08000000

#define SHT_PROGBITS        0x00000001
#define SHT_SYMTAB          0x00000002
#define SHT_STRTAB          0x00000003
#define SHT_RELA            0x00000004
#define SHT_NOBITS          0x00000008

#define SHT_RPL_EXPORTS     0x80000001
#define SHT_RPL_IMPORTS     0x80000002
#define SHT_RPL_CRCS        0x80000003
#define SHT_RPL_FILEINFO    0x80000004

#define CHUNK 16384
#define LEVEL 6

typedef unsigned char	u8;
typedef unsigned short	u16;
typedef unsigned int	u32;
typedef signed   char	s8;
typedef signed   short	s16;
typedef signed   int    s32;
typedef unsigned long	ulg;
typedef unsigned long long u64;
typedef signed long long   s64;

typedef struct {
	u8  e_ident[0x10];
	u16 e_type;
	u16 e_machine;
	u32 e_version;
	u32 e_entry;
	u32 e_phoff;
	u32 e_shoff;
	u32 e_flags;
	u16 e_ehsize;
	u16 e_phentsize;
	u16 e_phnum;
	u16 e_shentsize;
	u16 e_shnum;
	u16 e_shstrndx;
} Elf32_Ehdr;

typedef struct {
	u32 sh_name;
	u32 sh_type;
	u32 sh_flags;
	u32 sh_addr;
	u32 sh_offset;
	u32 sh_size;
	u32 sh_link;
	u32 sh_info;
	u32 sh_addralign;
	u32 sh_entsize;
} Elf32_Shdr;

typedef struct {
	u32 index;
	u32 sh_offset;
} Elf32_Shdr_Sort;

typedef struct {
	u32 st_name;
	u32 st_value;
	u32 st_size;
	u8  st_info;
	u8  st_other;
	u16 st_shndx;
} Elf32_Sym;

typedef struct {
	u32 r_offset;
	u32 r_info;
	s32 r_addend;
} Elf32_Rela;

typedef struct {
	u32 magic_version;
	u32 mRegBytes_Text;
	u32 mRegBytes_TextAlign;
	u32 mRegBytes_Data;
	u32 mRegBytes_DataAlign;
	u32 mRegBytes_LoaderInfo;
	u32 mRegBytes_LoaderInfoAlign;
	u32 mRegBytes_Temp;
	u32 mTrampAdj;
	u32 mSDABase;
	u32 mSDA2Base;
	u32 mSizeCoreStacks;
	u32 mSrcFileNameOffset;
	u32 mFlags;
	u32 mSysHeapBytes;
	u32 mTagsOffset;
} Rpl_Fileinfo;

namespace FileTypes {
	int rpx_decompress(std::ifstream& in, std::ofstream& out);
    int rpx_compress(std::ifstream& in, std::ofstream& out);
}
