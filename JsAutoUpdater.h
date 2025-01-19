
// JsAutoUpdater.h : PROJECT_NAME 응용 프로그램에 대한 주 헤더 파일입니다.
//

#pragma once

#ifndef __AFXWIN_H__
	#error "PCH에 대해 이 파일을 포함하기 전에 'stdafx.h'를 포함합니다."
#endif

#include "resource.h"		// 주 기호입니다.

#include <list>
#include <map>
#include <string>
#include <vector>

#include <wininet.h>

#include "JsStdint.h"

#include "UpdaterDlg.h"

#include "HttpClient.h"

#pragma comment(lib, "wininet.lib")

#define JSAUTOUPDATER_FI_TYPE_UPDATER     0x00000001
#define JSAUTOUPDATER_FI_TYPE_UPDATEFILE  0x00000002
#define JSAUTOUPDATER_FI_TYPE_REMOVEFILE  0x00000008
#define JSAUTOUPDATER_FI_TYPE_KILLPROCESS 0x00000010
#define JSAUTOUPDATER_FI_TYPE_ONLY32BITS  0x00001000
#define JSAUTOUPDATER_FI_TYPE_ONLY64BITS  0x00002000
#define JSAUTOUPDATER_FI_TYPE_EXECNWAIT   0x00000100
#define JSAUTOUPDATER_FI_TYPE_EXEC        0x00000200
#define JSAUTOUPDATER_FI_TYPE_EXECALWAYS  0x00000400
#define JSAUTOUPDATER_FI_TYPE_MAKELINK    0x00010000

struct UpdateFileInfo
{
	int     type;
	int64_t version;
	std::basic_string<TCHAR> serverfile;
	std::basic_string<TCHAR> clientfile;
	
	int64_t serverTimeVer;
	int64_t serverFileSize;
	int64_t localTimeVer;
	int64_t localFileSize;
	
	int     update_flag;

	unsigned char filehash[20];
	
	UpdateFileInfo()
	{
		this->type = 0;
		this->version = 0;
		this->serverTimeVer = 0;
		this->serverFileSize = 0;
		this->localTimeVer = 0;
		this->localFileSize = 0;
		this->update_flag = -1;
	}
};

#define JSAUTOUPDATER_SELFDATA_OPTION_AUTOSTART          0x00000001

// CJsAutoUpdaterApp:
// 이 클래스의 구현에 대해서는 JsAutoUpdater.cpp을 참조하십시오.
//

class CJsAutoUpdaterApp : public CWinAppEx
{
public:
	CJsAutoUpdaterApp();

// 재정의입니다.
	public:
	virtual BOOL InitInstance();

// 구현입니다.

	DECLARE_MESSAGE_MAP()

private:
	void UpdaterSettings();
public:
	int m_execflag_new;
	std::basic_string<TCHAR> m_execflag_cmd;

	std::basic_string<TCHAR> m_Updater_Name;
	HKEY m_Updater_CheckRegKey;
	std::basic_string<TCHAR> m_Updater_CheckRegPath;
	std::basic_string<TCHAR> m_Updater_CheckRegName;

	std::basic_string<TCHAR> m_Program_Name;
	std::basic_string<TCHAR> m_ContactAddr;
	std::basic_string<TCHAR> m_ServerURL;
	std::basic_string<TCHAR> m_ServerBasicUsername;
	std::basic_string<TCHAR> m_ServerBasicPassword;
	std::basic_string<TCHAR> m_Program_Desc;
	int m_UpdaterVersion;

	BOOL    m_ServerFTP_Charset_UTF8;

	std::basic_string<TCHAR> m_ExecAfterUpdate;
	BOOL    m_PermitExecWithArg;

	std::basic_string<TCHAR> m_serverData_rawFilesInfo;
	std::list<UpdateFileInfo> m_serverData_filesInfo;

	std::list<std::basic_string<TCHAR> > m_serverData_logoLinks;
	//std::list<std::vector<unsigned char> > m_serverData_logoPngs;

	COLORREF m_backgroundColor;
	COLORREF m_textColor;

	std::basic_string<TCHAR> m_ServerData_UpdateMsg;

	std::basic_string<TCHAR> m_Updater_FileDBPath;

	void ProgramExit();

	std::basic_string<TCHAR> makeErrorString(std::basic_string<TCHAR> errstr);
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	BOOL findServerFiles(TCHAR *lpPathBuffer, size_t cbPathBuffer);

	CUpdaterDlg   *m_dlg_updater;

public:
	HttpClient m_http_client;
};

extern CJsAutoUpdaterApp theApp;
