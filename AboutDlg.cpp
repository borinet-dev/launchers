// AboutDlg.cpp : ���� �����Դϴ�.
//

#include "stdafx.h"
#include "JsAutoUpdater.h"
#include "AboutDlg.h"


// CAboutDlg ��ȭ �����Դϴ�.

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


// CAboutDlg �޽��� ó�����Դϴ�.

BOOL CAboutDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	SetWindowText(theApp.m_Updater_Name.c_str());

	m_program_desc_value = theApp.m_Program_Desc.c_str();
	m_program_desc_ctl.SetWindowText(m_program_desc_value);

	return TRUE;  // return TRUE unless you set the focus to a control
	// ����: OCX �Ӽ� �������� FALSE�� ��ȯ�ؾ� �մϴ�.
}
