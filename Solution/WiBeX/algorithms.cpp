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


// [DrawLine]:
BOOL DrawLine(HDC hDC, int x0, int y0, int x1, int y1)
{
    MoveToEx(hDC, x0, y0, NULL);
    return LineTo(hDC, x1, y1);
}
// [/DrawLine]


// [DrawEmptyRectangle]:
void DrawEmptyRectangle(HDC hDC, int x0, int y0, int x1, int y1)
{
    DrawLine(hDC, x0, y0, x0, y1);
    DrawLine(hDC, x1, y0, x1, y1 + 1);
    DrawLine(hDC, x0, y0, x1, y0);
    DrawLine(hDC, x0, y1, x1, y1);
}
// [/DrawEmptyRectangle]
