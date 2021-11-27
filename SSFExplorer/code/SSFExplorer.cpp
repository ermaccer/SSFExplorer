#include "..\core\App.h"
#include "MKArchive.h"
#include "MKScript.h"
#include <CommCtrl.h>
#include <shlobj.h>
#include <WinUser.h>
#include <filesystem>
#include <windowsx.h>
#include "..\core\IniReader.h"
#include "..\core\FileFunctions.h"
#include "..\resource.h"
#include <memory>
#include <iostream>
#include <string>
#include "image.h"

void SSFExplorer::Init(HWND* log, HWND* list, HWND* data)
{
	eApp::bIsReady = false;
	hLogBox = log;
	hFilesList = list;
	hDataBox = data;
	m_settings.m_bFirstRun = true;
	m_bUsesFilenames = false;
	m_bFighterFix = false;
	ReadSettings();


}

void SSFExplorer::ReadSettings()
{
	std::wstring iniPath = getExecutablePath();

	iniPath += L"\\Settings.ini";
	if (std::filesystem::exists(iniPath))
	{
		CIniReader reader(iniPath.c_str());
		m_settings.m_bFirstRun = reader.ReadBoolean(L"Settings", L"bFirstRun", false);
		m_settings.m_egGame = (eGames)reader.ReadInteger(L"Settings", L"game", GAME_NULL);
		m_settings.m_epPlatform = (ePlatforms)reader.ReadInteger(L"Settings", L"platform", PLATFORM_PS2);
	}
	else
	{
		m_settings.m_bFirstRun = true;
		m_settings.m_egGame = GAME_NULL;
	}
}

void SSFExplorer::OpenFile(std::wstring input)
{
	if (pFile.is_open())
		pFile.close();

	Files.clear();
	InputPath = L" ";
	OutputPath = L" ";
	m_bUsesFilenames = false;
	m_bFighterFix = false;
	InputPath = input;
	SendMessage(*hFilesList, LB_RESETCONTENT, 0, 0);
	SetWindowText(*hLogBox, L"");
	SetWindowText(*hDataBox, L"");
	SetWindowText(GetDlgItem(eApp::hWindow, SSF_FILENAME), L"");
	SetWindowText(GetDlgItem(eApp::hWindow, SSF_BUILD_FILE), L"");
	ReadFile();
}

void SSFExplorer::OpenINI(std::wstring input)
{
	Files.clear();
	InputPath = L" ";
	OutputPath = L" ";
	m_bUsesFilenames = false;
	m_bFighterFix = false;
	InputPath = input;
	SendMessage(*hFilesList, LB_RESETCONTENT, 0, 0);
	SetWindowText(*hLogBox, L"");
	SetWindowText(*hDataBox, L"");
	SetWindowText(GetDlgItem(eApp::hWindow, SSF_FILENAME), L"");
	SetWindowText(GetDlgItem(eApp::hWindow, SSF_BUILD_FILE), L"");
	ReadINI();
}

void SSFExplorer::ReadFile()
{
	if (InputPath.empty())
		return;
	pFile.open(InputPath, std::ifstream::binary);

	if (!pFile.is_open())
	{
		MessageBox(eApp::hWindow, L"Failed to open file!", L"Error", MB_ICONERROR);
		return;
	}

	if (pFile.is_open())
	{
		Log(L"Opening: " + wsplitString(InputPath, true));
		section_file_header sec;
		pFile.read((char*)&sec, sizeof(section_file_header));

		if (m_settings.m_egGame == MORTAL_KOMBAT_DEADLY_ALLIANCE)
		{
			pFile.seekg(-4, pFile.cur);
		}


		if ((sec.header == ' CES') && !(m_settings.m_epPlatform == PLATFORM_GC_WII))
		{
			MessageBox(eApp::hWindow, L"Input file is a GC/WII file, but the platform setting doesn't match!", L"Error", MB_ICONERROR);
			return;
		}


		if (m_settings.m_epPlatform == PLATFORM_GC_WII)
		{
			if (!(sec.header == ' CES'))
			{
				MessageBox(eApp::hWindow, L"Input file is not a valid Section file!", L"Error", MB_ICONERROR);
				return;
			}
		}
		else
		{
			if (!(sec.header == 'SEC '))
			{
				MessageBox(eApp::hWindow, L"Input file is not a valid Section file!", L"Error", MB_ICONERROR);
				return;
			}
		}


		if (m_settings.m_epPlatform == PLATFORM_GC_WII)
		{
			Log(L"Processing as GC/Wii file");
			changeEndINT(&sec.files);
			changeEndINT(&sec.fileSize);
			changeEndINT(&sec.pad);
			changeEndINT(&sec.stringSize);
			changeEndINT(&sec.unknown);
			changeEndINT(&sec.version);
		}



		Log(L"Files: " + std::to_wstring(sec.files));

		for (int i = 0; i < sec.files; i++)
		{
			section_file_entry ent;
			pFile.read((char*)&ent, sizeof(section_file_entry));

			if (m_settings.m_egGame == MORTAL_KOMBAT_DEADLY_ALLIANCE)
			{
				pFile.seekg(-4, pFile.cur);

				// da
				// type
				// offset
				// size

			}


			file_entry file;
			file.ent = ent;

			if (m_settings.m_epPlatform == PLATFORM_GC_WII)
			{
				changeEndINT(&file.ent.offset);
				changeEndINT(&file.ent.size);
				changeEndINT((int*)&file.ent.type);
				changeEndINT(&file.ent.stringOffset);
			}

			Files.push_back(file);
		}

		for (unsigned int i = 0; i < Files.size(); i++)
			Log(L"Size: " + std::to_wstring(Files[i].ent.size) + L" Type: " + std::to_wstring(Files[i].ent.type));

		if (m_settings.m_egGame == MORTAL_KOMBAT_DEADLY_ALLIANCE)
		{
			if (sec.stringSize <= NO_STRING_DATA)
			{
				MessageBox(eApp::hWindow, L"Deadly Alliance subarchives are not supported!", L"Error", MB_ICONERROR);
				return;
			}

		}


		if (sec.stringSize >= NO_STRING_DATA || sec.stringSize <= 20)
		{

			if (!ReadNamesFromFile(InputPath))
			{
				// make file names
				for (unsigned int i = 0; i < Files.size(); i++)
				{
					std::wstring name;
					name = std::to_wstring(i);
					name += L".dat";
					Files[i].name = name;
				}

			}
		}
		else
		{
			m_bUsesFilenames = true;
			std::string line;
			for (unsigned int i = 0; i < Files.size(); i++)
			{
				std::getline(pFile, line, '\0');
				std::string str(line.c_str(), strlen(line.c_str()));
				std::wstring wstr(str.length(), L' ');
				std::copy(str.begin(), str.end(), wstr.begin());
				Files[i].name = wstr;
			}
		}

		SetWindowText(GetDlgItem(eApp::hWindow, SSF_FILENAME), wsplitString(InputPath,true).c_str());

		ListFiles();
		m_secHeader = sec;

		eApp::bIsIni = FALSE;
		eApp::bIsReady = TRUE;

	}
}

void SSFExplorer::ReadINI()
{
	if (InputPath.empty())
		return;

	if (!std::filesystem::exists(InputPath))
	{
		MessageBox(eApp::hWindow, L"Failed to open INI file!", L"Error", MB_ICONERROR);
		return;
	}

	Log(L"Loading INI: " + InputPath);

	CIniReader reader(InputPath.c_str());

	m_secHeader.files = reader.ReadInteger(L"Build.Settings", L"Files", 0);
	m_bUsesFilenames = reader.ReadInteger(L"Build.Settings", L"UsesFilenames", false);
	m_wstrBuildFolder = reader.ReadString(L"Build.Settings", L"Folder ", 0);



	wchar_t tmp[128] = {};
	for (int i = 0; i < m_secHeader.files; i++)
	{
		wsprintf(tmp, L"File%d", i);
		eSectionFileTypes type = (eSectionFileTypes)reader.ReadInteger(tmp, L"Type", 1);
		std::wstring name = reader.ReadString(tmp, L"Name", L"null");
		file_entry ent;
		ent.ent.type = type;
		ent.name = name;
		Files.push_back(ent);
	}
	ListFiles();

	SetWindowText(GetDlgItem(eApp::hWindow, SSF_FILENAME), L"Files to build");

	eApp::bIsIni = TRUE;
	eApp::bIsReady = TRUE;

}

void SSFExplorer::ListFiles()
{
	SendMessage(*hFilesList, LB_RESETCONTENT, 0, 0);

	for (unsigned int i = 0; i < Files.size(); i++)
		SendMessage(*hFilesList, LB_ADDSTRING, 0, (LPARAM)Files[i].name.c_str());

}

bool SSFExplorer::ReadNamesFromFile(std::wstring name)
{
	if (m_settings.m_egGame == MORTAL_KOMBAT_DEADLY_ALLIANCE)
		return false;

	std::wstring filename = wsplitString(name, true);
	filename = filename.substr(0, filename.length() - 4);
	
	std::wstring path = getExecutablePath();
	path += L"data";

	switch (m_settings.m_egGame)
	{
	case MORTAL_KOMBAT_UNCHAINED:
		path += L"\\mku\\";
		break;
	case MORTAL_KOMBAT_DECEPTION:
		path += L"\\mkd\\";
		break;
	case MORTAL_KOMBAT_ARMAGEDDON:
		path += L"\\mka\\";
		break;
	default:
		path += L"\\";
		break;
	}

	path += filename;
	path += L".txt";

	std::ifstream pList(path);
	Log(L"FILELIST: " + path);
	if (pList)
	{
		Log(L"Reading: " + filename + L".txt");
		std::string line;
		for (unsigned int i = 0; i < Files.size(); i++)
		{
			std::getline(pList, line);
			std::string str(line.c_str(), strlen(line.c_str()));
			std::wstring wstr(str.length(), L' ');
			std::copy(str.begin(), str.end(), wstr.begin());
			Files[i].name = wstr;
		}
		pList.close();
		return true;
	}

	Log(L"Could not find: " + filename + L".txt" + L" - names will be generated!");

	return false;
}

void SSFExplorer::DisplayFileData()
{
	if (eApp::bIsIni)
		return;

	DWORD dwSel = SendMessage(*hFilesList, LB_GETCURSEL, 0, 0);
	SetWindowText(*hDataBox, L"");
	if (dwSel >= 0 && eApp::bIsReady)
	{
		std::vector<section_file_entry> previewFiles;
		std::string line;
		std::wstring ver;
		AddData(L"Size: " + std::to_wstring(Files[dwSel].ent.size));
		AddData(L"Offset: " + std::to_wstring(Files[dwSel].ent.offset));
		AddData(GetSectionTypeName(Files[dwSel].ent.type));

		switch (Files[dwSel].ent.type)
		{
		case FILE_TYPE_SECTION:
		case FILE_TYPE_ANIMATION_SECTION:
			previewFiles.clear();
			pFile.seekg(Files[dwSel].ent.offset,pFile.beg);

			section_file_header sec;
			pFile.read((char*)&sec, sizeof(section_file_header));

			if (sec.header == 'SEC ')
			{
				AddData(L"Files: " + std::to_wstring(sec.files));

				for (unsigned int i = 0; i < sec.files; i++)
				{
					section_file_entry ent;
					pFile.read((char*)&ent, sizeof(section_file_entry));
					previewFiles.push_back(ent);
				}


				for (unsigned int i = 0; i < previewFiles.size(); i++)
				{
					std::getline(pFile, line, '\0');
					std::string str(line.c_str(), strlen(line.c_str()));
					std::wstring wstr(str.length(), L' ');
					std::copy(str.begin(), str.end(), wstr.begin());
					AddData(wstr + L" Size: " + std::to_wstring(previewFiles[i].size));

				}
			}
			else
				pFile.seekg(Files[dwSel].ent.offset, pFile.beg);

			
			break;
		case FILE_TYPE_SCRIPT:
			break;
		case FILE_TYPE_MODEL:
			pFile.seekg(Files[dwSel].ent.offset, pFile.beg);
			pFile.seekg(sizeof(int) * 4, pFile.cur);

			unsigned int rwVer;
			unsigned int tmp;
			pFile.read((char*)&tmp, sizeof(unsigned int));

			if (tmp & 0xFFFF0000)
				rwVer = (tmp >> 14 & 0x3FF00) + 0x30000 | (tmp >> 16 & 0x3F);
			else
				rwVer =  tmp << 8;

			wchar_t rw[24];
			wsprintf(rw, L"0x%X", rwVer);
			ver = rw;
			AddData(L"RenderWare Version: " + ver);

			break;
		default:
			break;
		}
	}
}

void SSFExplorer::ExtractSelected()
{
	DWORD dwSel = SendMessage(*hFilesList, LB_GETCURSEL, 0, 0);

	if (dwSel >= 0)
	{
		OutputPath = SetSavePathFromButtonWithName(Files[dwSel].name, L"All Files (*.*)\0*.*\0",L"",eApp::hWindow);
		if (OutputPath.empty())
			return;


		std::ofstream oFile(OutputPath, std::ofstream::binary);
		std::unique_ptr<char[]> dataBuff = std::make_unique<char[]>(Files[dwSel].ent.size);
		pFile.seekg(Files[dwSel].ent.offset, pFile.beg);
		pFile.read(dataBuff.get(), Files[dwSel].ent.size);
		oFile.write(dataBuff.get(), Files[dwSel].ent.size);

		Log(L"File: " + Files[dwSel].name + L" extracted!");

	}
}

void SSFExplorer::ExtractAll()
{
	std::wstring folder = SetFolderFromButton(eApp::hWindow);

	if (folder.empty())
		return;

	std::filesystem::current_path(folder);

	for (unsigned int i = 0; i < Files.size(); i++)
	{
		std::ofstream oFile(Files[i].name, std::ofstream::binary);
		std::unique_ptr<char[]> dataBuff = std::make_unique<char[]>(Files[i].ent.size);
		pFile.seekg(Files[i].ent.offset, pFile.beg);
		pFile.read(dataBuff.get(), Files[i].ent.size);
		oFile.write(dataBuff.get(), Files[i].ent.size);

		Log(L"File: " + Files[i].name + L" extracted!");
	}


}

void SSFExplorer::Export()
{
	if (m_settings.m_egGame == MORTAL_KOMBAT_DEADLY_ALLIANCE)
	{
		MessageBox(eApp::hWindow, L"Not available for Deadly Alliance!", L"Error", MB_ICONERROR);
		return;
	}


	std::wstring folder = SetFolderFromButton(eApp::hWindow);

	if (folder.empty())
		return;

	std::filesystem::current_path(folder);

	std::wstring folderName = wsplitString(InputPath, true);
	// make ini
	folderName = folderName.substr(0, folderName.length() - 4);
	std::wofstream ini(folderName + L".ini", std::ofstream::binary);
	folderName += L"_files";

	ini << L"[Build.Settings]\n";
	ini << L"Files = " << m_secHeader.files << std::endl;
	ini << L"UsesFilenames = " << m_bUsesFilenames << std::endl;
	ini << L"Folder = " << folderName.c_str() << std::endl << std::endl;;

	for (unsigned int i = 0; i < Files.size(); i++)
	{
		ini << L"[File" + std::to_wstring(i) + L"]" << std::endl;
		ini << L"Type = " + std::to_wstring(Files[i].ent.type) << std::endl;
		ini << L"Name = " + Files[i].name << std::endl;
	}
	Log(L"INI file created!");
	ini.close();

	std::filesystem::create_directory(folderName);

	std::filesystem::current_path(folderName);

	for (unsigned int i = 0; i < Files.size(); i++)
	{
		std::ofstream oFile(Files[i].name, std::ofstream::binary);
		std::unique_ptr<char[]> dataBuff = std::make_unique<char[]>(Files[i].ent.size);
		pFile.seekg(Files[i].ent.offset, pFile.beg);
		pFile.read(dataBuff.get(), Files[i].ent.size);
		oFile.write(dataBuff.get(), Files[i].ent.size);

		Log(L"File: " + Files[i].name + L" extracted!");
	}


}

void SSFExplorer::ExportTexture()
{
	if (m_settings.m_egGame == MORTAL_KOMBAT_DEADLY_ALLIANCE)
	{
		MessageBox(eApp::hWindow, L"Not available for Deadly Alliance!", L"Error", MB_ICONERROR);
		return;
	}


	if (m_settings.m_epPlatform == PLATFORM_XBOX || m_settings.m_epPlatform == PLATFORM_GC_WII)
	{
		MessageBox(eApp::hWindow, L"XBOX and Wii/Gamecube textures are not supported", 0, MB_ICONERROR);
		return;
	}

	if (m_settings.m_egGame == MORTAL_KOMBAT_ARMAGEDDON && m_settings.m_egGame == MORTAL_KOMBAT_DECEPTION)
	{
		if (m_settings.m_epPlatform == PLATFORM_PSP)
		{
			MessageBox(eApp::hWindow, L"Invalid platform!", 0, MB_ICONERROR);
			return;
		}
	}

	DWORD dwSel = SendMessage(*hFilesList, LB_GETCURSEL, 0, 0);

	if (dwSel >= 0)
	{
		if (!(Files[dwSel].ent.type == FILE_TYPE_ANIMATION_TEXTURE || Files[dwSel].ent.type == FILE_TYPE_FE_TEXTURE))
		{
			MessageBox(eApp::hWindow, L"This file is not a texture!", 0, MB_ICONWARNING);
			return;
		}

		int baseOffset = Files[dwSel].ent.offset;

		pFile.seekg(baseOffset, pFile.beg);

		if (m_settings.m_epPlatform == PLATFORM_PSP && m_settings.m_egGame == MORTAL_KOMBAT_UNCHAINED)
		{
			unsigned char len;
			pFile.read((char*)&len, sizeof(char));

			
			std::wstring outputName = Files[dwSel].name;


			if (len > 0)
			{
				std::unique_ptr<char[]> text = std::make_unique<char[]>(len);
				pFile.read(text.get(), len);
				std::string str(text.get(), len);

				std::wstring wstr(str.length(), L' ');
				std::copy(str.begin(), str.end(), wstr.begin());
				outputName = wstr;
			}




			int x = 0;
			int y = 0;

			pFile.read((char*)&x, sizeof(int));
			pFile.read((char*)&y, sizeof(int));

			if (x > 4096 || y > 4096)
			{
				MessageBox(eApp::hWindow, L"Invalid texture data, most likely not a texture file!", 0, 0);
				return;
			}

			image_data image;
			image.width = x;
			image.height = y;
			image.bits = 8;

			pFile.seekg(baseOffset + PSP_HEADER_SIZE, pFile.beg);

			std::unique_ptr<pal_psp_data[]> palBuff = std::make_unique<pal_psp_data[]>(PSP_PAL_SIZE / sizeof(pal_psp_data));

			for (int i = 0; i < PSP_PAL_SIZE / sizeof(pal_psp_data); i++)
				pFile.read((char*)&palBuff[i], sizeof(pal_psp_data));
	
			pFile.seekg(baseOffset + PSP_HEADER_SIZE + PSP_PAL_SIZE, pFile.beg);

			int dataSize = image.width * image.height;

			std::unique_ptr<char[]> dataBuff = std::make_unique<char[]>(dataSize);
			pFile.read(dataBuff.get(), dataSize);

			std::unique_ptr<char[]> dataOut = std::make_unique<char[]>(dataSize);

			std::unique_ptr<char[]> dataOutCopy = std::make_unique<char[]>(dataSize);

			outputName += L".bmp";

			std::wstring output = SetSavePathFromButtonWithName(outputName, L"Bitmap Image File (*.bmp)\0*.bmp\0All Files (*.*)\0*.*\0", L"bmp", eApp::hWindow);

			if (output.empty())
				return;

			std::ofstream oFile(output, std::ofstream::binary);

			// unswizzle
			Unswizzlers::PSP((unsigned int*)dataOut.get(), (unsigned int*)dataBuff.get(), image);

			// create bmp
			bmp_header bmp;
			bmp_info_header bmpf;
			bmp.bfType = 'MB';
			bmp.bfSize = dataSize;
			bmp.bfReserved1 = 0;
			bmp.bfReserved2 = 0;
			bmp.bfOffBits = sizeof(bmp_header) + sizeof(bmp_info_header) + PSP_PAL_SIZE;
			bmpf.biSize = sizeof(bmp_info_header);
			bmpf.biWidth = image.width;
			bmpf.biHeight = image.height;
			bmpf.biPlanes = 1;
			bmpf.biBitCount = image.bits;
			bmpf.biCompression = 0;
			bmpf.biXPelsPerMeter = 2835;
			bmpf.biYPelsPerMeter = 2835;
			bmpf.biClrUsed = 256;
			bmpf.biClrImportant = 0;

			// write headers
			oFile.write((char*)&bmp, sizeof(bmp_header));
			oFile.write((char*)&bmpf, sizeof(bmp_info_header));

			// write colors
			for (int i = 0; i < PSP_PAL_SIZE / sizeof(pal_psp_data); i++)
			{
				// swap red and blue
				oFile.write((char*)&palBuff[i].b, sizeof(char));
				oFile.write((char*)&palBuff[i].g, sizeof(char));
				oFile.write((char*)&palBuff[i].r, sizeof(char));
				oFile.write((char*)&palBuff[i].a, sizeof(char));
			}
			for (int y = image.height - 1; y >= 0; y--)
			{
				for (int x = 0; x < image.width; x++)
				{
					oFile.write((char*)&dataOut[x + (y * image.width)], sizeof(char));
				}
			}
			//oFile.write(dataOut.get(), dataSize);
			Log(L"Texture " + outputName + L" saved!");
		}
		else if (m_settings.m_epPlatform == PLATFORM_PS2)
		{
			// armageddon check

			bool isArmageddon = false;

			char check[4];
			pFile.read((char*)&check, sizeof(check));

			if (check[0] == 0x00 && check[1] == 0x00 && check[2] == 0x00 && check[3] == 0x00)
			{
				Log(L"Armageddon texture detected!");
				isArmageddon = true;
			}



			if (!isArmageddon)
				pFile.seekg(baseOffset, pFile.beg);

			unsigned char len;
			pFile.read((char*)&len, sizeof(char));


			std::wstring outputName = Files[dwSel].name;


			if (len > 0)
			{
				std::unique_ptr<char[]> text = std::make_unique<char[]>(len);
				pFile.read(text.get(), len);
				std::string str(text.get(), len);

				std::wstring wstr(str.length(), L' ');
				std::copy(str.begin(), str.end(), wstr.begin());
				outputName = wstr;
			}


			int x = 0;
			int y = 0;

			pFile.read((char*)&x, sizeof(int));
			pFile.read((char*)&y, sizeof(int));

			if (x > 4096 || y > 4096)
			{
				MessageBox(eApp::hWindow, L"Invalid texture data, most likely not a texture file!", 0, 0);
				return;
			}

			image_data image;
			image.width = x;
			image.height = y;
			image.bits = 8;


			int offset = 0;


			int adjust = 128;

			if (IsDlgButtonChecked(eApp::hWindow, PALFIX) == BST_CHECKED)
			{
				Log(L"Adjusting palette offset");
				adjust = 80;
			}
	

			pFile.seekg(baseOffset + PS2_HEADER_SIZE + (image.height * image.width) + adjust, pFile.beg);

			Log(L"Palette offset: " + std::to_wstring(baseOffset + PS2_HEADER_SIZE + (image.height * image.width) + adjust));


			std::unique_ptr<int[]> palBuff = std::make_unique<int[]>(PSP_PAL_SIZE / sizeof(pal_psp_data));

			for (int i = 0; i < PSP_PAL_SIZE / sizeof(pal_psp_data); i++)
				pFile.read((char*)&palBuff[i], sizeof(int));
	
			
			pFile.seekg(baseOffset + PS2_HEADER_SIZE, pFile.beg);

			Log(L"Texture offset: " + std::to_wstring(baseOffset + PS2_HEADER_SIZE));

			int dataSize = image.width * image.height;

			std::unique_ptr<char[]> dataBuff = std::make_unique<char[]>(dataSize);
			pFile.read(dataBuff.get(), dataSize);

			std::unique_ptr<char[]> dataOut = std::make_unique<char[]>(dataSize);

			outputName += L".bmp";

			std::wstring output = SetSavePathFromButtonWithName(outputName, L"Bitmap Image File (*.bmp)\0*.bmp\0All Files (*.*)\0*.*\0", L"bmp", eApp::hWindow);

			if (output.empty())
				return;

			std::ofstream oFile(output, std::ofstream::binary);

			Unswizzlers::PS2((unsigned char*)dataOut.get(), (unsigned char*)dataBuff.get(), image);


			// create bmp
			bmp_header bmp;
			bmp_info_header bmpf;
			bmp.bfType = 'MB';
			bmp.bfSize = dataSize;
			bmp.bfReserved1 = 0;
			bmp.bfReserved2 = 0;
			bmp.bfOffBits = sizeof(bmp_header) + sizeof(bmp_info_header) + PSP_PAL_SIZE;
			bmpf.biSize = sizeof(bmp_info_header);
			bmpf.biWidth = image.width;
			bmpf.biHeight = image.height;
			bmpf.biPlanes = 1;
			bmpf.biBitCount = image.bits;
			bmpf.biCompression = 0;
			bmpf.biXPelsPerMeter = 2835;
			bmpf.biYPelsPerMeter = 2835;
			bmpf.biClrUsed = 256;
			bmpf.biClrImportant = 0;

			// write headers
			oFile.write((char*)&bmp, sizeof(bmp_header));
			oFile.write((char*)&bmpf, sizeof(bmp_info_header));


			// convert colors
			for (int i = 0; i < 256; i++)
			{
				int j;
				if ((i & 0x18) == 0x10)
				{
					int tmp;
					j = i ^ 0x18;
					tmp = palBuff[i];
					palBuff[i] = palBuff[j];
					palBuff[j] = tmp;
				}
			}


			// write colors
			for (int i = 0; i < 256; i++)
			{
				// swap rgb to bgr
				char* ptr = (char*)&palBuff[i];
				char red = *(char*)(ptr);
				char blue = *(char*)(ptr + 2);
				*(char*)(ptr) = blue;
				*(char*)(ptr + 2) = red;
				oFile.write((char*)&palBuff[i], sizeof(int));

			}
			for (int y = image.height - 1; y >= 0; y--)
			{
				for (int x = 0; x < image.width; x++)
				{
					oFile.write((char*)&dataOut[x + (y * image.width)], sizeof(char));
				}
			}

			//oFile.write(dataOut.get(), dataSize);
			Log(L"Texture " + outputName + L" saved!");
		}
		else
		{
			MessageBox(eApp::hWindow, L"Cannot export the texture!", 0, MB_ICONWARNING);
			return;
		}
	}
}

void SSFExplorer::Build()
{

	bool failedBuild = false;	

	HWND hPathBox = GetDlgItem(eApp::hWindow, SSF_BUILD_FILE);

	if (GetWindowTextLength(hPathBox) == 0)
	{
		MessageBox(eApp::hWindow, L"Output file path cannot be empty!", 0, MB_ICONERROR);
		return;
	}

	wchar_t szOutputPath[_MAX_PATH] = {};

	GetWindowText(hPathBox, (LPWSTR)szOutputPath, GetWindowTextLength(hPathBox) + 1);

	std::ofstream oFile(szOutputPath, std::ofstream::binary);

	if (!oFile.is_open())
	{
		MessageBox(eApp::hWindow, L"Failed to open file for writing!", 0, MB_ICONERROR);
		return;
	}


	m_bFighterFix = IsDlgButtonChecked(eApp::hWindow, SSF_FIGHTER_FIX);

	Log(L"Saving header");


	m_secHeader.header = 'SEC ';
	m_secHeader.files = Files.size();
	m_secHeader.pad = 0;
	m_secHeader.fileSize = 0;
	m_secHeader.stringSize = 0;
	m_secHeader.unknown = 0;
	m_secHeader.version = 4;

	std::wstring path = wsplitString(InputPath, false);
	path += L"\\";
	path += m_wstrBuildFolder;

	Log(path);

	std::filesystem::current_path(path);

	for (unsigned int i = 0; i < Files.size(); i++)
	{
		if (std::filesystem::exists(Files[i].name))
		{
			if (!m_bUsesFilenames)
				m_secHeader.fileSize += makePad(std::filesystem::file_size(Files[i].name), DEFAULT_SSF_PADSIZE);
			else
				m_secHeader.fileSize += std::filesystem::file_size(Files[i].name);
		}

		else
		{
			std::wstring error = L"File ";
			error += Files[i].name;
			error += L" doesn't exist!";
			MessageBox(eApp::hWindow, error.c_str(), 0, MB_ICONERROR);
			failedBuild = true;
			break;
	
		}
	}

	if (failedBuild)
	{
		MessageBox(eApp::hWindow, L"Build failed!", 0, MB_ICONERROR);
		return;
	}

	if (m_bUsesFilenames)
	{
		for (unsigned int i = 0; i < Files.size(); i++)
		{
			m_secHeader.stringSize += Files[i].name.length() + 1;
		}
	}
	else
		m_secHeader.stringSize = NO_STRING_DATA;


	if (m_bUsesFilenames)
	{
		int hsize = sizeof(section_file_header) + (sizeof(section_file_entry) * Files.size()) + m_secHeader.stringSize;
		m_secHeader.fileSize += hsize;

		int psize = makePad(hsize, SEC_PADSIZE) - hsize;
		m_secHeader.fileSize += psize;

	}
	else
		m_secHeader.fileSize += makePad(sizeof(section_file_header), DEFAULT_SSF_PADSIZE);

	if (m_settings.m_epPlatform == PLATFORM_GC_WII)
	{
		Log(L"Saving as GC/Wii file");
		changeEndINT(&m_secHeader.header);

		changeEndINT(&m_secHeader.files);
		changeEndINT(&m_secHeader.fileSize);
		changeEndINT(&m_secHeader.pad);
		changeEndINT(&m_secHeader.stringSize);
		changeEndINT(&m_secHeader.unknown);
		changeEndINT(&m_secHeader.version);
	}

	oFile.write((char*)&m_secHeader, sizeof(section_file_header));

	int baseOffset = DEFAULT_SSF_PADSIZE;
	int stringsSize = 0;

	for (unsigned int i = 0; i < Files.size(); i++)
		stringsSize += Files[i].name.length() + 1;


	int headerSize = sizeof(section_file_header) + (sizeof(section_file_entry) * Files.size()) + stringsSize;


	int padOffset = makePad(headerSize, SEC_PADSIZE) - headerSize;

	if (m_bFighterFix)
		padOffset = 0;

	if (m_bUsesFilenames)
		baseOffset = headerSize + padOffset;


	int baseString = 0;

	Log(L"Saving file headers");

	bool firstFilePadFix = true;

	for (unsigned int i = 0; i < Files.size(); i++)
	{

		int fsize = makePad(std::filesystem::file_size(Files[i].name), DEFAULT_SSF_PADSIZE);
		Files[i].ent.size = std::filesystem::file_size(Files[i].name);


		Files[i].ent.offset = baseOffset;


		if (m_bFighterFix && firstFilePadFix)
		{
			padOffset = makePad(headerSize, SEC_PADSIZE) - headerSize;
			baseOffset = headerSize + padOffset - 12;
			firstFilePadFix = false;
		}


		if (m_bUsesFilenames)
			Files[i].ent.stringOffset = baseString;
		else
			Files[i].ent.stringOffset = NO_STRING_OFFSET;


		if (m_settings.m_epPlatform == PLATFORM_GC_WII)
		{
			changeEndINT(&Files[i].ent.offset);
			changeEndINT(&Files[i].ent.size);
			changeEndINT((int*)&Files[i].ent.type);
			changeEndINT(&Files[i].ent.stringOffset);
		}


		oFile.write((char*)&Files[i].ent, sizeof(section_file_entry));

		if (m_bUsesFilenames)
			baseOffset += std::filesystem::file_size(Files[i].name);
		else
			baseOffset += fsize;
		baseString += Files[i].name.length() + 1;
	}

	int padSize = makePad(sizeof(section_file_header), DEFAULT_SSF_PADSIZE) - sizeof(section_file_header) - (sizeof(section_file_entry) * Files.size());


	bool needsToSaveOriginalPad = false;

	if (m_bUsesFilenames)
	{
		int fighterFix = 0;
		Log(L"Saving names");
		for (unsigned int i = 0; i < Files.size(); i++)
		{
			std::wstring wstr = Files[i].name;
			std::string str(' ', Files[i].name.length());
			std::copy(wstr.begin(), wstr.end(), str.begin());

			oFile.write((char*)&str.c_str()[0], Files[i].name.length());
			char null = 0x00;
			oFile.write((char*)&null, sizeof(char));

		}

		if (m_bFighterFix)
			needsToSaveOriginalPad = true;

		padSize = makePad(headerSize, SEC_PADSIZE) - (headerSize);

		if (m_bFighterFix)
			padSize -= 12;

	}


	std::unique_ptr<char[]> pad = std::make_unique<char[]>(padSize);

	if (!m_bFighterFix)
		oFile.write(pad.get(), padSize);


	for (unsigned int i = 0; i < Files.size(); i++)
	{
		Log(L"Saving: " + Files[i].name);
		std::ifstream pInput(Files[i].name, std::ifstream::binary);
		int size = std::filesystem::file_size(Files[i].name);

		std::unique_ptr<char[]> dataBuff = std::make_unique<char[]>(size);
		pInput.read(dataBuff.get(), size);
		oFile.write(dataBuff.get(), size);

		if (needsToSaveOriginalPad)
		{
			oFile.write(pad.get(), padSize);
			needsToSaveOriginalPad = false;
		}

		if (!m_bUsesFilenames)
		{
			int pad_size = makePad(std::filesystem::file_size(Files[i].name), 2048);

			pad = std::make_unique<char[]>(pad_size - size);
			oFile.write(pad.get(), pad_size - size);
		}


	}

	MessageBox(0, L"Done!", L"Information", MB_ICONINFORMATION);

}

void SSFExplorer::Close()
{
	pFile.close();
	InputPath = L" ";
	OutputPath = L" ";
	SendMessage(*hFilesList, LB_RESETCONTENT, 0, 0);
	SetWindowText(*hLogBox, L"");
	SetWindowText(*hDataBox, L"");
	SetWindowText(GetDlgItem(eApp::hWindow, SSF_FILENAME), L"");
	SetWindowText(GetDlgItem(eApp::hWindow, SSF_BUILD_FILE), L"");

	m_bUsesFilenames = false;
	eApp::bIsReady = false;
	eApp::bIsIni = false;
}

void SSFExplorer::Log(std::wstring msg)
{
	PushLogMessage(*hLogBox, msg);
}

void SSFExplorer::AddData(std::wstring msg)
{
	PushLogMessage(*hDataBox, msg);
}



std::wstring SetPathFromButton(wchar_t* filter, wchar_t* ext, HWND hWnd)
{
	wchar_t szBuffer[MAX_PATH] = {};
	OPENFILENAME ofn = {};

	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hWnd;
	ofn.lpstrFilter = filter;
	ofn.lpstrFile = szBuffer;
	ofn.nMaxFile = MAX_PATH;
	ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
	ofn.lpstrDefExt = ext;
	std::wstring out;
	if (GetOpenFileName(&ofn))
		out = szBuffer;

	return out;
}

std::wstring SetSavePathFromButton(wchar_t* filter, wchar_t* ext, HWND hWnd)
{
	wchar_t szBuffer[MAX_PATH] = {};
	OPENFILENAME ofn = {};

	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hWnd;
	ofn.lpstrFilter = filter;
	ofn.lpstrFile = szBuffer;
	ofn.nMaxFile = MAX_PATH;
	ofn.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
	ofn.lpstrDefExt = ext;
	std::wstring out;
	if (GetSaveFileName(&ofn))
		out = szBuffer;

	return out;
}

std::wstring SetSavePathFromButtonWithName(std::wstring name, wchar_t * filter, wchar_t * ext, HWND hWnd)
{
	wchar_t szBuffer[MAX_PATH] = {};
	OPENFILENAME ofn = {};

	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hWnd;
	ofn.lpstrFilter = filter;
	wsprintf(szBuffer, name.c_str());
	ofn.lpstrFile = szBuffer;
	ofn.nMaxFile = MAX_PATH;
	ofn.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
	ofn.lpstrDefExt = ext;
	std::wstring out;
	if (GetSaveFileName(&ofn))
		out = szBuffer;

	return out;
}


std::wstring   SetFolderFromButton(HWND hWnd)
{
	wchar_t szBuffer[MAX_PATH];

	BROWSEINFO bi = {};
	bi.lpszTitle = L"Select Folder";
	bi.hwndOwner = hWnd;
	bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;

	LPITEMIDLIST idl = SHBrowseForFolder(&bi);

	std::wstring out;

	if (idl)
	{
		SHGetPathFromIDList(idl, szBuffer);
		out = szBuffer;

	}

	return out;
}

void PushLogMessage(HWND hWnd, std::wstring msg)
{
	msg += L"\r\n";
	int len = SendMessage(hWnd, WM_GETTEXTLENGTH, 0, 0);
	SendMessage(hWnd, EM_SETSEL, (WPARAM)len, (LPARAM)len);
	SendMessage(hWnd, EM_REPLACESEL, FALSE, (LPARAM)msg.c_str());
}
