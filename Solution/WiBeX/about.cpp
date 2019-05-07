#include <tchar.h>
#include <windows.h>
#include <windowsx.h>

#include "about.h"

static BOOL DlgOnInitDialog(HWND, HWND, LPARAM);                            // WM_INITDIALOG
static void DlgOnCommand(HWND, int, HWND, UINT);                            // WM_COMMAND


// [About_DialogProcedure]:
BOOL CALLBACK About_DialogProcedure(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
        HANDLE_MSG(hWnd, WM_INITDIALOG, DlgOnInitDialog);
        HANDLE_MSG(hWnd, WM_COMMAND, DlgOnCommand);

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hDC = BeginPaint(hWnd, &ps);
        FillRect(hDC, &ps.rcPaint, GetStockBrush(LTGRAY_BRUSH));

        int iOldBkMode = SetBkMode(hDC, TRANSPARENT);

        TCHAR szText[1024] = { 0 };
        RECT rc;
        GetClientRect(hWnd, &rc);
        const int iWindowWidth = rc.right - rc.left;
        const int iWindowHeight = rc.bottom - rc.top;

        lstrcpy(szText,
            _T("#-> WiBeX <-#\n")
            _T("Author: fastb1t\n")
            _T("Feedback:\n")
            _T("     Jabber: fastb1t@exploit.im\n")
            _T("     Telegram: @fastb1t\n")
            _T("Compiled: ") __DATE__ _T(" ") __TIME__
        );

        SetRect(&rc, 10, 10, iWindowWidth, iWindowHeight);

        DrawText(hDC, szText, lstrlen(szText), &rc, DT_LEFT);

        SetBkMode(hDC, iOldBkMode);

        EndPaint(hWnd, &ps);
    }
    break;

    case WM_ERASEBKGND:
        return 1;

    default:
        return FALSE;
    }
    return TRUE;
}
// [/About_DialogProcedure]


// [DlgOnInitDialog]:
static BOOL DlgOnInitDialog(HWND hWnd, HWND, LPARAM)
{
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
