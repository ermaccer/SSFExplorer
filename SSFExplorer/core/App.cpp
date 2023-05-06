#include "App.h"
#include "..\code\SSFExplorer.h"
#include "..\resource.h"
#include "FileFunctions.h"
#include <CommCtrl.h>
#include <windowsx.h>
#include <filesystem>
#include "IniReader.h"

HINSTANCE        eApp::hInst;
HWND             eApp::hWindow;
HWND             eApp::hList;
HWND             eApp::hDataBox;
HWND             eApp::hLog;
HWND             eApp::hTable;
HMENU            eApp::hMenu;
DWORD            eApp::dwSel;
BOOL             eApp::bIsReady;
BOOL             eApp::bIsIni;
int              eApp::nGameMode;

ssfexplorer_settings eApp::tmpSettings;


SSFExplorer*  eApp::pSSFExplorer;


INT_PTR CALLBACK eApp::Process(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	hWindow = hDlg;
	HANDLE hicon = 0;



	if (!bIsReady)
	{
		EnableWindow(GetDlgItem(hDlg, SSF_EXTRACT), FALSE);
		EnableWindow(GetDlgItem(hDlg, SSF_EXTRACT_ALL), FALSE);
		EnableWindow(GetDlgItem(hDlg, SSF_EXPORT), FALSE);
		EnableWindow(GetDlgItem(hDlg, SSF_TEXTURE), FALSE);
		EnableWindow(GetDlgItem(hDlg, SSF_BUILD_FILE), FALSE);
		EnableWindow(GetDlgItem(hDlg, SSF_SELECT_BUILD_FILE), FALSE);
		EnableWindow(GetDlgItem(hDlg, MAKE_SSF), FALSE);
		EnableMenuItem(hMenu, ID_FILE_CLOSE, MF_DISABLED);
		EnableMenuItem(hMenu, ID_EXPORTALLTEXTURES_DEFAULT, MF_DISABLED);
		EnableMenuItem(hMenu, ID_EXPORTALLTEXTURES_TGA, MF_DISABLED);
		EnableMenuItem(hMenu, ID_EXPORTALLTEXTURES_WITHALPHACHANNEL32779, MF_DISABLED);
	}
	else
	{
		if (bIsIni)
		{
			EnableWindow(GetDlgItem(hDlg, SSF_EXTRACT), FALSE);
			EnableWindow(GetDlgItem(hDlg, SSF_EXTRACT_ALL), FALSE);
			EnableWindow(GetDlgItem(hDlg, SSF_EXPORT), FALSE);
			EnableWindow(GetDlgItem(hDlg, SSF_BUILD_FILE), TRUE);
			EnableWindow(GetDlgItem(hDlg, SSF_SELECT_BUILD_FILE), TRUE);
			EnableWindow(GetDlgItem(hDlg, SSF_TEXTURE), FALSE);
			EnableWindow(GetDlgItem(hDlg, MAKE_SSF), TRUE);
		}
		else
		{

			EnableWindow(GetDlgItem(hDlg, SSF_EXTRACT), TRUE);
			EnableWindow(GetDlgItem(hDlg, SSF_EXTRACT_ALL), TRUE);
			EnableWindow(GetDlgItem(hDlg, SSF_EXPORT), TRUE);
			EnableWindow(GetDlgItem(hDlg, SSF_BUILD_FILE), FALSE);
			EnableWindow(GetDlgItem(hDlg, SSF_SELECT_BUILD_FILE), FALSE);
			EnableWindow(GetDlgItem(hDlg, MAKE_SSF), FALSE);
			EnableWindow(GetDlgItem(hDlg, SSF_TEXTURE), TRUE);
			EnableMenuItem(hMenu, ID_EXPORTALLTEXTURES_DEFAULT, MF_ENABLED);
			EnableMenuItem(hMenu, ID_EXPORTALLTEXTURES_WITHALPHACHANNEL32779, MF_ENABLED);
			EnableMenuItem(hMenu, ID_EXPORTALLTEXTURES_TGA, MF_ENABLED);

		}


		EnableMenuItem(hMenu, ID_FILE_CLOSE, MF_ENABLED);
	}




	switch (message)
	{
	case WM_INITDIALOG:
		Reset();
		hLog = GetDlgItem(hDlg, SSF_LOG);
		hList = GetDlgItem(hDlg, SSF_F_LIST);
		hDataBox = GetDlgItem(hDlg, SSF_DATA_BOX);
		PushLogMessage(hList, L"SSF Explorer v" + (std::wstring)SSF_EXPLORER_VERSION + L" ready!");
		hMenu = GetMenu(hDlg);
		pSSFExplorer = new SSFExplorer();
		pSSFExplorer->ReadSettings();
		hicon = LoadImage(GetModuleHandleW(NULL), MAKEINTRESOURCEW(IDI_SSFEXPLORER), IMAGE_ICON, 0, 0, LR_DEFAULTCOLOR | LR_DEFAULTSIZE);
		SendMessage(hDlg, WM_SETICON, ICON_BIG, (LPARAM)hicon);
		if (pSSFExplorer->m_settings.m_bFirstRun)
			DialogBox(hInst, MAKEINTRESOURCE(SSF_SETTINGS), hDlg, Settings);


		CreateTooltip(GetDlgItem(hDlg, SSF_FIGHTER_FIX), L"Check this option if the archive has FACEDAM file.");
		CreateTooltip(GetDlgItem(hDlg, PALFIX), L"Check this option if the exported texture has wrong colors.");
		return (INT_PTR)TRUE;
	case WM_CLOSE:
		EndDialog(hDlg, LOWORD(wParam));
		return (INT_PTR)TRUE;
	case WM_COMMAND:

		if (LOWORD(wParam) == ID_FILE_OPEN)
		{
			pSSFExplorer = new SSFExplorer();
			pSSFExplorer->Init(&hLog, &hList,&hDataBox);
			pSSFExplorer->OpenFile(SetPathFromButton(L"(Super) Section File (*.ssf)(*.sec)\0*.ssf;*.sec\0Unknown (.dat)\0*.dat;All Files (*.*)\0*.*\0", L"ssf", hDlg));
		}

		if (LOWORD(wParam) == ID_FILE_OPENINI)
		{
			pSSFExplorer = new SSFExplorer();
			pSSFExplorer->Init(&hLog, &hList, &hDataBox);
			pSSFExplorer->OpenINI(SetPathFromButton(L"Configuration File\0*.ini\0All Files (*.*)\0*.*\0", L"ini", hDlg));
		}
		if (HIWORD(wParam) == LBN_SELCHANGE)
			pSSFExplorer->DisplayFileData();

		if (LOWORD(wParam) == SSF_EXTRACT)
			pSSFExplorer->ExtractSelected();
		if (LOWORD(wParam) == SSF_EXTRACT_ALL)
			pSSFExplorer->ExtractAll();
		if (LOWORD(wParam) == SSF_EXPORT)
			pSSFExplorer->Export();
		if (LOWORD(wParam) == SSF_TEXTURE)
			pSSFExplorer->ExportTexture();
		if (LOWORD(wParam) == ID_EXPORTALLTEXTURES_DEFAULT)
			pSSFExplorer->ExportAllTextures();
		if (LOWORD(wParam) == ID_EXPORTALLTEXTURES_WITHALPHACHANNEL32779)
			pSSFExplorer->ExportAllTextures(true);
		if (LOWORD(wParam) == ID_EXPORTALLTEXTURES_TGA)
			pSSFExplorer->ExportAllTextures(true, OutputImage_TGA);
		if (LOWORD(wParam) == ID_TOOLS_MKDA)
		{
			pSSFExplorer = new SSFExplorer();
			pSSFExplorer->Init(&hLog, &hList, &hDataBox);
			pSSFExplorer->ExtractPAK();
			delete pSSFExplorer;

		}

		if (wParam == IDM_ABOUT)
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hDlg, About);

		if (wParam == ID_FILE_SETTINGS)
			DialogBox(hInst, MAKEINTRESOURCE(SSF_SETTINGS), hDlg, Settings);

		if (wParam == SSF_SELECT_BUILD_FILE)
			SetWindowText(GetDlgItem(hDlg, SSF_BUILD_FILE), (LPCWSTR)SetSavePathFromButton(L"Super Section File (*.ssf)\0*.ssf\0Section File(*.sec)\0*.sec\0Unknown (*.dat)\0*.dat\0\0All Files (*.*)\0*.*\0", L"ssf", hDlg).c_str());

		if (wParam == MAKE_SSF)
			pSSFExplorer->Build();

		if (LOWORD(wParam) == ID_FILE_CLOSE)
			pSSFExplorer->Close();

		if (LOWORD(wParam) == IDM_EXIT)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}

	}
	return (INT_PTR)FALSE;
}

INT_PTR eApp::About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}

INT_PTR eApp::Settings(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	const wchar_t* gameNames[SUPPORTED_GAMES - 1] = {L"Mortal Kombat: Unchained", L"Mortal Kombat: Deception", L"Mortal Kombat: Armageddon",L"Mortal Kombat: Deadly Alliance" };
	const wchar_t* platformNames[TOTAL_PLATFORMS] = { L"PlayStation 2", L"XBOX",L"Nintendo Gamecube/Wii", L"PlayStation Portable" };

	switch (message)
	{
	case WM_INITDIALOG:
		for (int i = 0; i < SUPPORTED_GAMES - 1; i++)
			SendMessage(GetDlgItem(hDlg,SET_GAME), CB_ADDSTRING, 0, (LPARAM)gameNames[i]);
		SendMessage(GetDlgItem(hDlg, SET_GAME), CB_SETCURSEL, 0, 0);
		for (int i = 0; i < TOTAL_PLATFORMS; i++)
			SendMessage(GetDlgItem(hDlg, SET_PLATFORM), CB_ADDSTRING, 0, (LPARAM)platformNames[i]);
		SendMessage(GetDlgItem(hDlg, SET_PLATFORM), CB_SETCURSEL, 0, 0);
		ReadSettings();
		SendMessage(GetDlgItem(hDlg, SET_GAME), CB_SETCURSEL, tmpSettings.m_egGame - 1, 0);
		SendMessage(GetDlgItem(hDlg, SET_PLATFORM), CB_SETCURSEL, tmpSettings.m_epPlatform, 0);
		return (INT_PTR)TRUE;

	case WM_COMMAND:

		if (LOWORD(wParam) == SET_SAVE)
		{
			tmpSettings.m_bFirstRun = 0;
			tmpSettings.m_egGame = (eGames)(SendMessage(GetDlgItem(hDlg, SET_GAME), CB_GETCURSEL, 0, 0) + 1);

			if (tmpSettings.m_egGame == MORTAL_KOMBAT_UNCHAINED)
				SendMessage(GetDlgItem(hDlg, SET_PLATFORM), CB_SETCURSEL, PLATFORM_PSP, 0);
			if (tmpSettings.m_egGame == MORTAL_KOMBAT_ARMAGEDDON || tmpSettings.m_egGame == MORTAL_KOMBAT_DECEPTION 
				|| tmpSettings.m_egGame == MORTAL_KOMBAT_DEADLY_ALLIANCE)
			{
				if (tmpSettings.m_epPlatform == PLATFORM_PSP)
					SendMessage(GetDlgItem(hDlg, SET_PLATFORM), CB_SETCURSEL, PLATFORM_PS2, 0);
			}


			tmpSettings.m_epPlatform = (ePlatforms)SendMessage(GetDlgItem(hDlg, SET_PLATFORM), CB_GETCURSEL, 0, 0);
			SaveSettings();

			if (pSSFExplorer)
				pSSFExplorer->ReadSettings();
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}

		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}

void eApp::ReadSettings()
{

	std::wstring iniPath = getExecutablePath();

	iniPath += L"\\Settings.ini";
	if (std::filesystem::exists(iniPath))
	{
		CIniReader reader(iniPath.c_str());
		tmpSettings.m_bFirstRun = reader.ReadBoolean(L"Settings", L"bFirstRun", false);
		tmpSettings.m_egGame = (eGames)reader.ReadInteger(L"Settings", L"Game", MORTAL_KOMBAT_UNCHAINED);
		tmpSettings.m_epPlatform = (ePlatforms)reader.ReadInteger(L"Settings", L"Platform", PLATFORM_PS2);
		tmpSettings.m_bGuessExtensions = reader.ReadBoolean(L"Settings", L"guess_extensions", false);
	}
	else
	{
		tmpSettings.m_bFirstRun = true;
		tmpSettings.m_egGame = MORTAL_KOMBAT_UNCHAINED;
	}
}

void eApp::SaveSettings()
{

	std::wstring iniPath = getExecutablePath();

	iniPath += L"\\Settings.ini";

	std::ofstream ini(iniPath, std::ofstream::binary);

	ini << "[Settings]\n";
	ini << "bFirstRun = " << tmpSettings.m_bFirstRun << std::endl;
	ini << "game = " << tmpSettings.m_egGame << std::endl;
	ini << "platform = " << tmpSettings.m_epPlatform << std::endl;
	ini << "guess_extensions = " << tmpSettings.m_bGuessExtensions << std::endl;
	ini.close();
}

void eApp::Reset()
{
	nGameMode = 0;
}

void eApp::CreateTooltip(HWND hWnd, LPCWSTR text)
{
	HWND hwndTT = CreateWindowEx(WS_EX_TOPMOST, TOOLTIPS_CLASS, NULL, WS_POPUP |
		TTS_NOPREFIX | TTS_ALWAYSTIP,
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, hWnd, NULL,
		hInst, NULL);
	SetWindowPos(hwndTT, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

	TOOLINFO ti;
	ti.cbSize = sizeof(TOOLINFO);
	ti.uFlags = TTF_SUBCLASS | TTF_IDISHWND;
	ti.hwnd = hWindow;
	ti.hinst = NULL;
	ti.uId = (UINT_PTR)hWnd;
	ti.lpszText = (LPWSTR)text;

	RECT rect;
	GetClientRect(hWnd, &rect);

	ti.rect.left = rect.left;
	ti.rect.top = rect.top;
	ti.rect.right = rect.right;
	ti.rect.bottom = rect.bottom;

	SendMessage(hwndTT, TTM_ADDTOOL, 0, (LPARAM)&ti);
}

void eApp::UpdateGameChange()
{

}

void eApp::Begin()
{
	pSSFExplorer = nullptr;
	DialogBox(hInst, MAKEINTRESOURCE(IDD_SSF_EXPLORER), 0, Process);
}
