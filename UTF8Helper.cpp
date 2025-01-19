#include "stdafx.h"
#include "UTF8Helper.h"

std::string UTF8Helper::tstr_to_utf8(const std::basic_string<TCHAR>& input)
{
	std::string ubuffer;

	if (input.empty())
		return "";

	ubuffer.reserve(input.length() * 4);
	ubuffer.resize(input.length() * 4);

	int nUTF8codeSize = WideCharToMultiByte(CP_UTF8, 0, input.c_str(), input.length(), &ubuffer[0], ubuffer.capacity(), NULL, NULL);
	ubuffer.resize(nUTF8codeSize);

	return ubuffer;
}

std::basic_string<TCHAR> UTF8Helper::utf8_to_tstr(const std::string& input)
{
	std::basic_string<wchar_t> wbuffer;

	if (input.empty())
		return _T("");

	wbuffer.reserve(input.length());
	wbuffer.resize(input.length());

	int nSize = MultiByteToWideChar(CP_UTF8, 0, input.c_str(), input.length(), &wbuffer[0], wbuffer.capacity());
	wbuffer.resize(nSize);

	return wbuffer;
}

