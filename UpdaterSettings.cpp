#include "stdafx.h"

#include "JsAutoUpdater.h"

#include <wininet.h>

void CJsAutoUpdaterApp::UpdaterSettings()
{
	/*
	 * �������� �̸� (Ÿ��Ʋ)�� �����մϴ�.
	 */
	this->m_Updater_Name = _T("�ͽ� - Lineage II Classic");

	/*
	 * �������� Path������ ������Ʈ�� �� Path
	 */
	//  HKEY_CURRENT_USER\\Software\\Microsoft\\7z\\InstallPath �� ���
	this->m_Updater_CheckRegKey = HKEY_CURRENT_USER;
	this->m_Updater_CheckRegPath = _T("Software\\Microsoft\\7z");
	this->m_Updater_CheckRegName = _T("InstallPath");
	
#if 0
	// Unused
	/*
	 * ������Ʈ�� ���α׷� �̸��� �����մϴ�.
	 */
	this->m_Program_Name = _T("���α׷��̸�");
#endif

	/*
	 * ���� URL�� �����մϴ�.
	 * ��) ftp://updater:12345678@example.com/updater
	 * ����) �������� "/"���� ������ ���ƾ� �մϴ�.
	 */
	this->m_ServerURL = _T("http://104.31.4.190/smartupdate_wassub");

	/*
	 * Basic ������ ����� ���� ������ �����մϴ�.
	 */
	m_ServerBasicUsername = _T("");
	m_ServerBasicPassword = _T("");

	/*
	* ���� �߻� �� �Բ� ����� �޽����� �Է��մϴ�.
	* ��) admin@example.com ���� �����ּ���.
	*/
	this->m_ContactAddr = _T("borinetserver@gmail.com ���� �����ּ���.");
	
	/*
	 * About Dialog�� ǥ���� ���α׷� ������ �����մϴ�.
	 */
	this->m_Program_Desc = _T("�ͽ� ����");

	/*
	 * ���������� ������ȣ�� �Է��մϴ�. Self update�� ���� ���˴ϴ�.
	 */
	this->m_UpdaterVersion = 30;

	/*
	 * ������Ʈ �Ϸ� �� ������ ���α׷��� �����մϴ�.
	 */

	this->m_ExecAfterUpdate = _T("systextures/shortcut.exe");

	/*
	 * ������Ʈ �Ϸ� �� ������ ���α׷��� ���� �μ��� ���� �� �ִ��� �����մϴ�.
	 * TRUE���� ���� ��
	 * Update.exe test.exe
	 * �̷��� �������͸� ������ ��� ������Ʈ���ʿ�/������Ʈ�Ϸ� �� test.exe�� �����մϴ�.
	 * �μ��� ���ٸ� ������ ������ ExecAfterUpdate�� �����մϴ�.
	 */
	this->m_PermitExecWithArg = FALSE;

	/*
	 * �⺻ �õ� FTP ���� ���ڵ� ����
	 * TRUE���� ������ OPTS UTF8 ON �� ���� UTF8������ �����ϸ� UTF8���� ó����
	 */
	this->m_ServerFTP_Charset_UTF8 = TRUE;

	this->m_backgroundColor = RGB(52, 52, 52); // ����
	this->m_textColor = RGB(255, 255, 255); // ���ڻ�

	this->m_serverData_logoLinks.push_back(_T("https://l2wassub.org")); // 1��° �α� ��ũ
	this->m_serverData_logoLinks.push_back(_T("https://discord.com/channels/1099477984777343009/1099479700692287549")); // 2��° �α� ��ũ
	this->m_serverData_logoLinks.push_back(_T("https://discord.com/channels/1099477984777343009/1099479748310204457")); // 3��° �α� ��ũ
	this->m_serverData_logoLinks.push_back(_T("https://discord.com/channels/1099477984777343009/1100690886838456351")); // 4��° �α� ��ũ
	this->m_serverData_logoLinks.push_back(_T("https://borinet-dev.github.io/AccountClassic/")); // 5��° �α� ��ũ
	this->m_serverData_logoLinks.push_back(_T("https://borinet-dev.github.io/boat/")); // 6��° �α� ��ũ
	this->m_serverData_logoLinks.push_back(_T("https://borinet-dev.github.io/blackcoupon/")); // 7��° �α� ��ũ(������)
	this->m_serverData_logoLinks.push_back(_T("https://borinet-dev.github.io/minigame/")); // 8��° �α� ��ũ(�̴ϰ���)
	this->m_serverData_logoLinks.push_back(_T("https://borinet-dev.github.io/mission/")); // 9��° �α� ��ũ(�̼�)
	this->m_serverData_logoLinks.push_back(_T("https://borinet-dev.github.io/board/")); // 10��° �α� ��ũ(Ŀ�´�Ƽ����)
	this->m_serverData_logoLinks.push_back(_T("https://borinet-dev.github.io/collection/")); // 11��° �α� ��ũ(�÷���)
	this->m_serverData_logoLinks.push_back(_T("https://borinet-dev.github.io/AutoUse/")); // 12��° �α� ��ũ(�ڵ�������)
	this->m_serverData_logoLinks.push_back(_T("https://borinet-dev.github.io/launcher/")); // 13��° �α� ��ũ(����)
	this->m_serverData_logoLinks.push_back(_T("https://borinet-dev.github.io/sg/")); // 14��° �α� ��ũ(����Ʈ����)
	this->m_serverData_logoLinks.push_back(_T("https://borinet-dev.github.io/commission/")); // 15��° �α� ��ũ(�ǸŴ���)
	this->m_serverData_logoLinks.push_back(_T("https://borinet-dev.github.io/GeneralStore/")); // 16��° �α� ��ũ(��������)
}