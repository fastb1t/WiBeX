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

    if (!AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(TOKEN_PRIVILEGES), NULL, NULL))
        return FALSE;

    if (GetLastError() == ERROR_NOT_ALL_ASSIGNED)
        return FALSE;
    return TRUE;
}
// [/SetPrivilege]


// [to_String]:
String to_String(long int k)
{
    TCHAR szTemp[16] = { 0 };
    wsprintf(szTemp, _T("%d"), k);
    return szTemp;
}
// [/to_String]


// [wchar_t_to_string]:
std::string wchar_t_to_string(wchar_t *input)
{
    std::string out = "";

    int res_len = WideCharToMultiByte(1251, 0, input, -1, NULL, 0, NULL, NULL);
    if (!res_len)
        return out;
    char *res = new char[sizeof(char) * res_len];
    if (!res)
        return out;
    if (!WideCharToMultiByte(1251, 0, input, -1, res, res_len, NULL, NULL))
    {
        delete[]res;
        return out;
    }
    out = res;
    delete[]res;
    return out;
}
// [/wchar_t_to_string]


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


// [DrawHeadlineForPart]: 
void DrawHeadlineForPart(HDC hDC, int x, int y, int iWidth, const TCHAR *szText, HBRUSH hBrush, DC *img)
{
    SIZE size;
    GetTextExtentPoint32(hDC, szText, lstrlen(szText), &size);

    int iHeight = size.cy;
    if (iHeight < 18)
        iHeight = 20;

    HBRUSH hOldBrush = NULL;
    if (hBrush)
        hOldBrush = (HBRUSH)SelectObject(hDC, hBrush);

    Rectangle(hDC, x, y, x + iWidth + 1, y + iHeight + 1);

    if (hOldBrush)
        SelectObject(hDC, hOldBrush);

    if (img->HasImage())
        img->Draw(hDC, x + 2, y + 2);

    COLORREF clrOldColor = SetTextColor(hDC, 0x00);
    TextOut(hDC, x + 20, y + (iHeight >> 1) - (size.cy >> 1), szText, lstrlen(szText));
    SetTextColor(hDC, clrOldColor);
}
// [/DrawHeadlineForPart]


// [FileExists]:
bool FileExists(String file)
{
    bool bRetVal = false;
    WIN32_FIND_DATA wfd;
    HANDLE hFile = FindFirstFile(file.c_str(), &wfd);
    if (hFile != INVALID_HANDLE_VALUE)
        bRetVal = true;
    FindClose(hFile);
    return bRetVal;
}
// [/FileExists]


// [InjectDll]:
bool InjectDll(const char *szDllName, DWORD dwProcessId)
{
    HANDLE hProcess = OpenProcess(PROCESS_CREATE_THREAD | PROCESS_QUERY_INFORMATION | PROCESS_VM_OPERATION | PROCESS_VM_READ | PROCESS_VM_WRITE, FALSE, dwProcessId);
    if (!hProcess)
        return false;

    HMODULE hKernel32 = GetModuleHandle(_T("kernel32.dll"));
    if (!hKernel32)
    {
        CloseHandle(hProcess);
        return false;
    }

    LPVOID lpLoadLibraryAddress = (LPVOID)GetProcAddress(hKernel32, "LoadLibraryA");
    if (!lpLoadLibraryAddress)
    {
        CloseHandle(hProcess);
        return false;
    }

    LPVOID lpRemoteString = (LPVOID)VirtualAllocEx(hProcess, NULL, strlen(szDllName) + 1, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    if (!lpRemoteString)
    {
        CloseHandle(hProcess);
        return false;
    }

    if (!WriteProcessMemory(hProcess, lpRemoteString, (LPVOID)szDllName, strlen(szDllName) + 1, NULL))
    {
        CloseHandle(hProcess);
        return false;
    }

    if (!CreateRemoteThread(hProcess, NULL, NULL, (LPTHREAD_START_ROUTINE)lpLoadLibraryAddress, lpRemoteString, NULL, NULL))
    {
        CloseHandle(hProcess);
        return false;
    }

    CloseHandle(hProcess);
    return true;
}
// [/InjectDll]
