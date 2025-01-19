
// JsAutoUpdater.cpp : 응용 프로그램에 대한 클래스 동작을 정의합니다.
//

#include "stdafx.h"
#include "JsAutoUpdater.h"

#include <iostream>
#include <wininet.h>

#include <list>
#include <string>

#include "Utils.h"

#include "UTF8Helper.h"

#include <openssl/sha.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#include <gdiplus.h>

#pragma comment(lib, "gdiplus.lib")

// CJsAutoUpdaterApp

BEGIN_MESSAGE_MAP(CJsAutoUpdaterApp, CWinAppEx)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()

extern "C" {
	FILE __iob_func[3] = { stdin, stdout, stderr };
}

// CJsAutoUpdaterApp 생성

static TCHAR *_tcstrim(TCHAR *str)
{
	TCHAR *lastptr = &str[_tcslen(str) - 1];
	while (*str && (*str == _T(' ') || *str == _T('\r') || *str == _T('\n') || *str == _T('\t')))
	{
		str++;
	}
	if (str <= lastptr)
	{
		while (*lastptr && (*lastptr == _T(' ') || *lastptr == _T('\r') || *lastptr == _T('\n') || *lastptr == _T('\t')))
		{
			*lastptr-- = 0;
		}
	}
	return str;
}

static std::list< std::basic_string<TCHAR> > strtokener(const std::basic_string<TCHAR>& text, const std::basic_string<TCHAR> &spliter, int maxTokens = -1)
{
	std::list< std::basic_string<TCHAR> > list;
	size_t pos = 0;
	size_t next;
	do {
		std::basic_string<TCHAR> token;
		next = text.find(spliter, pos);
		if (next != std::string::npos) {
			if (maxTokens > 0 && (list.size() + 1) >= maxTokens)
			{
				token = text.substr(pos);
				break;
			} else {
				token = text.substr(pos, next - pos);
			}
			pos = next + 1;
			list.push_back(token);
		}
	} while (next != std::string::npos);
	if (pos < text.length())
		list.push_back(text.substr(pos));
	return list;
}

template<typename T>
std::basic_string<T> strReplaceAll(const std::basic_string<T> &str, const std::basic_string<T>& from, const std::basic_string<T>& to) {
	size_t start_pos = 0; //string처음부터 검사
	std::basic_string<T> buffer = str;
	while ((start_pos = buffer.find(from, start_pos)) != std::string::npos)  //from을 찾을 수 없을 때까지
	{
		buffer.replace(start_pos, from.length(), to);
		start_pos += to.length(); // 중복검사를 피하고 from.length() > to.length()인 경우를 위해서
	}
	return buffer;
}

CJsAutoUpdaterApp::CJsAutoUpdaterApp()
{
	// TODO: 여기에 생성 코드를 추가합니다.
	// InitInstance에 모든 중요한 초기화 작업을 배치합니다.
	this->m_dlg_updater = NULL;
}


// 유일한 CJsAutoUpdaterApp 개체입니다.

CJsAutoUpdaterApp theApp;

ULONG_PTR gdiplusToken;

static int64_t MyGetLastTimeFromFileinfo(WIN32_FIND_DATA *pfiledata)
{
	int64_t a;
	int64_t b;

	a = (((int64_t)(pfiledata->ftCreationTime.dwHighDateTime)) << 32) | ((int64_t)(pfiledata->ftCreationTime.dwLowDateTime));
	b = (((int64_t)(pfiledata->ftLastWriteTime.dwHighDateTime)) << 32) | ((int64_t)(pfiledata->ftLastWriteTime.dwLowDateTime));

	return (a > b) ? a : b;
}

static bool parseHash(unsigned char *buf, const TCHAR* text) {
	int i;
	for (i = 0; i < 20; i++)
	{
		int j;
		unsigned char d = 0;
		for (j = 0; (j < 2) && (*text); j++, text++)
		{
			char c = (char)*text;
			d <<= 4;
			if ((c >= '0') && (c <= '9'))
				d |= (c - '0');
			else if ((c >= 'a') && (c <= 'f'))
				d |= (c - 'a') + 10;
			else if ((c >= 'A') && (c <= 'F'))
				d |= (c - 'A') + 10;
			else
				return false;
		}
		if (j != 2)
			return false;
		buf[i] = d;
	}
	if (i != 20)
		return false;
	return true;
}

// CJsAutoUpdaterApp 초기화

BOOL CJsAutoUpdaterApp::InitInstance()
{
	int i;

	BOOL bIs64bits = FALSE;

	BOOL           selfupdate_flag = FALSE;
	UpdateFileInfo selfupdate_fi;

	bool needupdate_flag = false;

	BOOL ini_bDontshow = FALSE;
	long long ini_DontshowTime = 0;

	std::basic_string<TCHAR> strdirpath = getExeDirPath();

	std::basic_string<TCHAR> strfolder_appdata;
	std::basic_string<TCHAR> strfolder_desktop;
	std::basic_string<TCHAR> strfolder_startmenuprograms;
	std::basic_string<TCHAR> strfolder_common_appdata;
	std::basic_string<TCHAR> strfolder_common_desktop;
	std::basic_string<TCHAR> strfolder_common_startmenuprograms;
	std::basic_string<TCHAR> strfolder_programfiles;
	std::basic_string<TCHAR> strfolder_programfiles_x86;
	std::basic_string<TCHAR> strfolder_programfiles_x64;
	std::basic_string<TCHAR> strfolder_windows;
	std::basic_string<TCHAR> strfolder_temp;

	int progress_min = 0;
	int progress_max;

	CString tmpstr;
	BOOL bRst;
	TCHAR szTempDir[MAX_PATH] = {0};

	bRst = GetFolderPath(CSIDL_COMMON_APPDATA, FOLDERID_ProgramData, strfolder_common_appdata);
	bRst = GetFolderPath(CSIDL_APPDATA, FOLDERID_RoamingAppData, strfolder_appdata);

	bRst = GetFolderPath(CSIDL_COMMON_DESKTOPDIRECTORY, FOLDERID_PublicDesktop, strfolder_common_desktop);
	bRst = GetFolderPath(CSIDL_DESKTOPDIRECTORY, FOLDERID_Desktop, strfolder_desktop);

	bRst = GetFolderPath(CSIDL_COMMON_PROGRAMS, FOLDERID_CommonPrograms, strfolder_common_startmenuprograms);
	bRst = GetFolderPath(CSIDL_PROGRAMS, FOLDERID_Programs, strfolder_startmenuprograms);

	bRst = GetFolderPath(CSIDL_PROGRAM_FILES, FOLDERID_ProgramFiles, strfolder_programfiles);
	bRst = GetFolderPath(CSIDL_PROGRAM_FILESX86, FOLDERID_ProgramFilesX86, strfolder_programfiles_x86);
	bRst = GetFolderPath(CSIDL_PROGRAM_FILES, FOLDERID_ProgramFilesX64, strfolder_programfiles_x64);
	bRst = GetFolderPath(CSIDL_WINDOWS, FOLDERID_Windows, strfolder_windows);

	{
		size_t templen;
		::GetTempPath(MAX_PATH, szTempDir);
		templen = _tcslen(szTempDir);
		if (templen > 0 && szTempDir[templen - 1] == _T('\\'))
			szTempDir[templen - 1] = 0;
		strfolder_temp = szTempDir;
	}


	// 응용 프로그램 매니페스트가 ComCtl32.dll 버전 6 이상을 사용하여 비주얼 스타일을
	// 사용하도록 지정하는 경우, Windows XP 상에서 반드시 InitCommonControlsEx()가 필요합니다.
	// InitCommonControlsEx()를 사용하지 않으면 창을 만들 수 없습니다.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// 응용 프로그램에서 사용할 모든 공용 컨트롤 클래스를 포함하도록
	// 이 항목을 설정하십시오.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinAppEx::InitInstance();

	AfxEnableControlContainer();

	// 표준 초기화
	// 이들 기능을 사용하지 않고 최종 실행 파일의 크기를 줄이려면
	// 아래에서 필요 없는 특정 초기화
	// 루틴을 제거해야 합니다.
	// 해당 설정이 저장된 레지스트리 키를 변경하십시오.
	// TODO: 이 문자열을 회사 또는 조직의 이름과 같은
	// 적절한 내용으로 수정해야 합니다.
	SetRegistryKey(_T("JsAutoUpdater"));

#if defined(WIN64) || defined(_WIN64)
	bIs64bits = TRUE;
#else
	IsWow64Process(GetCurrentProcess(), &bIs64bits);
#endif

	::Gdiplus::GdiplusStartupInput gdiplusStartupInfo;
	GdiplusStartup(&gdiplusToken, &gdiplusStartupInfo, 0);

	this->m_Updater_FileDBPath = strfolder_appdata + _T("\\Microsoft\\Windows\\borinet\\L2DB.txt");

	HttpClient::globalInit();
	
	for (i = 1; i<__argc; i++)
	{
		if (__targv[i][0] == _T('/'))
		{
			if (_tcsicmp(__targv[i], _T("/NEW")) == 0)
			{
				m_execflag_new = 1;
			}
			else if (_tcsicmp(__targv[i], _T("/OLD")) == 0)
			{
				i++;
				if (i >= __argc)
				{
					MessageBox(NULL, _T("인수가 잘못되었습니다."), this->m_Updater_Name.c_str(), 0);
					return FALSE;
				}
				DeleteFile(__targv[i]);
			} else {
				MessageBox(NULL, _T("인수가 잘못되었습니다."), this->m_Updater_Name.c_str(), 0);
				return FALSE;
			}
		} else {
			m_execflag_cmd = __targv[i];
			i++;
			for (; i<__argc; i++)
			{
				m_execflag_cmd = m_execflag_cmd + _T(" \"") + __targv[i] + _T("\"");
			}
		}
	}

	UpdaterSettings();

	{
		LRESULT lres;
		HKEY hKey = NULL;
		TCHAR szFileName[MAX_PATH * 2];
		TCHAR *pText;
		int i;
		::GetModuleFileName(::GetModuleHandle(NULL), szFileName, MAX_PATH * 2);
		pText = _tcsrchr(szFileName, _T('\\'));
		if (pText)
			*pText = 0;
		for (i = 0; i < 2; i++)
		{
			hKey = NULL;
			switch (i)
			{
			case 0:
				// Native
				lres = ::RegOpenKeyEx(this->m_Updater_CheckRegKey, this->m_Updater_CheckRegPath.c_str(), 0, KEY_READ, &hKey);
				break;
			case 1:
				// Wow
#if defined(WIN64) || defined(_WIN64)
				lres = ::RegOpenKeyEx(this->m_Updater_CheckRegKey, this->m_Updater_CheckRegPath.c_str(), 0, KEY_QUERY_VALUE | KEY_WOW64_32KEY, &hKey);
#else
				if(bIs64bits)
					lres = ::RegOpenKeyEx(this->m_Updater_CheckRegKey, this->m_Updater_CheckRegPath.c_str(), 0, KEY_READ | KEY_WOW64_64KEY, &hKey);
#endif
				break;
			}
			if (hKey)
			{
				TCHAR szData[MAX_PATH * 2];
				DWORD cbData = MAX_PATH * 2;
				DWORD dwType = REG_SZ;
				lres = ::RegQueryValueEx(hKey, this->m_Updater_CheckRegName.c_str(), NULL, &dwType, (LPBYTE)szData, &cbData);
				if (lres == ERROR_SUCCESS)
				{
					size_t tmpLen = _tcslen(szData);
					if (tmpLen > 0)
					{
						if (szData[tmpLen - 1] == _T('\\'))
							szData[tmpLen - 1] = 0;
					}
				}
				::RegCloseKey(hKey);
			}
		}
	}

	{
		std::basic_string<TCHAR>::size_type temppos = m_Updater_FileDBPath.find_last_of(_T('\\'));
		if (temppos != std::basic_string<TCHAR>::npos) {
			TCHAR szPathBuffer[MAX_PATH];
			_tcsncpy_s(szPathBuffer, m_Updater_FileDBPath.c_str(), temppos);
			if (::GetFileAttributes(szPathBuffer) == INVALID_FILE_ATTRIBUTES) {
				::CreateDirectory(szPathBuffer, NULL);
			}
		}
	}

	if(this->m_PermitExecWithArg)
	{
		if(m_execflag_cmd.length() > 0)
			this->m_ExecAfterUpdate = m_execflag_cmd;
	}

	m_http_client.setBaseUrl(UTF8Helper::tstr_to_utf8(this->m_ServerURL));
	if (!m_ServerBasicUsername.empty())
	{
		m_http_client.setBasicAuthorization(UTF8Helper::tstr_to_utf8(m_ServerBasicUsername), UTF8Helper::tstr_to_utf8(m_ServerBasicPassword));
	}

	//appendTextFromFtp(m_FTPClient_hConn, this->m_ServerFTP_Charset_UTF8, this->m_ServerFTP_DefUrl + _T("/update_msg.txt"), this->m_ServerData_UpdateMsg, this->m_HTTP_InfoFile_flag);

	/*
	{
		std::basic_string<TCHAR> readBuffer;
		appendTextFromFtp(m_FTPClient_hConn, this->m_ServerFTP_Charset_UTF8, m_ServerFTP_DefUrl + _T("/logolinks.txt"), readBuffer, this->m_HTTP_InfoFile_flag);
		std::list<std::basic_string<TCHAR> > lines = strtokener(readBuffer, _T("\n"));
		for (std::list<std::basic_string<TCHAR> >::iterator iter = lines.begin(); iter != lines.end(); iter++)
		{
			TCHAR *text = _tcstrim((TCHAR*)iter->data());
			if (_tcslen(text) > 0)
				m_serverData_logoLinks.push_back(text);
		}
	}
	*/

#if 0
	// 로고 가져오기
	for(i=0; i<5; i++)
	{
		TCHAR szFileName[MAX_PATH];
		HINTERNET    hFile = NULL;
		DWORD        dwReadSize = 0;

		char *readBuffer = NULL;
		DWORD readBufferSize = 0;

		wchar_t *wstrBuffer;

		int iLen;

		_stprintf_s(szFileName, _T("%s/logos/image_%d.png"), m_ServerFTP_DefUrl.c_str(), i + 1);

		do {
			DWORD dwSize;
			uint64_t totalSize = 0;

			std::vector<unsigned char> buffer;
			buffer.resize(1048576);
			
			//if(bUTF8)
			//	hFile = FtpOpenFileA(hConn, CT2A(strUrl, CP_UTF8), GENERIC_READ , FTP_TRANSFER_TYPE_BINARY | INTERNET_FLAG_TRANSFER_BINARY, NULL);
			//else		
			hFile = FtpOpenFile(m_FTPClient_hConn, szFileName, GENERIC_READ, FTP_TRANSFER_TYPE_BINARY | INTERNET_FLAG_TRANSFER_BINARY, NULL);

			if (hFile == NULL) break;

			do {
				dwSize = 0;
				if (!InternetReadFile(hFile, buffer.data() + totalSize, buffer.size() - totalSize, &dwSize))
					break;
				totalSize += dwSize;
				if ((buffer.size() - totalSize) < 1048576)
				{
					buffer.resize(buffer.size() + 1048576);
				}
			} while (dwSize > 0);

			buffer.resize(totalSize);
			m_serverData_logoPngs.push_back(buffer);
		} while (0);

		if (hFile) InternetCloseHandle(hFile);
	}
#endif

	{
		m_ExecAfterUpdate = strReplaceAll<TCHAR>(m_ExecAfterUpdate, _T("<APPPATH>"), strdirpath);
		m_ExecAfterUpdate = strReplaceAll<TCHAR>(m_ExecAfterUpdate, _T("<COMMON_APPDATA>"), strfolder_common_appdata);
		m_ExecAfterUpdate = strReplaceAll<TCHAR>(m_ExecAfterUpdate, _T("<APPDATA>"), strfolder_appdata);
		m_ExecAfterUpdate = strReplaceAll<TCHAR>(m_ExecAfterUpdate, _T("<COMMON_DESKTOP>"), strfolder_common_desktop);
		m_ExecAfterUpdate = strReplaceAll<TCHAR>(m_ExecAfterUpdate, _T("<DESKTOP>"), strfolder_desktop);
		m_ExecAfterUpdate = strReplaceAll<TCHAR>(m_ExecAfterUpdate, _T("<COMMON_PROGRAMS>"), strfolder_common_startmenuprograms);
		m_ExecAfterUpdate = strReplaceAll<TCHAR>(m_ExecAfterUpdate, _T("<PROGRAMS>"), strfolder_startmenuprograms);
		m_ExecAfterUpdate = strReplaceAll<TCHAR>(m_ExecAfterUpdate, _T("<PROGRAMFILES>"), strfolder_programfiles);
		m_ExecAfterUpdate = strReplaceAll<TCHAR>(m_ExecAfterUpdate, _T("<PROGRAMFILES_X86>"), strfolder_programfiles_x86);
		m_ExecAfterUpdate = strReplaceAll<TCHAR>(m_ExecAfterUpdate, _T("<PROGRAMFILES_X64>"), strfolder_programfiles_x64);
		m_ExecAfterUpdate = strReplaceAll<TCHAR>(m_ExecAfterUpdate, _T("<WINDOWS>"), strfolder_windows);
		m_ExecAfterUpdate = strReplaceAll<TCHAR>(m_ExecAfterUpdate, _T("<TEMP>"), strfolder_temp);
	}

	{
		size_t tmp;
		size_t readPos = 0;
		TCHAR szPathBuffer[MAX_PATH * 2];

		{
			std::unique_ptr<HttpClient::SimpleResponse> response = m_http_client.simpleRequest("/L2class.php");
			m_serverData_rawFilesInfo.clear();
			m_serverData_rawFilesInfo.insert(m_serverData_rawFilesInfo.end(), response->response_body.begin(), response->response_body.end());
			tmp = this->m_serverData_rawFilesInfo.find(_T("*"), 0);
			if (tmp == std::string::npos)
			{
				MessageBox(NULL, _T("업데이트 서버에 접속할 수 없습니다.\n홈페이지 공지사항을 확인하세요!"), this->m_Updater_Name.c_str(), 0);
				ProgramExit();
				return FALSE;
			}
			this->m_serverData_rawFilesInfo = this->m_serverData_rawFilesInfo.substr(tmp + 1);
			}

		this->m_serverData_filesInfo.clear();

		{
			std::list< std::basic_string<TCHAR> > lines = strtokener(m_serverData_rawFilesInfo, _T("\n"));
			for (std::list< std::basic_string<TCHAR> >::iterator iterLine = lines.begin(); iterLine != lines.end(); iterLine++) {
				std::basic_string<TCHAR> line = _tcstrim((TCHAR*)iterLine->data());
				if ((line.length() == 0) || (line.length() > 0 && line.at(0) == _T(';')))
					continue;

				std::list< std::basic_string<TCHAR> > tokens = strtokener(line, _T("|"), 5);
				std::list< std::basic_string<TCHAR> >::iterator tokenIter = tokens.begin();
				if (tokens.size() < 4)
				{
					// Error
					continue;
				}

				UpdateFileInfo fileinfo;

				const TCHAR *flagptr = tokenIter->c_str();
				while (*flagptr)
				{
					if (_tcsnicmp(flagptr, _T("SU"), 2) == 0)
					{
						// Self update
						fileinfo.type |= JSAUTOUPDATER_FI_TYPE_UPDATER;
						flagptr += 2;
					}
					else if (_tcsnicmp(flagptr, _T("O32"), 3) == 0)
					{
						fileinfo.type |= JSAUTOUPDATER_FI_TYPE_ONLY32BITS;
						flagptr += 2;
					}
					else if (_tcsnicmp(flagptr, _T("O64"), 3) == 0)
					{
						fileinfo.type |= JSAUTOUPDATER_FI_TYPE_ONLY64BITS;
						flagptr += 3;
					}
					else if (_tcsnicmp(flagptr, _T("K"), 1) == 0)
					{
						fileinfo.type |= JSAUTOUPDATER_FI_TYPE_KILLPROCESS;
						flagptr += 1;
					}
					else if (_tcsnicmp(flagptr, _T("U"), 1) == 0)
					{
						fileinfo.type |= JSAUTOUPDATER_FI_TYPE_UPDATEFILE;
						flagptr += 1;
					}
					else if (_tcsnicmp(flagptr, _T("R"), 1) == 0)
					{
						fileinfo.type |= JSAUTOUPDATER_FI_TYPE_REMOVEFILE;
						flagptr += 1;
					}
					else if (_tcsnicmp(flagptr, _T("EW"), 2) == 0)
					{
						fileinfo.type |= JSAUTOUPDATER_FI_TYPE_EXECNWAIT;
						flagptr += 2;
					}
					else if (_tcsnicmp(flagptr, _T("EAU"), 3) == 0)
					{
						fileinfo.type |= JSAUTOUPDATER_FI_TYPE_EXECALWAYS;
						flagptr += 3;
					}
					else if (_tcsnicmp(flagptr, _T("E"), 1) == 0)
					{
						fileinfo.type |= JSAUTOUPDATER_FI_TYPE_EXEC;
						flagptr += 1;
					}
					else if (_tcsnicmp(flagptr, _T("ML"), 2) == 0)
					{
						fileinfo.type |= JSAUTOUPDATER_FI_TYPE_MAKELINK;
						flagptr += 2;
					}
					else {
						// Error
						break;
					}
				}

				if (fileinfo.type & JSAUTOUPDATER_FI_TYPE_ONLY32BITS)
				{
					if (bIs64bits) continue;
				} else if (fileinfo.type & JSAUTOUPDATER_FI_TYPE_ONLY64BITS)
				{
					if (!bIs64bits) continue;
				}

				tokenIter++; // Version
				if ((*tokenIter) == _T("?"))
					fileinfo.version = -1;
				else
					fileinfo.version = _ttoi(tokenIter->c_str());
				tokenIter++; // serverFile
				fileinfo.serverfile = *tokenIter;
				tokenIter++;
				fileinfo.clientfile = strReplaceAll<TCHAR>(*tokenIter, _T("/"), _T("\\"));
				tokenIter++;

				fileinfo.clientfile = strReplaceAll<TCHAR>(fileinfo.clientfile, _T("<APPPATH>"), strdirpath);
				fileinfo.clientfile = strReplaceAll<TCHAR>(fileinfo.clientfile, _T("<COMMON_APPDATA>"), strfolder_common_appdata);
				fileinfo.clientfile = strReplaceAll<TCHAR>(fileinfo.clientfile, _T("<APPDATA>"), strfolder_appdata);
				fileinfo.clientfile = strReplaceAll<TCHAR>(fileinfo.clientfile, _T("<COMMON_DESKTOP>"), strfolder_common_desktop);
				fileinfo.clientfile = strReplaceAll<TCHAR>(fileinfo.clientfile, _T("<DESKTOP>"), strfolder_desktop);
				fileinfo.clientfile = strReplaceAll<TCHAR>(fileinfo.clientfile, _T("<COMMON_PROGRAMS>"), strfolder_common_startmenuprograms);
				fileinfo.clientfile = strReplaceAll<TCHAR>(fileinfo.clientfile, _T("<PROGRAMS>"), strfolder_startmenuprograms);
				fileinfo.clientfile = strReplaceAll<TCHAR>(fileinfo.clientfile, _T("<PROGRAMFILES>"), strfolder_programfiles);
				fileinfo.clientfile = strReplaceAll<TCHAR>(fileinfo.clientfile, _T("<PROGRAMFILES_X86>"), strfolder_programfiles_x86);
				fileinfo.clientfile = strReplaceAll<TCHAR>(fileinfo.clientfile, _T("<PROGRAMFILES_X64>"), strfolder_programfiles_x64);
				fileinfo.clientfile = strReplaceAll<TCHAR>(fileinfo.clientfile, _T("<WINDOWS>"), strfolder_windows);
				fileinfo.clientfile = strReplaceAll<TCHAR>(fileinfo.clientfile, _T("<TEMP>"), strfolder_temp);
				if (PathIsRelative(fileinfo.clientfile.c_str()))
				{
					fileinfo.clientfile = strdirpath + _T("\\") + fileinfo.clientfile;
				}

				if (fileinfo.type & JSAUTOUPDATER_FI_TYPE_MAKELINK)
				{
					fileinfo.serverfile = strReplaceAll<TCHAR>(fileinfo.serverfile, _T("/"), _T("\\"));
					fileinfo.serverfile = strReplaceAll<TCHAR>(fileinfo.serverfile, _T("<APPPATH>"), strdirpath);
					fileinfo.serverfile = strReplaceAll<TCHAR>(fileinfo.serverfile, _T("<COMMON_APPDATA>"), strfolder_common_appdata);
					fileinfo.serverfile = strReplaceAll<TCHAR>(fileinfo.serverfile, _T("<APPDATA>"), strfolder_appdata);
					fileinfo.serverfile = strReplaceAll<TCHAR>(fileinfo.serverfile, _T("<COMMON_DESKTOP>"), strfolder_common_desktop);
					fileinfo.serverfile = strReplaceAll<TCHAR>(fileinfo.serverfile, _T("<DESKTOP>"), strfolder_desktop);
					fileinfo.serverfile = strReplaceAll<TCHAR>(fileinfo.serverfile, _T("<COMMON_PROGRAMS>"), strfolder_common_startmenuprograms);
					fileinfo.serverfile = strReplaceAll<TCHAR>(fileinfo.serverfile, _T("<PROGRAMS>"), strfolder_startmenuprograms);
					fileinfo.serverfile = strReplaceAll<TCHAR>(fileinfo.serverfile, _T("<PROGRAMFILES>"), strfolder_programfiles);
					fileinfo.serverfile = strReplaceAll<TCHAR>(fileinfo.serverfile, _T("<PROGRAMFILES_X86>"), strfolder_programfiles_x86);
					fileinfo.serverfile = strReplaceAll<TCHAR>(fileinfo.serverfile, _T("<PROGRAMFILES_X64>"), strfolder_programfiles_x64);
					fileinfo.serverfile = strReplaceAll<TCHAR>(fileinfo.serverfile, _T("<WINDOWS>"), strfolder_windows);
					fileinfo.serverfile = strReplaceAll<TCHAR>(fileinfo.serverfile, _T("<TEMP>"), strfolder_temp);
					if (PathIsRelative(fileinfo.serverfile.c_str()))
					{
						fileinfo.serverfile = strdirpath + _T("\\") + fileinfo.serverfile;
					}
				} else {
					fileinfo.serverfile = strReplaceAll<TCHAR>(fileinfo.serverfile, _T("\\"), _T("/"));
				}

				if (tokenIter != tokens.end()) {
					fileinfo.serverFileSize = _ttoll(tokenIter->c_str());
				}

				if (fileinfo.type == JSAUTOUPDATER_FI_TYPE_UPDATER)
				{
					if (m_UpdaterVersion >= fileinfo.version) continue;
					selfupdate_flag = TRUE;
					selfupdate_fi = fileinfo;
				}

				m_serverData_filesInfo.push_back(fileinfo);
			}
		}

		szPathBuffer[0] = 0;
		//findServerFiles(szPathBuffer, MAX_PATH * 2);
	}

	if (selfupdate_flag && (!m_execflag_new))
	{
		int nRst;

		std::basic_string<TCHAR> strselfexepath;
		std::basic_string<TCHAR> strmovepath;
		std::basic_string<TCHAR> strdirpath;

		std::basic_string<TCHAR> strnewpath;

		CString strexecmd;

		strselfexepath = getExePath();
		strdirpath = getExeDirPath();
		strmovepath = strselfexepath + _T("~");
		bRst = MoveFileEx(strselfexepath.c_str(), strmovepath.c_str(), MOVEFILE_REPLACE_EXISTING);
		if (!bRst)
		{
			MessageBox(NULL, makeErrorString(_T("자가업데이트에 실패하였습니다.\n자동 업데이터를 종료합니다.\n(업데이터 치환 실패)")).c_str(), m_Program_Name.c_str(), 0);
			return FALSE;
		}

		strnewpath = selfupdate_fi.clientfile;
		if (PathIsRelative(strnewpath.c_str()))
		{
			strnewpath = strdirpath + _T("\\") + strnewpath;
		}

		{
			std::unique_ptr<HttpClient::SimpleResponse> response = m_http_client.simpleDownload(UTF8Helper::tstr_to_utf8(_T("/L2-Class/") + selfupdate_fi.serverfile), strnewpath.c_str());
			if (!response->success)
			{
				CString strerr;
				strerr.Format(_T("자가업데이트에 실패하였습니다.\n자동 업데이터를 종료합니다.\n(파일 다운 실패 : 오류번호  %d)"), nRst);
				MessageBox(NULL, strerr, m_Program_Name.c_str(), 0);
				return FALSE;
			}
		}

		strexecmd.Format(_T("/NEW /OLD \"%s\" %s"), strmovepath.c_str(), this->m_execflag_cmd.c_str());

		ShellExecute(NULL, _T("open"), strnewpath.c_str(), strexecmd, NULL, SW_SHOW);

		return FALSE;
	}

	std::vector<unsigned char> filebuf;
	filebuf.resize(1048576 * 4);

	do {
		FILE *fp;
		errno_t eno;

		TCHAR linebuf[512];

		eno = _tfopen_s(&fp, this->m_Updater_FileDBPath.c_str(), _T("rt"));
		if (eno != 0) break;
		while (_fgetts(linebuf, sizeof(linebuf) / sizeof(TCHAR), fp))
		{
			std::basic_string<TCHAR> tmpcstr = _tcstrim(linebuf);
			TCHAR *pContext = NULL;
			TCHAR *pVersion = NULL;
			TCHAR *pServerFilePath = NULL;
			TCHAR *pLocalFilePath = NULL;
			TCHAR* pFileHash = NULL;

			int64_t localVersion = 0;

			HANDLE hFile;
			LARGE_INTEGER liFileSize = { 0 };

			unsigned char srv_hash[20] = { 0 };
			unsigned char cli_hash[20] = { 0 };

			if (tmpcstr.length() <= 1) continue;

			pVersion = _tcstok_s(linebuf, _T("|"), &pContext);
			pServerFilePath = _tcstok_s(NULL, _T("|"), &pContext);
			pLocalFilePath = _tcstok_s(NULL, _T("|"), &pContext);
			pFileHash = _tcstok_s(NULL, _T("|"), &pContext);
			localVersion = _ttoi64(pVersion);

			if (pFileHash) {
				parseHash(srv_hash, pFileHash);
			}

			for (std::list<UpdateFileInfo>::iterator iter = m_serverData_filesInfo.begin(); iter != m_serverData_filesInfo.end(); iter++)
			{
				if (iter->serverfile == pServerFilePath)
				{
					hFile = CreateFile(iter->clientfile.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
					if (hFile && hFile != INVALID_HANDLE_VALUE)
					{
						BOOL test = ::GetFileSizeEx(hFile, &liFileSize);
						SHA_CTX hash_ctx;
						DWORD dwReadBytes;

						SHA1_Init(&hash_ctx);
						while (ReadFile(hFile, &filebuf[0], filebuf.size(), &dwReadBytes, NULL) && dwReadBytes)
						{
							SHA1_Update(&hash_ctx, filebuf.data(), dwReadBytes);
						}
						SHA1_Final(cli_hash, &hash_ctx);
						
						::CloseHandle(hFile);
					}

					if (memcmp(cli_hash, srv_hash, 20))
					{
						iter->update_flag = 1;
						needupdate_flag = true;
						break;
					}

					memcpy(iter->filehash, srv_hash, 20);

					iter->localFileSize = liFileSize.QuadPart;
					if (iter->version < 0)
					{
						if (iter->serverTimeVer > localVersion || iter->serverFileSize != iter->localFileSize)
						{
							iter->update_flag = 1;
							needupdate_flag = true;
						}
						else {
							iter->update_flag = 0;
						}
					}
					else if (iter->version > localVersion || iter->serverFileSize != iter->localFileSize) {
						iter->update_flag = 1;
						needupdate_flag = true;
					}
					else {
						iter->update_flag = 0;
					}
					break;
				}
			}
		}
		fclose(fp);
	} while (0);

	if(!needupdate_flag)
	{
		for (std::list<UpdateFileInfo>::iterator iter = m_serverData_filesInfo.begin(); iter != m_serverData_filesInfo.end(); iter++)
		{
			if (iter->type & JSAUTOUPDATER_FI_TYPE_UPDATEFILE)
			{
				if (iter->update_flag != 0)
				{
					needupdate_flag = true;
					break;
				}
			}
		}

	}

#if 0
	if(needupdate_flag)
	{
		INT_PTR nResponse;

		this->m_dlg_updater = new CUpdaterDlg();
		//m_pMainWnd = this->m_dlg_updater;
		nResponse = this->m_dlg_updater->DoModal();
	}else{
		//MessageBox(NULL, _T("업데이트가 필요하지 않습니다."), this->m_Updater_Name, 0);
	}

	//if(m_dlg_updater) delete m_dlg_updater;
#else
	{
		INT_PTR nResponse;

		this->m_dlg_updater = new CUpdaterDlg();
		this->m_dlg_updater->setNeedUpdate(needupdate_flag);
		//m_pMainWnd = this->m_dlg_updater;
		nResponse = this->m_dlg_updater->DoModal(); 
	}
#endif
	ProgramExit();

	// 대화 상자가 닫혔으므로 응용 프로그램의 메시지 펌프를 시작하지 않고  응용 프로그램을 끝낼 수 있도록 FALSE를
	// 반환합니다.
	return FALSE;
}

BOOL CJsAutoUpdaterApp::PreTranslateMessage(MSG* pMsg)
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.

	return CWinAppEx::PreTranslateMessage(pMsg);
}

#if 0
BOOL CJsAutoUpdaterApp::findServerFiles(TCHAR *lpPathBuffer, size_t cbPathBuffer)
{
	TCHAR szDirPath[MAX_PATH * 2];
	HINTERNET hFind;
	BOOL bFind;
	BOOL bRst;
	DWORD dwErr;
	WIN32_FIND_DATA filedata = {0};
	size_t dirLength = _tcslen(lpPathBuffer);

	int retval = 0;

	std::list<WIN32_FIND_DATA> dirlist;
	std::list<WIN32_FIND_DATA>::iterator itor;

	_tcscpy_s(szDirPath, this->m_ServerFTP_DefUrl_files.c_str());
	_tcscat_s(szDirPath, _T("/"));
	_tcscat_s(szDirPath, lpPathBuffer);

	/*if(this->m_ServerFTP_Charset_UTF8)
		bRst = FtpSetCurrentDirectory(this->m_FTPClient_hConn, strdir, CP_UTF8));
	else
		*/
	bRst = FtpSetCurrentDirectory(this->m_FTPClient_hConn, szDirPath);

	if(!bRst)
	{
		retval = -1;
		return retval;
	}

	bFind = TRUE;
	hFind = FtpFindFirstFile(this->m_FTPClient_hConn, NULL, &filedata, 0, NULL);
	dwErr = GetLastError();
	while(hFind && bFind)
	{
		if(!((_tcscmp(filedata.cFileName, _T(".")) == 0) || (_tcscmp(filedata.cFileName, _T("..")) == 0)))
			dirlist.push_back(filedata);
		bFind = InternetFindNextFile(hFind, &filedata);
		dwErr = GetLastError();
	}
	
	InternetCloseHandle(hFind);

	if(dirlist.size() <= 0)
	{
		return FALSE;
	}

	for(itor = dirlist.begin(); itor != dirlist.end(); itor++)
	{
		lpPathBuffer[dirLength] = 0;
		if (dirLength > 0)
			_tcscat_s(lpPathBuffer, cbPathBuffer, _T("/"));
		_tcscat_s(lpPathBuffer, cbPathBuffer, itor->cFileName);

		if(itor->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			findServerFiles(lpPathBuffer, cbPathBuffer);
		}else if(itor->dwFileAttributes & FILE_ATTRIBUTE_NORMAL)
		{
			for (std::list<UpdateFileInfo>::iterator iter = m_serverData_filesInfo.begin(); iter != m_serverData_filesInfo.end(); iter++)
			{
				if (iter->serverfile == lpPathBuffer)
				{
					iter->serverTimeVer = MyGetLastTimeFromFileinfo(&(*itor));
					iter->serverFileSize = (((uint64_t)itor->nFileSizeHigh) << 32) | (((uint64_t)itor->nFileSizeLow) << 0);
					break;
				}
			}
		}
	}

	retval = 1;

	return retval;
}
#endif
void CJsAutoUpdaterApp::ProgramExit()
{
	BOOL bRst;
	DWORD dwErr;

	::Gdiplus::GdiplusShutdown(gdiplusToken);

	exit(0);
}

std::basic_string<TCHAR> CJsAutoUpdaterApp::makeErrorString(std::basic_string<TCHAR> errstr)
{
	std::basic_string<TCHAR> str;
	str = errstr + _T("\n") + m_ContactAddr;
	return str;
}