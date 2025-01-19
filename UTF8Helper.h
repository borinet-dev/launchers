#pragma once

#include <string>
#include <tchar.h>
#include <windows.h>

class UTF8Helper
{
public:
	static std::string tstr_to_utf8(const std::basic_string<TCHAR> &input);
	static std::basic_string<TCHAR> utf8_to_tstr(const std::string& input);
};

