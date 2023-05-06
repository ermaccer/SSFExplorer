#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <Windows.h>
#include <fstream>
#include <time.h>
#include "MKArchive.h"


#define SSF_EXPLORER_VERSION L"1.2"


enum eOutputImageFormat {
	OutputImage_BMP,
	OutputImage_TGA,
	// TODO
	OutputImage_PNG
};



enum eGames {
	GAME_NULL,
	MORTAL_KOMBAT_UNCHAINED,
	MORTAL_KOMBAT_DECEPTION,
	MORTAL_KOMBAT_ARMAGEDDON,
	MORTAL_KOMBAT_DEADLY_ALLIANCE,
	SUPPORTED_GAMES,
};


enum ePlatforms {
	PLATFORM_PS2,
	PLATFORM_XBOX,
	PLATFORM_GC_WII,
	PLATFORM_PSP,
	TOTAL_PLATFORMS


};

struct file_entry {
	std::wstring       name;
	section_file_entry ent;
};


struct ssfexplorer_settings {
	bool   m_bFirstRun;
	eGames m_egGame;
	ePlatforms m_epPlatform;
	bool   m_bGuessExtensions;

};


enum TextureExporter_Flags {
	TE_ALPHA_CHANNEL = 1,
	TE_SILENT = 2,
	TE_BATCH_EXPORT = 4,
	TE_TGA_EXPORT = 8
};


class SSFExplorer {
private:
	HWND* hLogBox;
	HWND* hFilesList;
	HWND* hGroupBox;
	HWND* hDataBox;
public:
	std::ifstream pFile;
	std::wstring InputPath;
	std::wstring OutputPath;

	std::vector<file_entry> Files;

	section_file_header  m_secHeader;
	bool				 m_bUsesFilenames;
	bool				 m_bFighterFix;
	int					 m_nUnknownDA;
	std::wstring         m_wstrBuildFolder;

	ssfexplorer_settings m_settings;

	void         Init(HWND* log, HWND* list, HWND* box);

	void		 ReadSettings();

	void OpenFile(std::wstring input);
	void OpenINI(std::wstring input);
	void ReadFile();
	void ReadFile_D();
	void ReadFile_DA();
	void ReadINI();
	void ListFiles();
	bool ReadNamesFromFile(std::wstring name);
	void DisplayFileData();


	void ExtractSelected();
	void ExtractAll();
	void Export();

	void ExportTexture();

	void ExportAllTextures(bool alpha = false, eOutputImageFormat img = OutputImage_BMP);

	bool TextureExporter_Unchained(int baseOffset, DWORD dwSel, int flags = 0);
	bool TextureExporter_DADA(int baseOffset, DWORD dwSel, int flags = 0);


	void Build();

	void ExtractPAK();

	void Close();
	void Log(std::wstring msg);
	void AddData(std::wstring msg);

};


std::wstring   SetPathFromButton(wchar_t* filter, wchar_t* ext, HWND hWnd);
std::wstring   SetSavePathFromButton(wchar_t* filter, wchar_t* ext, HWND hWnd);
std::wstring   SetSavePathFromButtonWithName(std::wstring name, wchar_t* filter, wchar_t* ext, HWND hWnd);
std::wstring   SetFolderFromButton(HWND hWnd);
void		   PushLogMessage(HWND hWnd, std::wstring msg);
