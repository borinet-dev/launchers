#ifndef __UTILS_H__
#define __UTILS_H__

#include <string>

extern int GetFindCharCount(std::basic_string<TCHAR> msg, char find_char);


extern std::basic_string<TCHAR> getExePath();
extern std::basic_string<TCHAR> getExeDirPath();
extern std::basic_string<TCHAR> getParentDirPath(TCHAR *str);

extern void killProcessByName(TCHAR *filepath);

extern std::basic_string<TCHAR> utf8ToString(char *str, int len);

extern BOOL CreateShortcut(LPCTSTR szTargetPath,
                    LPCTSTR szShortcutPath,
                    LPCTSTR szDesc,
                    LPCTSTR szArgument,
                    LPCTSTR szIconPath,
                    int iIconIndex,
                    WORD wHotkey,
                    int nShowCmd);

extern BOOL GetFolderPath(int nFolder, REFKNOWNFOLDERID rfid, std::basic_string<TCHAR>& retPath);

#endif