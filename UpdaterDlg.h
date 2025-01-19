#pragma once
#include "afxwin.h"

#include "MyAtomicLong.h"

#include "GdipButton.h"
#include "CGdiPlusBitmap.h"

// CUpdaterDlg 대화 상자입니다.

class CUpdaterDlg : public CDialog
{
	DECLARE_DYNAMIC(CUpdaterDlg)

public:
	CUpdaterDlg(CWnd* pParent = NULL);   // 표준 생성자입니다.
	virtual ~CUpdaterDlg();

// 대화 상자 데이터입니다.
	enum { IDD = IDD_UPDATER_DIALOG };

	virtual BOOL PreTranslateMessage(MSG* pMsg);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

protected:
	HICON m_hIcon;

	// 생성된 메시지 맵 함수
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	DECLARE_MESSAGE_MAP()

private:
	class DownloadContext;

	BOOL m_bShown;

	CProgressCtrl m_progress_ctl;
	CGdipButton m_updatebtn_ctl;
	CStatic m_status_ctl;
	CString m_status_initialText;
	
	ITaskbarList3* m_pTaskBarlist;

	CGdiPlusBitmapResource m_background;
	CBrush m_backgroundBrush;

	int m_sizeratio_updateloglist_x;
	int m_sizeratio_updateloglist_y;
	int m_sizeratio_progress_x;
	int m_sizeratio_progress_y;
	int m_sizeratio_downloadspeed_y;

	void startUpdate();

	static DWORD WINAPI WorkerThreadProc(LPVOID lparam);

public:
	CGdipButton m_logo_ctls[17];

	volatile BOOL m_status;

	HANDLE         m_Worker_hThread;
	volatile BOOL  m_Worker_bRun;

	HANDLE         m_DownloadSpeedChecker_hThread;
	volatile BOOL  m_DownloadSpeedChecker_bRun;
	MyAtomicLong_t m_DownloadSpeedChecker_Sum;
	volatile long  m_DownloadSpeed;

	void setProgress(int min, int max, int value);

	void openLogoLink(int index);

public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedLogo1();
	afx_msg void OnBnClickedLogo2();
	afx_msg void OnBnClickedLogo3();
	afx_msg void OnBnClickedLogo4();
	afx_msg void OnBnClickedLogo5();
	afx_msg void OnBnClickedLogo6();
	afx_msg void OnBnClickedLogo7();
	afx_msg void OnBnClickedLogo8();
	afx_msg void OnBnClickedLogo9();
	afx_msg void OnBnClickedLogo10();
	afx_msg void OnBnClickedLogo11();
	afx_msg void OnBnClickedLogo12();
	afx_msg void OnBnClickedLogo13();
	afx_msg void OnBnClickedLogo14();
	afx_msg void OnBnClickedLogo15();
	afx_msg void OnBnClickedLogo16();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);

	void setNeedUpdate(bool needUpdate);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnStnClickedStatus();
};
