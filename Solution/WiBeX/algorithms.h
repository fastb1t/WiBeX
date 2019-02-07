#pragma once

#include <tchar.h>
#include <string>
#include <windows.h>

#include "DC.h"

typedef std::basic_string<TCHAR, std::char_traits<TCHAR>, std::allocator<TCHAR> > String;

BOOL SetPrivilege(HANDLE hToken, const TCHAR *lpszPrivilege, BOOL bEnablePrivilege);
String to_String(long int);
std::string wchar_t_to_string(wchar_t *input);

BOOL DrawLine(HDC hDC, int x0, int y0, int x1, int y1);
void DrawEmptyRectangle(HDC hDC, int x0, int y0, int x1, int y1);

void DrawHeadlineForPart(HDC hDC, int x, int y, int iWidth, const TCHAR *szText, HBRUSH hBrush, DC *img);

bool FileExists(String file);
bool MutexIsExist(const TCHAR *szMutexName);
bool InjectDll(const char *szDllName, DWORD dwProcessId);
