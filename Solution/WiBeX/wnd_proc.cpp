#include <tchar.h>
#include <windows.h>
#include <windowsx.h>

#include "wnd_proc.h"
#include "resource.h"
#include "algorithms.h"
#include "DC.h"

static BOOL OnCreate(HWND, LPCREATESTRUCT);                                     	// WM_CREATE
static void OnCommand(HWND, int, HWND, UINT);                                      	// WM_COMMAND
static void OnPaint(HWND);                                                        	// WM_PAINT
static void OnTimer(HWND, UINT);                                                    // WM_TIMER
static void OnDestroy(HWND);                                                        // WM_DESTROY

static DC img_dot_red;
static DC img_dot_blue;
static DC img_settings;
static DC img_log;
static DC img_minimize;
static DC img_maximize;
static DC img_close;
static DC img_fixed;


// [WindowProcedure]: 
LRESULT CALLBACK WindowProcedure(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
        HANDLE_MSG(hWnd, WM_CREATE, OnCreate);
        HANDLE_MSG(hWnd, WM_COMMAND, OnCommand);
        HANDLE_MSG(hWnd, WM_PAINT, OnPaint);
        HANDLE_MSG(hWnd, WM_TIMER, OnTimer);
        HANDLE_MSG(hWnd, WM_DESTROY, OnDestroy);
    }
    return DefWindowProc(hWnd, msg, wParam, lParam);
}
// [/WindowProcedure]


// [OnCreate]: WM_CREATE
static BOOL OnCreate(HWND hWnd, LPCREATESTRUCT lpcs)
{
    RECT rc;
    GetClientRect(hWnd, &rc);
    const int iWindowWidth = rc.right - rc.left;
    const int iWindowHeight = rc.bottom - rc.top;

    img_dot_red.CreateFromBitmap(LoadBitmap(lpcs->hInstance, MAKEINTRESOURCE(IDB_DOT_RED)));
    img_dot_blue.CreateFromBitmap(LoadBitmap(lpcs->hInstance, MAKEINTRESOURCE(IDB_DOT_BLUE)));
    img_settings.CreateFromBitmap(LoadBitmap(lpcs->hInstance, MAKEINTRESOURCE(IDB_SETTINGS)));
    img_log.CreateFromBitmap(LoadBitmap(lpcs->hInstance, MAKEINTRESOURCE(IDB_LOG)));
    img_minimize.CreateFromBitmap(LoadBitmap(lpcs->hInstance, MAKEINTRESOURCE(IDB_MINIMIZE)));
    img_maximize.CreateFromBitmap(LoadBitmap(lpcs->hInstance, MAKEINTRESOURCE(IDB_MAXIMIZE)));
    img_close.CreateFromBitmap(LoadBitmap(lpcs->hInstance, MAKEINTRESOURCE(IDB_CLOSE)));
    img_fixed.CreateFromBitmap(LoadBitmap(lpcs->hInstance, MAKEINTRESOURCE(IDB_FIXED)));

    img_dot_red.ReplaceColors(RGB(255, 255, 255), RGB(140, 140, 140));
    img_dot_blue.ReplaceColors(RGB(255, 255, 255), RGB(140, 140, 140));
    img_settings.ReplaceColors(RGB(255, 255, 255), RGB(140, 140, 140));
    img_log.ReplaceColors(RGB(255, 255, 255), RGB(140, 140, 140));
    img_minimize.ReplaceColors(RGB(255, 255, 255), RGB(140, 140, 140));
    img_maximize.ReplaceColors(RGB(255, 255, 255), RGB(140, 140, 140));
    img_close.ReplaceColors(RGB(255, 255, 255), RGB(140, 140, 140));
    img_fixed.ReplaceColors(RGB(255, 255, 255), RGB(140, 140, 140));

    return TRUE;
}
// [/OnCreate]


// [OnCommand]: WM_COMMAND
static void OnCommand(HWND hWnd, int id, HWND hWndCtl, UINT codeNotify)
{
    switch (id)
    {
    case IDC_EXIT:
    {
        DestroyWindow(hWnd);
    }
    break;

    case IDC_ABOUT:
    {
        MessageBox(hWnd, _T("Author: fastb1t"), _T("Information"), MB_OK | MB_ICONINFORMATION);
    }
    break;
    }
}
// [/OnCommand]


// [OnPaint]: WM_PAINT
static void OnPaint(HWND hWnd)
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

    FillRect(hMemDC, &ps.rcPaint, (HBRUSH)GetStockObject(LTGRAY_BRUSH));

    int iOldBkMode = SetBkMode(hMemDC, TRANSPARENT);
    COLORREF clrOldColor = GetTextColor(hMemDC);
    HPEN hOldPen = (HPEN)GetCurrentObject(hMemDC, OBJ_PEN);
    HBRUSH hOldBrush = (HBRUSH)GetCurrentObject(hMemDC, OBJ_BRUSH);
    HFONT hOldFont = (HFONT)GetCurrentObject(hMemDC, OBJ_FONT);

    TCHAR szText[256] = { 0 };
    SIZE size;

    img_dot_red.Draw(hMemDC, 10, 10);
    img_dot_blue.Draw(hMemDC, 30, 10);
    img_settings.Draw(hMemDC, 50, 10);
    img_log.Draw(hMemDC, 70, 10);
    img_minimize.Draw(hMemDC, 90, 10);
    img_maximize.Draw(hMemDC, 110, 10);
    img_close.Draw(hMemDC, 130, 10);
    img_fixed.Draw(hMemDC, 150, 10);

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
// [/OnPaint]


// [OnTimer]: WM_TIMER
static void OnTimer(HWND hWnd, UINT id)
{
    RECT rc;
    GetClientRect(hWnd, &rc);
    const int iWindowWidth = rc.right - rc.left;
    const int iWindowHeight = rc.bottom - rc.top;

    switch (id)
    {
    case -1:
        break;
    }
}
// [/OnTimer]


// [OnDestroy]: WM_DESTROY
static void OnDestroy(HWND hWnd)
{
    img_dot_red.Clear();
    img_dot_blue.Clear();
    img_settings.Clear();
    img_log.Clear();
    img_minimize.Clear();
    img_maximize.Clear();
    img_close.Clear();
    img_fixed.Clear();
    PostQuitMessage(0);
}
// [/OnDestroy]
