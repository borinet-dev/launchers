#include "stdafx.h"

#include "Utils.h"


#include <windows.h>
#include <Shlwapi.h>
#include <process.h>
#include <Tlhelp32.h>
#include <winbase.h>
#include <string.h>
#include <Psapi.h>
#include <intshcut.h>
#pragma comment(lib, "Shlwapi.lib")
#pragma comment(lib, "Psapi.lib")



int GetFindCharCount(std::basic_string<TCHAR> msg, char find_char)
{
	int msg_len = msg.length();
	int find_cnt = 0;

	for(int i =0 ; i<msg_len ; i++)
	{
		if(msg[i] == find_char)
		{
			find_cnt++;
		}
	}
	return find_cnt;  
}

std::basic_string<TCHAR> getExePath()
{
	std::basic_string<TCHAR> cstrpath;
	TCHAR strpath[MAX_PATH];
	GetModuleFileName(NULL, strpath, MAX_PATH);
	cstrpath = strpath;
	return cstrpath;
}

std::basic_string<TCHAR> getExeDirPath()
{
	std::basic_string<TCHAR> cstrpath;
	TCHAR strpath[MAX_PATH];
	TCHAR *plastbs;
	GetModuleFileName(NULL, strpath, MAX_PATH);
	plastbs = _tcsrchr(strpath, _T('\\'));
	if(plastbs) *plastbs = 0;
	cstrpath = strpath;
	return cstrpath;
}

std::basic_string<TCHAR> getParentDirPath(TCHAR *str)
{
	TCHAR strpath[MAX_PATH];
	std::basic_string<TCHAR> cstrpath;
	TCHAR *plastbs;
	_tcscpy_s(strpath, MAX_PATH, str);
	plastbs = _tcsrchr(strpath, _T('\\'));
	if(plastbs)
	{
		*plastbs = 0;
		cstrpath = strpath;
	}else{
		cstrpath = std::basic_string<TCHAR>(_T(""));
	}
	return cstrpath;
}

BOOL DirectoryExists(LPCTSTR szPath)
{
	DWORD dwAttrib = GetFileAttributes(szPath);

	return (dwAttrib != INVALID_FILE_ATTRIBUTES && 
		(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}

std::basic_string<TCHAR> utf8ToString(char *str, int len)
{
	int iLen;
	std::basic_string<TCHAR> str2;
	wchar_t *wstrBuffer = (wchar_t*)malloc(sizeof(wchar_t)*(len + 1));

	iLen = MultiByteToWideChar(CP_UTF8, 0, (LPCSTR)str, len, NULL, NULL);
	iLen = MultiByteToWideChar(CP_UTF8, 0, (LPCSTR)str, len, wstrBuffer, iLen);
	wstrBuffer[iLen] = 0;

	str2 = wstrBuffer;

	free(wstrBuffer);

	return str2;
}

char *stringToUTF8(TCHAR *tstr, int len)
{
	int iLen;
	char *strBuffer = (char*)malloc(len*4 + 1);

	iLen = WideCharToMultiByte(CP_UTF8, 0, tstr, len, NULL, 0, NULL, NULL);
	iLen = WideCharToMultiByte(CP_UTF8, 0, tstr, len, strBuffer, iLen, NULL, NULL);
	strBuffer[iLen] = 0;

	return strBuffer;
}

void killProcessByName(TCHAR *filepath)
{
	HANDLE hSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPALL, NULL);
	PROCESSENTRY32 pEntry;
	BOOL hRes;

	TCHAR *strfilename;
	TCHAR strprocfilepath[MAX_PATH];

	strfilename = _tcsrchr(filepath, _T('\\'));
	if(strfilename == NULL)
		strfilename = filepath;
	else
		strfilename++;

	pEntry.dwSize = sizeof (pEntry);
	hRes = Process32First(hSnapShot, &pEntry);
	while (hRes)
	{
		if (_tcsicmp(strfilename, pEntry.szExeFile)==0)
		{
			HANDLE hProcess = OpenProcess(PROCESS_TERMINATE | PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, 0,
				(DWORD) pEntry.th32ProcessID);
			if (hProcess != NULL)
			{
				DWORD s = GetModuleFileNameEx(hProcess, NULL, strprocfilepath, MAX_PATH);
				DWORD dwErr = GetLastError();
				if(s > 0)
				{
					if(_tcsicmp(filepath, strprocfilepath)==0)
					{
						TerminateProcess(hProcess, 9);
					}
				}
				CloseHandle(hProcess);
			}
		}
		hRes = Process32Next(hSnapShot, &pEntry);
	}
	CloseHandle(hSnapShot);
} 

BOOL CreateShortcut(LPCTSTR szTargetPath,
					LPCTSTR szShortcutPath,
					LPCTSTR szDesc = NULL,
					LPCTSTR szArgument = NULL,
					LPCTSTR szIconPath = NULL,
					int iIconIndex = -1,
					WORD wHotkey = 0,
					int nShowCmd = SW_SHOW)
{
	// Validate parameters
	ASSERT(szTargetPath);
	ASSERT(szShortcutPath);

	HRESULT hRes;
	IUniformResourceLocator* purl = NULL;
	IShellLink* psl = NULL;
	IPersistFile* ppf = NULL;
	BOOL bRet = FALSE;

	TCHAR szDrive[_MAX_DRIVE], szDir[_MAX_PATH], szExt[_MAX_EXT];

	CString sExt = szExt;

	::CoInitialize(NULL);

	_tsplitpath_s(szShortcutPath, szDrive, _MAX_DRIVE, szDir, _MAX_PATH, NULL, 0, szExt, _MAX_EXT);

	sExt = szExt;

	// Shortcut
	if (0 == sExt.CollateNoCase(_T(".lnk")))
	{
		hRes = ::CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER,
			IID_IShellLink, (LPVOID*)&psl);
		hRes = psl->QueryInterface(IID_IPersistFile, (LPVOID*)&ppf);
	}

	// URL Shortcut
	else if (0 == sExt.CollateNoCase(_T(".url")))
	{
		hRes = ::CoCreateInstance(CLSID_InternetShortcut, NULL, CLSCTX_INPROC_SERVER,
			IID_IUniformResourceLocator, (LPVOID*)&purl);
		if (SUCCEEDED(hRes))
		{
			hRes = purl->QueryInterface(IID_IShellLink, (LPVOID*)&psl);
			if (SUCCEEDED(hRes))
				hRes = purl->QueryInterface(IID_IPersistFile, (LPVOID*) &ppf);
		}
	}
	else
	{
		// Shortcut file extention must be .lnk or .url
		::CoUninitialize();
		return FALSE;
	}

	if (SUCCEEDED(hRes))
	{
		hRes = psl->SetPath(szTargetPath);
		if (szDesc)
			psl->SetDescription(szDesc);

		if (szArgument)
			psl->SetArguments(szArgument);

		if (szIconPath && iIconIndex >= 0)
			psl->SetIconLocation(szIconPath, iIconIndex);

		if (wHotkey)
			psl->SetHotkey(wHotkey);

		CString sDir;
		sDir.Format(_T("%s%s"), szDrive, szDir);
		psl->SetWorkingDirectory(sDir);
		psl->SetShowCmd(nShowCmd);

		USES_CONVERSION;
		hRes = ppf->Save(T2COLE(szShortcutPath), TRUE);
		if (hRes != S_OK)
			TRACE(_T("IPersistFile::Save Error\n"));
		else
			bRet = TRUE;

		ppf->Release();
		psl->Release();
		if (purl)
			purl->Release();
	}
	::CoUninitialize();
	return bRet;
}

typedef UINT (CALLBACK* LP_SHGetKnownFolderPath) (REFKNOWNFOLDERID /*rfid*/, DWORD /*dwFlags*/, HANDLE /*hToken*/, __out PWSTR * /*ppszPath*/);

typedef HRESULT (CALLBACK* LP_SHGetFolderPathA) (
  _In_  HWND   hwndOwner,
  _In_  int    nFolder,
  _In_  HANDLE hToken,
  _In_  DWORD  dwFlags,
  _Out_ LPCSTR pszPath
);

BOOL GetFolderPath(int nFolder, REFKNOWNFOLDERID rfid, std::basic_string<TCHAR>& retPath)
{
	BOOL bRst = FALSE;
	PWSTR pszPath;
	CHAR cpath[MAX_PATH+1];
	HINSTANCE hShell = LoadLibrary(_T("Shell32.dll"));
	LP_SHGetKnownFolderPath pGetKnownPath;    // Function pointer
	LP_SHGetFolderPathA pGetFolderPath;
	
	do{
		pGetKnownPath = (LP_SHGetKnownFolderPath)GetProcAddress(hShell, "SHGetKnownFolderPath");
		if (pGetKnownPath)
		{
			if (SUCCEEDED(pGetKnownPath(rfid, 0, NULL, &pszPath)))
			{
				retPath = pszPath;
				CoTaskMemFree(pszPath);
				bRst = TRUE;
			}
			break;
		}
		pGetFolderPath = (LP_SHGetFolderPathA)GetProcAddress(hShell, "SHGetFolderPathA");
		if(pGetFolderPath)
		{
			if(pGetFolderPath(NULL, nFolder, NULL, 0, cpath) == S_OK)
			{
#ifdef _UNICODE
				retPath = CA2W(cpath);
#else
				retPath = cpath;
#endif
				bRst = TRUE;
			}
			break;
		}
	}while(0);

	FreeLibrary(hShell);

	return bRst;
}