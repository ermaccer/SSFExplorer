#include "MKArchive.h"

std::wstring GetSectionTypeName(eSectionFileTypes type)
{
	std::wstring output = L"Type: ";

	switch (type)
	{
	case FILE_TYPE_SECTION:
		output += L"Archive (.sec)";
		break;
	case FILE_TYPE_SCRIPT:
		output += L"Script (.mko) or Frontend Texture";
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
