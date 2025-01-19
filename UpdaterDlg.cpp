// UpdaterDlg.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "JsAutoUpdater.h"
#include "UpdaterDlg.h"

#include "Utils.h"
#include "ScaleBitmap.h"

#include "Shobjidl.h"

#include "AboutDlg.h"

#include <gdiplus.h>

#include "UTF8Helper.h"

#include <openssl/sha.h>

// CUpdaterDlg 대화 상자입니다.

const GUID MY_CLSID_TaskbarList ={ 0x56FDF344,0xFD6D,0x11d0,{0x95,0x8A,0x00,0x60,0x97,0xC9,0xA0,0x90}};
const GUID MY_IID_ITaskbarList3 = { 0xea1afb91,0x9e28,0x4b86,{0x90,0xe9,0x9e,0x9f,0x8a,0x5e,0xef,0xaf}};

IMPLEMENT_DYNAMIC(CUpdaterDlg, CDialog)

CUpdaterDlg::CUpdaterDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CUpdaterDlg::IDD, pParent)
{
	m_bShown = FALSE;

	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	m_pTaskBarlist = NULL;

	m_status = 0;
	
	m_DownloadSpeed = 0;
	MyAtomicLong_init(&m_DownloadSpeedChecker_Sum);
	m_Worker_hThread = INVALID_HANDLE_VALUE;
	m_DownloadSpeedChecker_hThread = INVALID_HANDLE_VALUE;
}

CUpdaterDlg::~CUpdaterDlg()
{
	m_DownloadSpeedChecker_bRun = FALSE;
	m_Worker_bRun = FALSE;
	if(m_Worker_hThread != INVALID_HANDLE_VALUE)
	{
		WaitForSingleObject(m_Worker_hThread, 2000);
		CloseHandle(m_Worker_hThread);
	}
	if(m_DownloadSpeedChecker_hThread != INVALID_HANDLE_VALUE)
	{
		WaitForSingleObject(m_DownloadSpeedChecker_hThread, 1200);
		CloseHandle(m_DownloadSpeedChecker_hThread);
	}
	MyAtomicLong_destroy(&m_DownloadSpeedChecker_Sum);

	if(m_pTaskBarlist)
	{
		m_pTaskBarlist = NULL;
	}
}

void CUpdaterDlg::setNeedUpdate(bool needUpdate)
{
	this->m_status = needUpdate ? 0 : 2;
	if (needUpdate)
	{
		m_status_initialText = _T("업데이트를 시작합니다.");
	} else {
		m_status_initialText = _T("'게임시작' 버튼을 눌러 게임을 실행해 주시기 바랍니다.");
	}
}

void CUpdaterDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LOGO_1, m_logo_ctls[0]);
	DDX_Control(pDX, IDC_LOGO_2, m_logo_ctls[1]);
	DDX_Control(pDX, IDC_LOGO_3, m_logo_ctls[2]);
	DDX_Control(pDX, IDC_LOGO_4, m_logo_ctls[3]);
	DDX_Control(pDX, IDC_LOGO_5, m_logo_ctls[4]);
	DDX_Control(pDX, IDC_LOGO_6, m_logo_ctls[5]);
	DDX_Control(pDX, IDC_LOGO_7, m_logo_ctls[6]);
	DDX_Control(pDX, IDC_LOGO_8, m_logo_ctls[7]);
	DDX_Control(pDX, IDC_LOGO_9, m_logo_ctls[8]);
	DDX_Control(pDX, IDC_LOGO_10, m_logo_ctls[9]);
	DDX_Control(pDX, IDC_LOGO_11, m_logo_ctls[10]);
	DDX_Control(pDX, IDC_LOGO_12, m_logo_ctls[11]);
	DDX_Control(pDX, IDC_LOGO_13, m_logo_ctls[12]);
	DDX_Control(pDX, IDC_LOGO_14, m_logo_ctls[13]);
	DDX_Control(pDX, IDC_LOGO_15, m_logo_ctls[14]);
	DDX_Control(pDX, IDC_LOGO_16, m_logo_ctls[15]);
	DDX_Control(pDX, IDC_PROGRESS1, m_progress_ctl);
	DDX_Control(pDX, IDC_STATUS, m_status_ctl);
	DDX_Control(pDX, IDC_RUN, m_updatebtn_ctl);
}


BEGIN_MESSAGE_MAP(CUpdaterDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_WM_SIZE()
	ON_BN_CLICKED(IDC_RUN, &CUpdaterDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDC_LOGO_1, &CUpdaterDlg::OnBnClickedLogo1)
	ON_BN_CLICKED(IDC_LOGO_2, &CUpdaterDlg::OnBnClickedLogo2)
	ON_BN_CLICKED(IDC_LOGO_3, &CUpdaterDlg::OnBnClickedLogo3)
	ON_BN_CLICKED(IDC_LOGO_4, &CUpdaterDlg::OnBnClickedLogo4)
	ON_BN_CLICKED(IDC_LOGO_5, &CUpdaterDlg::OnBnClickedLogo5)
	ON_BN_CLICKED(IDC_LOGO_6, &CUpdaterDlg::OnBnClickedLogo6)
	ON_BN_CLICKED(IDC_LOGO_7, &CUpdaterDlg::OnBnClickedLogo7)
	ON_BN_CLICKED(IDC_LOGO_8, &CUpdaterDlg::OnBnClickedLogo8)
	ON_BN_CLICKED(IDC_LOGO_9, &CUpdaterDlg::OnBnClickedLogo9)
	ON_BN_CLICKED(IDC_LOGO_10, &CUpdaterDlg::OnBnClickedLogo10)
	ON_BN_CLICKED(IDC_LOGO_11, &CUpdaterDlg::OnBnClickedLogo11)
	ON_BN_CLICKED(IDC_LOGO_12, &CUpdaterDlg::OnBnClickedLogo12)
	ON_BN_CLICKED(IDC_LOGO_13, &CUpdaterDlg::OnBnClickedLogo13)
	ON_BN_CLICKED(IDC_LOGO_14, &CUpdaterDlg::OnBnClickedLogo14)
	ON_BN_CLICKED(IDC_LOGO_15, &CUpdaterDlg::OnBnClickedLogo15)
	ON_BN_CLICKED(IDC_LOGO_16, &CUpdaterDlg::OnBnClickedLogo16)
	ON_WM_ERASEBKGND()
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()



static DWORD WINAPI DownloadSpeedCheckerThreadProc(LPVOID lparam);
static DWORD WINAPI WorkerThreadProc(LPVOID lparam);

// CUpdaterDlg 메시지 처리기입니다.

BOOL CUpdaterDlg::OnInitDialog()
{
	int i;
	CDialog::OnInitDialog();

	// 시스템 메뉴에 "정보..." 메뉴 항목을 추가합니다.

	// IDM_ABOUTBOX는 시스템 명령 범위에 있어야 합니다.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 이 대화 상자의 아이콘을 설정합니다. 응용 프로그램의 주 창이 대화 상자가 아닐 경우에는
	//  프레임워크가 이 작업을 자동으로 수행합니다.
	SetIcon(m_hIcon, TRUE);			// 큰 아이콘을 설정합니다.
	SetIcon(m_hIcon, FALSE);		// 작은 아이콘을 설정합니다.

	// TODO: 여기에 추가 초기화 작업을 추가합니다.
	
	m_backgroundBrush.CreateSolidBrush(theApp.m_backgroundColor);

	SetWindowText(theApp.m_Updater_Name.c_str());
	m_status_ctl.SetWindowText(m_status_initialText);

	m_background.Load(IDB_BG_PNG, _T("RC_DATA"));

	m_logo_ctls[0].LoadStdImage(IDB_LOGO_1_PNG, _T("RC_DATA"));
	m_logo_ctls[1].LoadStdImage(IDB_LOGO_2_PNG, _T("RC_DATA"));
	m_logo_ctls[2].LoadStdImage(IDB_LOGO_3_PNG, _T("RC_DATA"));
	m_logo_ctls[3].LoadStdImage(IDB_LOGO_4_PNG, _T("RC_DATA"));
	m_logo_ctls[4].LoadStdImage(IDB_LOGO_5_PNG, _T("RC_DATA"));
	m_logo_ctls[5].LoadStdImage(IDB_LOGO_6_PNG, _T("RC_DATA"));
	m_logo_ctls[6].LoadStdImage(IDB_LOGO_7_PNG, _T("RC_DATA"));
	m_logo_ctls[7].LoadStdImage(IDB_LOGO_8_PNG, _T("RC_DATA"));
	m_logo_ctls[8].LoadStdImage(IDB_LOGO_9_PNG, _T("RC_DATA"));
	m_logo_ctls[9].LoadStdImage(IDB_LOGO_10_PNG, _T("RC_DATA"));
	m_logo_ctls[10].LoadStdImage(IDB_LOGO_11_PNG, _T("RC_DATA"));
	m_logo_ctls[11].LoadStdImage(IDB_LOGO_12_PNG, _T("RC_DATA"));
	m_logo_ctls[12].LoadStdImage(IDB_LOGO_13_PNG, _T("RC_DATA"));
	m_logo_ctls[13].LoadStdImage(IDB_LOGO_14_PNG, _T("RC_DATA"));
	m_logo_ctls[14].LoadStdImage(IDB_LOGO_15_PNG, _T("RC_DATA"));
	m_logo_ctls[15].LoadStdImage(IDB_LOGO_16_PNG, _T("RC_DATA"));
	m_logo_ctls[16].LoadStdImage(IDB_LOGO_17_PNG, _T("RC_DATA"));

	/*
	i = 0;
	for (std::list<std::vector<unsigned char> >::const_iterator iterImage = theApp.m_serverData_logoPngs.begin(); iterImage != theApp.m_serverData_logoPngs.end(); iterImage++)
	{
		CComPtr<IStream> stream = SHCreateMemStream(iterImage->data(), iterImage->size());
		m_logo_ctls[i].EnableToggle(FALSE);
		if (stream)
		{
			m_logo_ctls[i].LoadStdImage(stream);
		}
		stream = SHCreateMemStream(iterImage->data(), iterImage->size());
		if (stream)
		{
			m_logo_ctls[i].LoadAltImage(stream);
		}
		i++;
	}
	*/
	
	m_updatebtn_ctl.LoadStdImage(IDB_PNG1, _T("RC_DATA"));

	Invalidate();

	m_DownloadSpeedChecker_bRun = TRUE;
	m_DownloadSpeedChecker_hThread = CreateThread(NULL, 0, DownloadSpeedCheckerThreadProc, this, 0, NULL);

	if (m_status == 0)
	{
		startUpdate();
	}

	return TRUE;  // 포커스를 컨트롤에 설정하지 않으면 TRUE를 반환합니다.
}

// 사용자가 최소화된 창을 끄는 동안에 커서가 표시되도록 시스템에서
//  이 함수를 호출합니다.
HCURSOR CUpdaterDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

BOOL CUpdaterDlg::PreTranslateMessage(MSG* pMsg)
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.

	if (!m_bShown) {
		// On pre-Win 7, anyone can register a message called "TaskbarButtonCreated"
		// and broadcast it, so make sure the OS is Win 7 or later before acting on
		// the message. (This isn't a problem for this app, which won't run on pre-7,
		// but you should definitely do this check if your app will run on pre-7.)
		OSVERSIONINFOEX  versionInfo;
		versionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
		GetVersionEx((POSVERSIONINFO)&versionInfo);

		m_bShown = TRUE;

		// Check that the Windows version is at least 6.1 (yes, Win 7 is version 6.1).
		if ( versionInfo.dwMajorVersion > 6 || ( versionInfo.dwMajorVersion == 6 && versionInfo.dwMinorVersion > 0 ) )
		{
			CoCreateInstance(CLSID_TaskbarList, NULL, CLSCTX_ALL, MY_IID_ITaskbarList3, (void**)&m_pTaskBarlist);
			if(m_pTaskBarlist == NULL)
			{
				DWORD dwErr = GetLastError();
				printf("dummy");
			}else{
				m_pTaskBarlist->HrInit();
			}
		}
	}

	switch(pMsg->message)
	{
	case WM_KEYDOWN:
        if( VK_ESCAPE == pMsg->wParam )
        {
            return TRUE;
        }
        else if( VK_RETURN == pMsg->wParam )
        {
            return TRUE;
        }
		break;
    }

	return CDialog::PreTranslateMessage(pMsg);
}

void CUpdaterDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);
}

void CUpdaterDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		switch(nID)
		{
		case SC_CLOSE:
			if(m_status == 1) return;
			break;
		}
		CDialog::OnSysCommand(nID, lParam);
	}
}

void CUpdaterDlg::setProgress(int min, int max, int value)
{
	int max2 = max - min;
	int val2 = value - min;
	m_progress_ctl.SetRange32(min, max);
	m_progress_ctl.SetPos(value);
	if ( m_pTaskBarlist )
            m_pTaskBarlist->SetProgressValue(m_hWnd, val2, max2);
}

void CUpdaterDlg::startUpdate()
{
	m_updatebtn_ctl.EnableWindow(FALSE);

	if(m_Worker_hThread != INVALID_HANDLE_VALUE)
	{
		WaitForSingleObject(m_Worker_hThread, 2000);
		CloseHandle(m_Worker_hThread);
	}

	m_Worker_bRun = TRUE;
	m_Worker_hThread = CreateThread(NULL, 0, WorkerThreadProc, this, 0, NULL);
}

static CString getHumanBytes(long bytes)
{
	CString str;

	float flt = (float)bytes;

	if(bytes < 1024)
	{
		str.Format(_T("%d Byte/s"), bytes);
	}else if(bytes < (1024*1024))
	{
		flt /= 1024;
		str.Format(_T("%.3f KB/s"), flt);
	}else if(bytes < (1024*1024*1024))
	{
		flt /= (1024 * 1024);
		str.Format(_T("%.3f MB/s"), flt);
	}else{
		flt /= (1024 * 1024 * 1024);
		str.Format(_T("%.3f GB/s"), flt);
	}

	return str;
}

DWORD WINAPI DownloadSpeedCheckerThreadProc(LPVOID lparam)
{
	CUpdaterDlg *pthis = (CUpdaterDlg*)lparam;

	while(pthis->m_DownloadSpeedChecker_bRun)
	{
		CString str;
		long tmp;
		tmp = MyAtomicLong_getAndset(&pthis->m_DownloadSpeedChecker_Sum, 0);
		pthis->m_DownloadSpeed = tmp;
		str = _T("Download Speed : \n") + getHumanBytes(tmp);
		//pthis->m_downloadspeed_ctl.SetWindowText(str);
		Sleep(1000);
	}

	return 0;
}

class CUpdaterDlg::DownloadContext : public HttpClient::HttpResponseHandler
{
private:
	MyAtomicLong_t* psum_;

public:
	SHA_CTX hash_ctx_;
	
public:
	DownloadContext(MyAtomicLong_t* psum)
		: psum_(psum)
	{
		SHA1_Init(&hash_ctx_);
	}
	bool onHttpResponse(CURL* curl, void* contents, size_t size, size_t nmemb) override
	{
		MyAtomicLong_getAndplus(psum_, nmemb * size);
		SHA1_Update(&hash_ctx_, contents, size * nmemb);
		return true;
	}
};

static std::string hashToHex(const unsigned char *hash)
{
	int i;
	static const char* HEXSTRINGS = "0123456789abcdef";
	std::string hexText;
	for (i = 0; i < 20; i++)
	{
		int j;
		unsigned char d = hash[i];
		for (j = 0; j < 2; j++)
		{
			unsigned char c = (d & 0xf0) >> 4;
			hexText.append(&HEXSTRINGS[c], 1);
			d <<= 4;
		}
	}
	return hexText;
}

DWORD WINAPI CUpdaterDlg::WorkerThreadProc(LPVOID lparam)
{
	CUpdaterDlg *pthis = (CUpdaterDlg*)lparam;

	int nRst;
	int count = 0;

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

	int progress_min = 0;
	int progress_max;

	int listCount;

	std::basic_string<TCHAR> tmpstr;

	pthis->m_status = 1;
	pthis->m_status_ctl.SetWindowText(_T("업데이트 중..."));

	progress_max = theApp.m_serverData_filesInfo.size();

	for(std::list<UpdateFileInfo>::iterator itor = theApp.m_serverData_filesInfo.begin(); pthis->m_Worker_bRun && (itor != theApp.m_serverData_filesInfo.end()); itor++)
	{
		BOOL bRst;
		DWORD dwErr;

		CString str;
		std::basic_string<TCHAR> strserverpath = itor->serverfile;
		std::basic_string<TCHAR> strclientpath = itor->clientfile;
		
		if (itor->type & JSAUTOUPDATER_FI_TYPE_UPDATEFILE)
		{
			if (itor->update_flag != 0) {
				std::basic_string<TCHAR> parentDir;

				//str.Format(_T("파일다운로드: %s"), strclientpath);
				//pthis->m_updateloglist_ctl.AddString(str);

				parentDir = getParentDirPath((TCHAR*)strclientpath.c_str());

				nRst = SHCreateDirectoryEx(NULL, parentDir.c_str(), NULL);
				if (!((nRst == 0) || (nRst == 183)))
				{
					//str.Format(_T("상위 디렉토리 생성 실패! %d"), nRst);
					//pthis->m_updateloglist_ctl.AddString(str);
				}

				{
					DownloadContext download_context(&pthis->m_DownloadSpeedChecker_Sum);
					std::unique_ptr<HttpClient::SimpleResponse> response = theApp.m_http_client.simpleDownload(UTF8Helper::tstr_to_utf8(_T("/L2-Class/") + itor->serverfile), strclientpath.c_str(), &download_context);
					SHA1_Final(itor->filehash, &download_context.hash_ctx_);
					if ((response->http_status >= 200) && (response->http_status < 400))
					{
						//str.Format(_T("완료!"));
						//theApp.m_Updater_FileDB_Rows[itor->m_clientfile] = itor->m_version;
					}
					else {
						//str.Format(_T("오류! %d"), nRst);
					}
				}
				//pthis->m_updateloglist_ctl.AddString(str);
			}
		}
		else if (itor->type & JSAUTOUPDATER_FI_TYPE_REMOVEFILE)
		{
			//str.Format(_T("파일삭제: %s"), strclientpath);
			//pthis->m_updateloglist_ctl.AddString(str);
			bRst = DeleteFile(strclientpath.c_str());
			if (bRst)
			{
				str.Format(_T("완료!"));
			}
			else {
				str.Format(_T("실패!"));
			}
			//pthis->m_updateloglist_ctl.AddString(str);
		}
		else if (itor->type & JSAUTOUPDATER_FI_TYPE_KILLPROCESS)
		{
			str.Format(_T("프로세스종료: %s"), strclientpath);
			//pthis->m_updateloglist_ctl.AddString(str);
			killProcessByName((TCHAR*)strclientpath.c_str());
		}
		else if (itor->type & JSAUTOUPDATER_FI_TYPE_EXEC)
		{
			str.Format(_T("파일실행: %s"), strclientpath);
			//pthis->m_updateloglist_ctl.AddString(str);
			_tsystem(std::basic_string<TCHAR>(_T("start ") + strclientpath).c_str());
		}
		else if (itor->type & JSAUTOUPDATER_FI_TYPE_EXECNWAIT)
		{
			str.Format(_T("파일실행 후 기다림: %s"), strclientpath);
			//pthis->m_updateloglist_ctl.AddString(str);
			_tsystem(strclientpath.c_str());
			str.Format(_T("완료!"));
			//pthis->m_updateloglist_ctl.AddString(str);
		}
		else if (itor->type & JSAUTOUPDATER_FI_TYPE_MAKELINK)
		{
			str.Format(_T("바로가기 생성: %s"), strclientpath);
			//pthis->m_updateloglist_ctl.AddString(str);
			bRst = CreateShortcut(strserverpath.c_str(), strclientpath.c_str(), NULL, NULL, NULL, -1, 0, SW_SHOW);
			if (bRst)
			{
				str.Format(_T("완료!"));
			}
			else {
				str.Format(_T("실패!"));
			}
			//pthis->m_updateloglist_ctl.AddString(str);
		}
		else {
			//str.Format(_T("지정되지 않은 오류입니다."));
			//pthis->m_updateloglist_ctl.AddString(str);
		}

		count++;

		pthis->setProgress(progress_min, progress_max, count);
	}

	{
		FILE *fp;
		errno_t eno;
		eno = _tfopen_s(&fp, theApp.m_Updater_FileDBPath.c_str(), _T("wt"));
		for (std::list<UpdateFileInfo>::iterator itor = theApp.m_serverData_filesInfo.begin(); pthis->m_Worker_bRun && (itor != theApp.m_serverData_filesInfo.end()); itor++)
		{
			if (itor->type & JSAUTOUPDATER_FI_TYPE_UPDATEFILE)
			{
				std::string hash = hashToHex(itor->filehash);
				if (itor->version < 0)
				{
					_ftprintf(fp, _T("%lld|%s|%s|%S\n"), (int64_t)itor->serverTimeVer, itor->serverfile.c_str(), itor->clientfile.c_str(), hash.c_str());
				} else {
					_ftprintf(fp, _T("%lld|%s|%s|%S\n"), (int64_t)itor->version, itor->serverfile.c_str(), itor->clientfile.c_str(), hash.c_str());
				}
			}
		}
		fclose(fp);

	}
	pthis->m_status_ctl.SetWindowText(_T("업데이트가 완료되었습니다."));

	pthis->m_Worker_bRun = FALSE;
	pthis->m_updatebtn_ctl.EnableWindow(TRUE);

	pthis->m_status = 2;
	return 0;
}

void CUpdaterDlg::OnBnClickedOk()
{
	if (m_status != 2)
	{
		startUpdate();
	} else {
		BOOL bRst = FALSE;
		DWORD dwErr = 0;

		for (std::list<UpdateFileInfo>::iterator itor = theApp.m_serverData_filesInfo.begin(); itor != theApp.m_serverData_filesInfo.end(); itor++)
		{
			std::basic_string<TCHAR> strclientpath = itor->clientfile;
			if (itor->type & JSAUTOUPDATER_FI_TYPE_EXECALWAYS)
			{
				_tsystem(std::basic_string<TCHAR>(_T("start ") + strclientpath).c_str());
			}
		}

		if (!theApp.m_ExecAfterUpdate.empty())
		{
			std::basic_string<TCHAR> strExec;
			SHELLEXECUTEINFO si;

			if (PathIsRelative(theApp.m_ExecAfterUpdate.c_str()))
			{
				strExec = getExeDirPath() + _T("\\") + theApp.m_ExecAfterUpdate;
			}
			else {
				strExec = theApp.m_ExecAfterUpdate;
			}

			if (PathFileExists(strExec.c_str()))
			{
				ZeroMemory(&si, sizeof(SHELLEXECUTEINFO));
				si.cbSize = sizeof(SHELLEXECUTEINFO);
				si.lpVerb = _T("open");
				si.lpFile = strExec.c_str();
				si.lpParameters = NULL;
				si.lpDirectory = NULL;
				si.nShow = SW_SHOW;
				si.fMask = SEE_MASK_NOCLOSEPROCESS | SEE_MASK_FLAG_DDEWAIT;
				si.hwnd = NULL;
				bRst = ShellExecuteEx(&si);
				dwErr = GetLastError();
				this->PostMessage(WM_QUIT);
			} else {
				::MessageBox(NULL, std::basic_string<TCHAR>(theApp.m_ExecAfterUpdate + _T(" 파일이 존재하지 않습니다.")).c_str(), theApp.m_Updater_Name.c_str(), 0);
			}
		}
	}
}

void CUpdaterDlg::openLogoLink(int index)
{
	int i = 0;
	for (std::list<std::basic_string<TCHAR> >::const_iterator iter = theApp.m_serverData_logoLinks.begin(); iter != theApp.m_serverData_logoLinks.end(); iter++)
	{
		i++;
		if (i == index)
		{
			::ShellExecute(NULL, _T("open"), iter->c_str(), NULL, NULL, SW_SHOW);
			break;
		}
	}
}

afx_msg void CUpdaterDlg::OnBnClickedLogo1() {
	openLogoLink(1);
}
afx_msg void CUpdaterDlg::OnBnClickedLogo2() {
	openLogoLink(2);
}
afx_msg void CUpdaterDlg::OnBnClickedLogo3() {
	openLogoLink(3);
}
afx_msg void CUpdaterDlg::OnBnClickedLogo4() {
	openLogoLink(4);
}
afx_msg void CUpdaterDlg::OnBnClickedLogo5() {
	openLogoLink(5);
}
afx_msg void CUpdaterDlg::OnBnClickedLogo6() {
	openLogoLink(6);
}
afx_msg void CUpdaterDlg::OnBnClickedLogo7() {
	openLogoLink(7);
}
afx_msg void CUpdaterDlg::OnBnClickedLogo8() {
	openLogoLink(8);
}
afx_msg void CUpdaterDlg::OnBnClickedLogo9() {
	openLogoLink(9);
}
afx_msg void CUpdaterDlg::OnBnClickedLogo10() {
	openLogoLink(10);
}
afx_msg void CUpdaterDlg::OnBnClickedLogo11() {
	openLogoLink(11);
}
afx_msg void CUpdaterDlg::OnBnClickedLogo12() {
	openLogoLink(12);
}
afx_msg void CUpdaterDlg::OnBnClickedLogo13() {
	openLogoLink(13);
}
afx_msg void CUpdaterDlg::OnBnClickedLogo14() {
	openLogoLink(14);
}
afx_msg void CUpdaterDlg::OnBnClickedLogo15() {
	openLogoLink(15);
}
afx_msg void CUpdaterDlg::OnBnClickedLogo16() {
	openLogoLink(16);
}


BOOL CUpdaterDlg::OnEraseBkgnd(CDC* pDC)
{
	CRect bodyRect;
	HDC screen = ::GetDC(0);
	CDialog::OnEraseBkgnd(pDC);

	int dpiX = GetDeviceCaps(screen, LOGPIXELSX);
	int dpiY = GetDeviceCaps(screen, LOGPIXELSY);
	float dpi = dpiY / 81.0f;

	GetClientRect(&bodyRect);

	CBrush *pOld = pDC->SelectObject(&m_backgroundBrush);
	BOOL bRes = pDC->PatBlt(0, 0, bodyRect.Width(), bodyRect.Height(), PATCOPY);
	pDC->SelectObject(pOld); // restore old brush

	if (m_background.m_pBitmap)
	{
		Gdiplus::Graphics graphics(pDC->m_hDC);
		CRect wndRect;
		CRect btnRect;
		GetWindowRect(&wndRect);
		m_updatebtn_ctl.GetWindowRect(&btnRect);
		wndRect.bottom = btnRect.top - (wndRect.Width() - bodyRect.Width()) - 20 * dpi;
		graphics.DrawImage(m_background.m_pBitmap, 0, 0, wndRect.Width(), wndRect.Height());
	}

	return TRUE;
}


HBRUSH CUpdaterDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
	if (pWnd->GetDlgCtrlID() == IDC_STATUS) {
		pDC->SetBkColor(theApp.m_backgroundColor);
		pDC->SetTextColor(theApp.m_textColor);
	}
	hbr = m_backgroundBrush;
	return hbr;
}
