#pragma once
#include <string>

#define NO_STRING_DATA 1000000
#define NO_STRING_OFFSET 2088770127
#define DEFAULT_SSF_PADSIZE 2048
#define SEC_PADSIZE			512

enum eSectionFileTypes : int {
	FILE_TYPE_SECTION = 1,
	FILE_TYPE_SCRIPT,
	FILE_TYPE_FE_TEXTURE = 2,
	FILE_TYPE_ANIMATION_SECTION = 3,
	FILE_TYPE_ANIMATION_TEXTURE = 3,
	FILE_TYPE_MODEL,
	FILE_TYPE_ANIMATION

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
	int   version; // 4?
	int   pad;
	int   unknown;
	int   files;
	int   stringSize;
};


struct section_file_entry_da {
	eSectionFileTypes type;
	int				  offset;
	int				  size;
	int               stringOffset;
};





std::wstring GetSectionTypeName(eSectionFileTypes type);