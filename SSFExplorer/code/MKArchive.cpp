#include "MKArchive.h"

section_file_header DA2D_Header(section_file_header_da da_header)
{
	section_file_header result = {};

	result.header = da_header.header;
	result.version = da_header.version;
	result.pad = da_header.pad;
	result.unknown = da_header.unknown;
	result.files = da_header.files;
	result.fileSize = da_header.fileSize;
	result.stringSize = NO_STRING_DATA;

	return result;
}

section_file_entry DA2D_Entry(section_file_entry_da da_entry)
{
	section_file_entry result = {};
	result.type = da_entry.type;
	result.size = da_entry.size;
	result.offset = da_entry.offset;
	

	return result;
}

section_file_header_da D2DA_Header(section_file_header d_header)
{
	section_file_header_da result = {};

	result.header = d_header.header;
	result.version = d_header.version;
	result.pad = d_header.pad;
	result.unknown = d_header.unknown;
	result.files = d_header.files;
	result.fileSize = d_header.fileSize;

	return result;
}

section_file_entry_da D2DA_Entry(section_file_entry d_entry)
{
	section_file_entry_da result = {};
	result.type = d_entry.type;
	result.size = d_entry.size;
	result.offset = d_entry.offset;
	return result;
}

std::wstring GetSectionTypeName(eSectionFileTypes type)
{
	std::wstring output = L"Type: ";

	switch (type)
	{
	case FILE_TYPE_SECTION:
		output += L"Archive (.sec)";
		break;
	case FILE_TYPE_SCRIPT:
		output += L"Script (.mko/cmo) or Frontend Texture";
		break;
	case FILE_TYPE_ANIMATION_SECTION:
		output += L"Animation Archive (.sec) or Texture";
		break;
	case FILE_TYPE_MODEL:
		output += L"Model (custom .dff)";
		break;
	case FILE_TYPE_ANIMATION:
		output += L"Animation (.mka)";
		break;
	default:
		break;
	}

	output += L" (" + std::to_wstring(type) + L")";

	return output;
}

std::wstring GetSectionTypeName_DA(eSectionFileTypes type)
{
	std::wstring output = L"Type: ";

	switch (type)
	{
	case FILE_TYPE_SECTION_DA:
		output += L"Archive (.sec)";
		break;
	case FILE_TYPE_SCRIPT_DA:
		output += L"Script (.mko/cmo) or Frontend Texture";
		break;
	case FILE_TYPE_ANIMATION_SECTION_DA:
		output += L"Animation Archive (.sec) or Texture";
	default:
		break;
	}

	output += L" (" + std::to_wstring(type) + L")";

	return output;
}
