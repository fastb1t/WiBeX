#pragma once

#include <tchar.h>
#include <string>
#include <windows.h>

typedef std::basic_string<TCHAR, std::char_traits<TCHAR>, std::allocator<TCHAR> > String;

BOOL SetPrivilege(HANDLE hToken, const TCHAR *lpszPrivilege, BOOL bEnablePrivilege);

BOOL DrawLine(HDC hDC, int x0, int y0, int x1, int y1);
void DrawEmptyRectangle(HDC hDC, int x0, int y0, int x1, int y1);

String to_String(long int);
