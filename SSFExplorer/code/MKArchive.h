#pragma once
#include <string>

#define NO_STRING_DATA 1000000
#define NO_STRING_OFFSET 2088770127
#define DEFAULT_SSF_PADSIZE 2048
#define SEC_PADSIZE			512
#define SEC_FILEPADSIZE		128
#define SEC_STRINGPAD		4
#define DUMYMDATA_SIZE		4096

enum eSectionFileTypes : int {
	FILE_TYPE_SECTION = 1,
	FILE_TYPE_SCRIPT,
	FILE_TYPE_FE_TEXTURE = 2,
	FILE_TYPE_ANIMATION_SECTION = 3,
	FILE_TYPE_ANIMATION_TEXTURE = 3,
	FILE_TYPE_MODEL,
	FILE_TYPE_ANIMATION,
	FILE_TYPE_SECTION_DA,
	FILE_TYPE_SCRIPT_DA,
	FILE_TYPE_ANIMATION_SECTION_DA

};

struct section_file_header {
	int   header;
	int   version; // 4?
	int   pad;
	int   unknown;
	int   files;
	int   stringSize;
	int   fileSize;
};

struct section_file_entry {
	eSectionFileTypes type;
	int				  offset;
	int				  size;	
	int               stringOffset;
};

struct section_file_header_da {
	int   header;
	int   version;
	int   pad;
	int   unknown;
	int   files;
	int   fileSize;
};


struct section_file_entry_da {
	eSectionFileTypes type;
	int				  offset;
	int				  size;
};


struct pak_header {
	int header; // PAK 
	int version; // 256 or 1
	int files;
	int filesOffset;
	int stringSize;

	char pad[2028] = {};
};

struct pak_entry {
	int stringOffset;
	int offset;
	int size;
};


section_file_header DA2D_Header(section_file_header_da da_header);
section_file_entry DA2D_Entry(section_file_entry_da da_entry);


section_file_header_da D2DA_Header(section_file_header d_header);
section_file_entry_da D2DA_Entry(section_file_entry d_entry);



std::wstring GetSectionTypeName(eSectionFileTypes type);
std::wstring GetSectionTypeName_DA(eSectionFileTypes type);


extern const unsigned char gDummyData[DUMYMDATA_SIZE];