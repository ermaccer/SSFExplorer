#include "IniReader.h"
#include <iostream>
#include <Windows.h>
using namespace std;
#pragma warning(disable:4996)
EXTERN_C IMAGE_DOS_HEADER __ImageBase;

CIniReader::CIniReader(const wchar_t* szFileName)
{
	wsprintf(m_szFileName,L"%s",szFileName);
}
int CIniReader::ReadInteger(wchar_t* szSection, wchar_t* szKey, int iDefaultValue)
{
 int iResult = GetPrivateProfileInt(szSection,  szKey, iDefaultValue, m_szFileName); 
 return iResult;
}
float CIniReader::ReadFloat(wchar_t* szSection, wchar_t* szKey, float fltDefaultValue)
{
 wchar_t szResult[255];
 wchar_t szDefault[255];
 float fltResult;
 wsprintf(szDefault, L"%f", fltDefaultValue);
 GetPrivateProfileString(szSection,  szKey, szDefault, szResult, 255, m_szFileName); 
 fltResult = (float)_wtof(szResult);
 return fltResult;
}
bool CIniReader::ReadBoolean(wchar_t* szSection, wchar_t* szKey, bool bolDefaultValue)
{
 wchar_t szResult[255];
 wchar_t szDefault[255];
 bool bolResult;
 wsprintf(szDefault, L"%s", bolDefaultValue ? L"True" : L"False");
 GetPrivateProfileString(szSection, szKey, szDefault, szResult, 255, m_szFileName); 
 bolResult =  (wcscmp(szResult, L"True") == 0 || 
		wcscmp(szResult, L"true") == 0) ? true : false;
 return bolResult;
}
wchar_t* CIniReader::ReadString(wchar_t* szSection, wchar_t* szKey, const wchar_t* szDefaultValue)
{
 wchar_t* szResult = new wchar_t[255];
 memset(szResult, 0x00, 255);
 GetPrivateProfileString(szSection,  szKey, 
		szDefaultValue, szResult, 255, m_szFileName); 
 return szResult;
}
void CIniReader::WriteInteger(wchar_t* szSection, wchar_t* szKey, int iValue)
{
	wchar_t szValue[255];
	wsprintf(szValue,L"%d", iValue);
	WritePrivateProfileString(szSection, szKey, szValue, m_szFileName);
}
void CIniReader::WriteFloat(wchar_t* szSection, wchar_t* szKey, float fltValue)
{
	wchar_t szValue[255];
	wsprintf(szValue, L"%f", fltValue);
	WritePrivateProfileString(szSection, szKey, szValue, m_szFileName);
}
void CIniReader::WriteBoolean(wchar_t* szSection, wchar_t* szKey, bool bolValue)
{
	wchar_t szValue[255];
	wsprintf(szValue, L"%s", bolValue ? L"True" : L"False");
	WritePrivateProfileString(szSection, szKey, szValue, m_szFileName);
}
void CIniReader::WriteString(wchar_t* szSection, wchar_t* szKey, const wchar_t* szValue)
{
	WritePrivateProfileString(szSection, szKey, szValue, m_szFileName);
}