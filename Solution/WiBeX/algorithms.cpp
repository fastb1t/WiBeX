#include "algorithms.h"

// [SetPrivilege]:
BOOL SetPrivilege(HANDLE hToken, const TCHAR *lpszPrivilege, BOOL bEnablePrivilege)
{
    LUID luid;
    if (!LookupPrivilegeValue(NULL, lpszPrivilege, &luid))
        return FALSE;

    TOKEN_PRIVILEGES tp;
    tp.PrivilegeCount = 1;
    tp.Privileges[0].Luid = luid;
    if (bEnablePrivilege)
        tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
    else
        tp.Privileges[0].Attributes = 0;

    if (!AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(TOKEN_PRIVILEGES), (PTOKEN_PRIVILEGES)NULL, (PDWORD)NULL))
        return FALSE;

    if (GetLastError() == ERROR_NOT_ALL_ASSIGNED)
        return FALSE;
    return TRUE;
}
// [/SetPrivilege]
