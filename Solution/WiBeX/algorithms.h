#pragma once

#include <tchar.h>
#include <windows.h>

BOOL SetPrivilege(HANDLE hToken, const TCHAR *lpszPrivilege, BOOL bEnablePrivilege);
