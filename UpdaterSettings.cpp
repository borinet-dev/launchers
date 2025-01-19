#include "stdafx.h"

#include "JsAutoUpdater.h"

#include <wininet.h>

void CJsAutoUpdaterApp::UpdaterSettings()
{
	/*
	 * 업데이터 이름 (타이틀)을 설정합니다.
	 */
	this->m_Updater_Name = _T("와썹 - Lineage II Classic");

	/*
	 * 업데이터 Path검증할 레지스트리 값 Path
	 */
	//  HKEY_CURRENT_USER\\Software\\Microsoft\\7z\\InstallPath 인 경우
	this->m_Updater_CheckRegKey = HKEY_CURRENT_USER;
	this->m_Updater_CheckRegPath = _T("Software\\Microsoft\\7z");
	this->m_Updater_CheckRegName = _T("InstallPath");
	
#if 0
	// Unused
	/*
	 * 업데이트할 프로그램 이름을 설정합니다.
	 */
	this->m_Program_Name = _T("프로그램이름");
#endif

	/*
	 * 서버 URL을 설정합니다.
	 * 예) ftp://updater:12345678@example.com/updater
	 * 주의) 마지막은 "/"으로 끝나지 말아야 합니다.
	 */
	this->m_ServerURL = _T("http://104.31.4.190/smartupdate_wassub");

	/*
	 * Basic 인증시 사용자 계정 정보를 설정합니다.
	 */
	m_ServerBasicUsername = _T("");
	m_ServerBasicPassword = _T("");

	/*
	* 오류 발생 시 함께 출력할 메시지를 입력합니다.
	* 예) admin@example.com 으로 연락주세요.
	*/
	this->m_ContactAddr = _T("borinetserver@gmail.com 으로 연락주세요.");
	
	/*
	 * About Dialog에 표시할 프로그램 정보를 설정합니다.
	 */
	this->m_Program_Desc = _T("와썹 런쳐");

	/*
	 * 업데이터의 버전번호를 입력합니다. Self update를 위해 사용됩니다.
	 */
	this->m_UpdaterVersion = 30;

	/*
	 * 업데이트 완료 후 실행할 프로그램을 지정합니다.
	 */

	this->m_ExecAfterUpdate = _T("systextures/shortcut.exe");

	/*
	 * 업데이트 완료 후 실행할 프로그램을 실행 인수로 받을 수 있는지 설정합니다.
	 * TRUE으로 설정 시
	 * Update.exe test.exe
	 * 이렇게 업데이터를 실행할 경우 업데이트불필요/업데이트완료 후 test.exe을 실행합니다.
	 * 인수가 없다면 위에서 지정한 ExecAfterUpdate을 실행합니다.
	 */
	this->m_PermitExecWithArg = FALSE;

	/*
	 * 기본 시도 FTP 서버 인코딩 설정
	 * TRUE으로 설정시 OPTS UTF8 ON 을 보내 UTF8적용이 가능하면 UTF8으로 처리함
	 */
	this->m_ServerFTP_Charset_UTF8 = TRUE;

	this->m_backgroundColor = RGB(52, 52, 52); // 배경색
	this->m_textColor = RGB(255, 255, 255); // 글자색

	this->m_serverData_logoLinks.push_back(_T("https://l2wassub.org")); // 1번째 로그 링크
	this->m_serverData_logoLinks.push_back(_T("https://discord.com/channels/1099477984777343009/1099479700692287549")); // 2번째 로그 링크
	this->m_serverData_logoLinks.push_back(_T("https://discord.com/channels/1099477984777343009/1099479748310204457")); // 3번째 로그 링크
	this->m_serverData_logoLinks.push_back(_T("https://discord.com/channels/1099477984777343009/1100690886838456351")); // 4번째 로그 링크
	this->m_serverData_logoLinks.push_back(_T("https://borinet-dev.github.io/AccountClassic/")); // 5번째 로그 링크
	this->m_serverData_logoLinks.push_back(_T("https://borinet-dev.github.io/boat/")); // 6번째 로그 링크
	this->m_serverData_logoLinks.push_back(_T("https://borinet-dev.github.io/blackcoupon/")); // 7번째 로그 링크(블랙쿠폰)
	this->m_serverData_logoLinks.push_back(_T("https://borinet-dev.github.io/minigame/")); // 8번째 로그 링크(미니게임)
	this->m_serverData_logoLinks.push_back(_T("https://borinet-dev.github.io/mission/")); // 9번째 로그 링크(미션)
	this->m_serverData_logoLinks.push_back(_T("https://borinet-dev.github.io/board/")); // 10번째 로그 링크(커뮤니티보드)
	this->m_serverData_logoLinks.push_back(_T("https://borinet-dev.github.io/collection/")); // 11번째 로그 링크(컬렉션)
	this->m_serverData_logoLinks.push_back(_T("https://borinet-dev.github.io/AutoUse/")); // 12번째 로그 링크(자동아이템)
	this->m_serverData_logoLinks.push_back(_T("https://borinet-dev.github.io/launcher/")); // 13번째 로그 링크(런쳐)
	this->m_serverData_logoLinks.push_back(_T("https://borinet-dev.github.io/sg/")); // 14번째 로그 링크(스마트가드)
	this->m_serverData_logoLinks.push_back(_T("https://borinet-dev.github.io/commission/")); // 15번째 로그 링크(판매대행)
	this->m_serverData_logoLinks.push_back(_T("https://borinet-dev.github.io/GeneralStore/")); // 16번째 로그 링크(만물상점)
}