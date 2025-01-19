// AboutDlg.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "JsAutoUpdater.h"
#include "AboutDlg.h"


// CAboutDlg 대화 상자입니다.

IMPLEMENT_DYNAMIC(CAboutDlg, CDialog)

CAboutDlg::CAboutDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CAboutDlg::IDD, pParent)
	, m_program_desc_value(_T(""))
{

}

CAboutDlg::~CAboutDlg()
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT1, m_program_desc_ctl);
	DDX_Text(pDX, IDC_EDIT1, m_program_desc_value);
}


BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// CAboutDlg 메시지 처리기입니다.

BOOL CAboutDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	SetWindowText(theApp.m_Updater_Name.c_str());

	m_program_desc_value = theApp.m_Program_Desc.c_str();
	m_program_desc_ctl.SetWindowText(m_program_desc_value);

	return TRUE;  // return TRUE unless you set the focus to a control
	// 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}
