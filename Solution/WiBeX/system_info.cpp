#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <tchar.h>
#include <winsock2.h>
#include <windows.h>
#include <windowsx.h>

#include "system_info.h"

static BOOL DlgOnInitDialog(HWND, HWND, LPARAM);                            // WM_INITDIALOG
static void DlgOnCommand(HWND, int, HWND, UINT);                            // WM_COMMAND
static void DlgOnPaint(HWND);                                               // WM_PAINT


// [SystemInfo_DialogProcedure]:
BOOL CALLBACK SystemInfo_DialogProcedure(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
        HANDLE_MSG(hWnd, WM_INITDIALOG, DlgOnInitDialog);
        HANDLE_MSG(hWnd, WM_COMMAND, DlgOnCommand);
        HANDLE_MSG(hWnd, WM_PAINT, DlgOnPaint);

    case WM_ERASEBKGND:
        return TRUE;

    default:
        return FALSE;
    }
    return TRUE;
}
// [/SystemInfo_DialogProcedure]


// [DlgOnInitDialog]:
static BOOL DlgOnInitDialog(HWND hWnd, HWND, LPARAM)
{
    TCHAR szTemp[1024] = { 0 };
    DWORD dwSize;
    
    memset(szTemp, 0, sizeof(szTemp));
    dwSize = sizeof(szTemp) / sizeof(TCHAR);
    if (GetUserName(szTemp, &dwSize))
    {
        SetWindowText(GetDlgItem(hWnd, 10001), szTemp);
    }

    memset(szTemp, 0, sizeof(szTemp));
    dwSize = sizeof(szTemp) / sizeof(TCHAR);
    if (GetComputerName(szTemp, &dwSize))
    {
        SetWindowText(GetDlgItem(hWnd, 10002), szTemp);
    }
    
    memset(szTemp, 0, sizeof(szTemp));
    char szHostName[1024] = { 0 };
    if (!gethostname((char *)&szHostName, sizeof(szHostName)))
    {
        struct hostent *adr = gethostbyname(szHostName);
        if (adr)
        {
            wsprintf(szTemp, _T("%d.%d.%d.%d"),
                (unsigned char)adr->h_addr_list[0][0],
                (unsigned char)adr->h_addr_list[0][1],
                (unsigned char)adr->h_addr_list[0][2],
                (unsigned char)adr->h_addr_list[0][3]);
            SetWindowText(GetDlgItem(hWnd, 10003), szTemp);
        }
    }
    

    memset(szTemp, 0, sizeof(szTemp));
    wsprintf(szTemp, _T("%d x %d"), GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN));
    SetWindowText(GetDlgItem(hWnd, 10004), szTemp);

    memset(szTemp, 0, sizeof(szTemp));
    HDC hDC = GetDC(NULL);
    if (hDC)
    {
        wsprintf(szTemp, _T("%d"), GetDeviceCaps(hDC, VREFRESH));
        ReleaseDC(NULL, hDC);
        SetWindowText(GetDlgItem(hWnd, 10005), szTemp);
    }

    memset(szTemp, 0, sizeof(szTemp));
    dwSize = sizeof(szTemp) / sizeof(TCHAR);
    if (GetWindowsDirectory(szTemp, dwSize))
    {
        SetWindowText(GetDlgItem(hWnd, 10006), szTemp);
    }

    memset(szTemp, 0, sizeof(szTemp));
    dwSize = sizeof(szTemp) / sizeof(TCHAR);
    if (GetSystemDirectory(szTemp, dwSize))
    {
        SetWindowText(GetDlgItem(hWnd, 10007), szTemp);
    }

    memset(szTemp, 0, sizeof(szTemp));
    dwSize = sizeof(szTemp) / sizeof(TCHAR);
    if (GetTempPath(dwSize, szTemp))
    {
        SetWindowText(GetDlgItem(hWnd, 10008), szTemp);
    }

    return TRUE;
}
// [/DlgOnInitDialog]


// [DlgOnCommand]:
static void DlgOnCommand(HWND hWnd, int id, HWND, UINT)
{
    switch (id)
    {
    case IDOK:
    case IDCANCEL:
        EndDialog(hWnd, 0);
    }
}
// [/DlgOnCommand]


// [DlgOnPaint]: WM_PAINT
static void DlgOnPaint(HWND hWnd)
{
    PAINTSTRUCT ps;
    HDC hDC = BeginPaint(hWnd, &ps);
    FillRect(hDC, &ps.rcPaint, GetStockBrush(LTGRAY_BRUSH));
    EndPaint(hWnd, &ps);
}
// [/DlgOnPaint]
