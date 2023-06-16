#include <tchar.h>
#include <windows.h>
#include <windowsx.h>

#include "about.h"
#include "resource.h"
#include "algorithms.h"

static BOOL DlgOnInitDialog(HWND, HWND, LPARAM);                            // WM_INITDIALOG
static void DlgOnCommand(HWND, int, HWND, UINT);                            // WM_COMMAND
static void DlgOnPaint(HWND);                                               // WM_PAINT

static HBRUSH g_hBackgroundBrush = NULL;

static HFONT g_hCaptionFont = NULL;
static HFONT g_hTitleFont = NULL;
static HFONT g_hDefaultFont = NULL;


// [About_DialogProcedure]:
BOOL CALLBACK About_DialogProcedure(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
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
// [/About_DialogProcedure]


// [DlgOnInitDialog]:
static BOOL DlgOnInitDialog(HWND hWnd, HWND, LPARAM)
{
    HINSTANCE hInstance = (HINSTANCE)GetWindowLongPtr(GetParent(hWnd), GWL_HINSTANCE);

    SetClassLongPtr(hWnd, GCL_HICON, (LONG)LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON)));
    SetClassLongPtr(hWnd, GCL_HICONSM, (LONG)LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON)));

    RECT rc;

    GetWindowRect(GetParent(hWnd), &rc);
    const int iParentWindowX = rc.left;
    const int iParentWindowY = rc.top;
    const int iParentWindowWidth = rc.right - rc.left;
    const int iParentWindowHeight = rc.bottom - rc.top;

    SetRect(&rc, 0, 0, 330, 200);
    AdjustWindowRectEx(
        &rc,
        GetWindowLongPtr(hWnd, GWL_STYLE),
        FALSE,
        GetWindowLongPtr(hWnd, GWL_EXSTYLE)
    );

    MoveWindow(
        hWnd,
        iParentWindowX + (iParentWindowWidth >> 1) - ((rc.right - rc.left) >> 1),
        iParentWindowY + (iParentWindowHeight >> 1) - ((rc.bottom - rc.top) >> 1),
        rc.right - rc.left,
        rc.bottom - rc.top,
        TRUE
    );

    GetClientRect(hWnd, &rc);
    const int iWindowWidth = rc.right - rc.left;
    const int iWindowHeight = rc.bottom - rc.top;


    g_hBackgroundBrush = CreateSolidBrush(RGB(150, 180, 200));

    g_hCaptionFont = CreateFont(17, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, _T("Arial"));

    g_hTitleFont = CreateFont(15, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, _T("Arial"));

    g_hDefaultFont = CreateFont(15, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, _T("Arial"));

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
    {
        DeleteObject(g_hBackgroundBrush);
        
        DeleteObject(g_hCaptionFont);
        DeleteObject(g_hTitleFont);
        DeleteObject(g_hDefaultFont);

        EndDialog(hWnd, 0);
    }
    }
}
// [/DlgOnCommand]


// [DlgOnPaint]: WM_PAINT
static void DlgOnPaint(HWND hWnd)
{
    PAINTSTRUCT ps;
    HDC hDC = BeginPaint(hWnd, &ps);
    RECT rc;
    GetClientRect(hWnd, &rc);
    const int iWindowWidth = rc.right - rc.left;
    const int iWindowHeight = rc.bottom - rc.top;
    HDC hMemDC = CreateCompatibleDC(hDC);
    HBITMAP hBitmap = CreateCompatibleBitmap(hDC, iWindowWidth, iWindowHeight);
    HBITMAP hOldBitmap = (HBITMAP)SelectObject(hMemDC, hBitmap);

    FillRect(hMemDC, &ps.rcPaint, g_hBackgroundBrush);// (HBRUSH)GetStockObject(LTGRAY_BRUSH));

    int iOldBkMode = SetBkMode(hMemDC, TRANSPARENT);
    COLORREF clrOldColor = GetTextColor(hMemDC);
    HPEN hOldPen = (HPEN)GetCurrentObject(hMemDC, OBJ_PEN);
    HBRUSH hOldBrush = (HBRUSH)GetCurrentObject(hMemDC, OBJ_BRUSH);
    HFONT hOldFont = (HFONT)GetCurrentObject(hMemDC, OBJ_FONT);

    TCHAR szText[256] = { 0 };
    SIZE size;


    SelectObject(hMemDC, g_hCaptionFont);
    lstrcpy(szText, _T("WiBeX"));

    GetTextExtentPoint32(hMemDC, szText, lstrlen(szText), &size);
    TextOut(hMemDC, (iWindowWidth >> 1) - (size.cx >> 1), 10, szText, lstrlen(szText));


    SelectObject(hMemDC, g_hDefaultFont);
    lstrcpy(szText, _T("Window repair master"));

    GetTextExtentPoint32(hMemDC, szText, lstrlen(szText), &size);
    TextOut(hMemDC, (iWindowWidth >> 1) - (size.cx >> 1), 30, szText, lstrlen(szText));


    DrawLine(hMemDC, 3, 50, iWindowWidth - 3, 50);


    SelectObject(hMemDC, g_hTitleFont);
    lstrcpy(szText, _T("Author:"));
    TextOut(hMemDC, 10, 60, szText, lstrlen(szText));

    SelectObject(hMemDC, g_hDefaultFont);
    lstrcpy(szText, _T("fastb1t"));
    TextOut(hMemDC, 100, 60, szText, lstrlen(szText));


    SelectObject(hMemDC, g_hTitleFont);
    lstrcpy(szText, _T("URL:"));
    TextOut(hMemDC, 10, 80, szText, lstrlen(szText));

    SelectObject(hMemDC, g_hDefaultFont);
    lstrcpy(szText, _T("https://github.com/fastb1t/WiBeX"));
    TextOut(hMemDC, 100, 80, szText, lstrlen(szText));


    SelectObject(hMemDC, g_hTitleFont);
    lstrcpy(szText, _T("Contacts:"));
    TextOut(hMemDC, 10, 100, szText, lstrlen(szText));

    SelectObject(hMemDC, g_hTitleFont);
    lstrcpy(szText, _T("E-mail:"));
    TextOut(hMemDC, 30, 120, szText, lstrlen(szText));

    SelectObject(hMemDC, g_hDefaultFont);
    lstrcpy(szText, _T("freelance.programmer.000@gmail.com"));
    TextOut(hMemDC, 100, 120, szText, lstrlen(szText));

    SelectObject(hMemDC, g_hTitleFont);
    lstrcpy(szText, _T("Telegram:"));
    TextOut(hMemDC, 30, 140, szText, lstrlen(szText));

    SelectObject(hMemDC, g_hDefaultFont);
    lstrcpy(szText, _T("@fastb1t"));
    TextOut(hMemDC, 100, 140, szText, lstrlen(szText));


    SelectObject(hMemDC, g_hTitleFont);
    lstrcpy(szText, _T("Compiled:"));
    TextOut(hMemDC, 10, 160, szText, lstrlen(szText));

    SelectObject(hMemDC, g_hDefaultFont);
    lstrcpy(szText, (__DATE__ _T(" ") __TIME__));
    TextOut(hMemDC, 100, 160, szText, lstrlen(szText));

    
    SelectObject(hMemDC, hOldFont);
    SelectObject(hMemDC, hOldBrush);
    SelectObject(hMemDC, hOldPen);
    SetTextColor(hMemDC, clrOldColor);
    SetBkMode(hMemDC, iOldBkMode);

    BitBlt(hDC, 0, 0, iWindowWidth, iWindowHeight, hMemDC, 0, 0, SRCCOPY);
    SelectObject(hMemDC, hOldBitmap);
    DeleteObject(hBitmap);
    DeleteDC(hMemDC);
    EndPaint(hWnd, &ps);
}
// [/DlgOnPaint]
