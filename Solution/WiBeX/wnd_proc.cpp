#include <tchar.h>
#include <windows.h>
#include <windowsx.h>
#include <richedit.h>
#include <commctrl.h>

#include "wnd_proc.h"
#include "resource.h"
#include "algorithms.h"
#include "DC.h"

static BOOL OnCreate(HWND, LPCREATESTRUCT);                                     	// WM_CREATE
static void OnCommand(HWND, int, HWND, UINT);                                      	// WM_COMMAND
static void OnPaint(HWND);                                                        	// WM_PAINT
static void OnTimer(HWND, UINT);                                                    // WM_TIMER
static void OnDestroy(HWND);                                                        // WM_DESTROY

#define IDT_TIMER1 1000

#define LOG_ERROR 0x01
#define LOG_DEFAULT 0x02

#define IDC_LOG                     20
#define IDC_FIXED_WINDOW            21
#define IDC_CAPTURE_WINDOW_CURSOR   22

static bool capture_window_with_the_cursor = true;

static HFONT hFont;
static HBRUSH hHeadLineBrush;

static DC img_current_window;

static TCHAR szCurrentTime[1024] = { 0 };

static DC img_dot_red;
static DC img_dot_blue;
static DC img_settings;
static DC img_log;
static DC img_minimize;
static DC img_maximize;
static DC img_close;
static DC img_fixed;


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
        hOldBrush = SelectBrush(hDC, hBrush);

    Rectangle(hDC, x, y, x + iWidth + 1, y + iHeight + 1);

    if (hOldBrush)
        SelectBrush(hDC, hOldBrush);

    if (img->HasImage())
        img->Draw(hDC, x + 2, y + 2);

    COLORREF clrOldColor = SetTextColor(hDC, 0x00);
    TextOut(hDC, x + 20, y + (iHeight >> 1) - (size.cy >> 1), szText, lstrlen(szText));
    SetTextColor(hDC, clrOldColor);
}
// [/DrawHeadlineForPart]


// [SelectWindow]:
static bool SelectWindow(HWND hMyWnd, HWND hWnd)
{
    // TODO: Доробити: якщо інформація про вікно не змінилася, то не оновлюємо.
    if (hWnd &&
        hWnd != hMyWnd &&
        hMyWnd != GetParent(hWnd))
    {
        //ClearProccessInfo(hWnd);

        //if (GetWindowLongPtr(hWnd, GWL_EXSTYLE) & WS_EX_TOPMOST)
        //    SendMessage(GetDlgItem(hMyWnd, IDC_CLIENT_FIXED_WINDOW), BM_SETCHECK, TRUE, 0L);
        //else
        //    SendMessage(GetDlgItem(hMyWnd, IDC_CLIENT_FIXED_WINDOW), BM_SETCHECK, FALSE, 0L);

        //ScreenShot(&window_image, hWnd);
        img_current_window.MakeScreenShot(hWnd);
        img_current_window.TransformImage(400, 200);

        //Process tmp_proc(process);

        //process.setWindow(hWnd);

        //if (process.getClassName() != tmp_proc.getClassName())
        //    SetWindowText(GetDlgItem(hMyWnd, IDC_CLIENT_CLASS_NAME), process.getClassName().c_str());

        //if (process.getTitle() != tmp_proc.getTitle())
        //    SetWindowText(GetDlgItem(hMyWnd, IDC_CLIENT_TITLE), process.getTitle().c_str());

        //if (process.getExeName() != tmp_proc.getExeName())
        //    SetWindowText(GetDlgItem(hMyWnd, IDC_CLIENT_PROCESS_NAME), process.getExeName().c_str());

        //if (process.getPID() != tmp_proc.getPID())
        //    SetWindowText(GetDlgItem(hMyWnd, IDC_CLIENT_PROCESS_PID), process.getPID().c_str());

        //if (process.getFullPath() != tmp_proc.getFullPath())
        //    SetWindowText(GetDlgItem(hMyWnd, IDC_CLIENT_PATH), process.getFullPath().c_str());

        //if (process.getExecuteTime() != tmp_proc.getExecuteTime())
        //    SetWindowText(GetDlgItem(hMyWnd, IDC_CLIENT_EXECUTE_TIME), process.getExecuteTime().c_str());

        //if (process.GetXPos() != tmp_proc.GetXPos())
        //    SetWindowText(GetDlgItem(hMyWnd, IDC_CLIENT_X), process.GetXPos().c_str());
        //if (process.GetYPos() != tmp_proc.GetYPos())
        //    SetWindowText(GetDlgItem(hMyWnd, IDC_CLIENT_Y), process.GetYPos().c_str());
        //if (process.GetWidth() != tmp_proc.GetWidth())
        //    SetWindowText(GetDlgItem(hMyWnd, IDC_CLIENT_SIZE_X), process.GetWidth().c_str());
        //if (process.GetHeight() != tmp_proc.GetHeight())
        //    SetWindowText(GetDlgItem(hMyWnd, IDC_CLIENT_SIZE_Y), process.GetHeight().c_str());

        return true;
    }
    return false;
}
// [/SelectWindow]


// [Thread]:
static DWORD WINAPI Thread(LPVOID lpObject)
{
    HWND hWnd = (HWND)lpObject;
    RECT rc;
    GetClientRect(hWnd, &rc);
    const int window_width = rc.right - rc.left;
    const int window_height = rc.bottom - rc.top;

    while (IsWindow(hWnd))
    {
        //if (!IsWindow(process.getWindow()))
        //    ClearProccessInfo(hWnd);

        if (capture_window_with_the_cursor)
            SelectWindow(hWnd, GetForegroundWindow());
        //else
        //    SelectWindow(hWnd, process.getWindow());

        SetRect(&rc, 0, 0, 420 + 7, 220 + 29);
        InvalidateRect(hWnd, &rc, FALSE);

        Sleep(1000);
    }
    ExitThread(1337);
}
// [/Thread]


// [AddTextToLog]:
bool AddTextToLog(HWND hParentWnd, String str, int mode)
{
    if (!hParentWnd)
        return false;

    HWND hLogWnd = GetDlgItem(hParentWnd, IDC_LOG);
    if (!hLogWnd)
        return false;

    CHARFORMAT cf;
    RtlZeroMemory(&cf, sizeof(CHARFORMAT));
    SendMessage(hLogWnd, EM_GETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);

    cf.cbSize = sizeof(cf);
    cf.dwMask = CFM_COLOR;
    cf.dwEffects = 0;
    lstrcpy(cf.szFaceName, _T("Times New Roman"));

    if (mode == LOG_ERROR)
        cf.crTextColor = RGB(230, 20, 20);
    else
        cf.crTextColor = RGB(30, 30, 30);

    TCHAR szTime[128] = { 0 };

    SYSTEMTIME st;
    GetLocalTime(&st);
    wsprintf(szTime, _T("[%s%d.%s%d.%d %s%d:%s%d:%s%d]: "),
        st.wDay <= 9 ? _T("0") : _T(""), st.wDay,
        st.wMonth <= 9 ? _T("0") : _T(""), st.wMonth,
        st.wYear,
        st.wHour <= 9 ? _T("0") : _T(""), st.wHour,
        st.wMinute <= 9 ? _T("0") : _T(""), st.wMinute,
        st.wSecond <= 9 ? _T("0") : _T(""), st.wSecond);

    String out = szTime + str + _T("\n");

    SendMessage(hLogWnd, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
    SendMessage(hLogWnd, EM_REPLACESEL, 0, (LPARAM)out.c_str());
    SendMessage(hLogWnd, WM_VSCROLL, SB_BOTTOM, 0L);

    return true;
}
// [/AddTextToLog]


static WNDPROC oldRichEditProcedure = NULL;


// [NewRichEditProcedure]:
static LRESULT CALLBACK NewRichEditProcedure(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_RBUTTONDOWN:
    {
        POINT pt;
        pt.x = GET_X_LPARAM(lParam);
        pt.y = GET_Y_LPARAM(lParam);

        ClientToScreen(hWnd, &pt);

        HMENU hFloatMenu = CreatePopupMenu();

        TCHAR szText[128] = { 0 };
        lstrcpy(szText, _T("Copy"));

        MENUITEMINFO mii;
        RtlZeroMemory(&mii, sizeof(MENUITEMINFO));
        mii.cbSize = sizeof(MENUITEMINFO);
        mii.fMask = MIIM_STATE | MIIM_TYPE | MIIM_SUBMENU | MIIM_ID;
        mii.fType = MFT_STRING;
        mii.fState = MFS_ENABLED;
        mii.dwTypeData = szText;
        mii.cch = lstrlen(szText);
        mii.wID = 1000;
        mii.hSubMenu = NULL;
        InsertMenuItem(hFloatMenu, 0, 0, &mii);

        TrackPopupMenu(hFloatMenu, TPM_CENTERALIGN | TPM_LEFTBUTTON | TPM_VCENTERALIGN, pt.x, pt.y, 0, hWnd, NULL);
        DestroyMenu(hFloatMenu);
        return 0;
    }

    case WM_COMMAND:
    {
        if (LOWORD(wParam) == 1000)
        {
            SendMessage(hWnd, WM_COPY, 0, 0L);
            return 0;
        }
    }

    default:
        break;
    }
    return CallWindowProc(oldRichEditProcedure, hWnd, msg, wParam, lParam);
}
// [/NewRichEditProcedure]


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

    case WM_ERASEBKGND:
        return 1;

    case WM_CTLCOLORSTATIC:
    {
        HWND hTmpWnd = (HWND)lParam;
        SetBkColor((HDC)wParam, RGB(192, 192, 192));
        return (LRESULT)GetStockBrush(NULL_BRUSH);
    }
    break;

    //case WM_NOTIFY:
    //{
    //    const MSGFILTER * pF = (MSGFILTER *)lParam;
    //    if (pF->nmhdr.hwndFrom == GetDlgItem(hWnd, IDC_LOG))
    //    {
    //        if (pF->msg == WM_RBUTTONDOWN)
    //        {
    //            //SetWindowText(hWnd, "right  button  clicked");
    //            //SendMessage(pF->nmhdr.hwndFrom, WM_COPY, 0, 0L);
    //        }
    //    }
    //}
    //break;
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

    hHeadLineBrush = CreateSolidBrush(RGB(140, 140, 140));

    hFont = CreateFont(-12, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, _T("Arial"));


    const int xcenter = iWindowWidth >> 1;

    HWND hLogWnd = CreateWindowEx(WS_EX_STATICEDGE, RICHEDIT_CLASS, _T(""),
        WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_TABSTOP | ES_SAVESEL | ES_NOHIDESEL | ES_MULTILINE | ES_READONLY,
        7, iWindowHeight - 160 + 33, xcenter - 7 - 7, 120, hWnd, (HMENU)IDC_LOG, lpcs->hInstance, 0);

    if (hLogWnd)
    {
        SendMessage(hLogWnd, EM_SETBKGNDCOLOR, 0, RGB(192, 192, 192));
        //SendMessage(hLogWnd, EM_SETEVENTMASK, (WPARAM)0, (LPARAM)ENM_MOUSEEVENTS); // Для сповіщення про нажаття клавіш мишки. See WM_NOTIFY.
        SendMessage(hLogWnd, WM_SETFONT, (WPARAM)hFont, 0L);

        oldRichEditProcedure = (WNDPROC) SetWindowLongPtr(hLogWnd, GWL_WNDPROC, (LONG_PTR)NewRichEditProcedure);

        AddTextToLog(hWnd, _T("line 1"), LOG_DEFAULT);
        AddTextToLog(hWnd, _T("line 2"), LOG_DEFAULT);
        AddTextToLog(hWnd, _T("line 3"), LOG_DEFAULT);
        AddTextToLog(hWnd, _T("line 4"), LOG_DEFAULT);
        AddTextToLog(hWnd, _T("line 1"), LOG_ERROR);
    }


    CreateWindowEx(0, _T("button"), _T("Закріпити вікно поверх всіх вікон"),
        WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX, xcenter + 9, iWindowHeight - 160 + 33, xcenter - 7 - 7 - 1, 20,
        hWnd, (HMENU)IDC_FIXED_WINDOW, NULL, NULL);
    SendMessage(GetDlgItem(hWnd, IDC_FIXED_WINDOW), WM_SETFONT, (WPARAM)hFont, 0L);
    SendMessage(GetDlgItem(hWnd, IDC_FIXED_WINDOW), BM_SETCHECK, true, 0L);

    CreateWindowEx(0, _T("button"), _T("Захоплення вікна курсором"),
        WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX, xcenter + 9, iWindowHeight - 160 + 33 + 20, xcenter - 7 - 7 - 1, 20,
        hWnd, (HMENU)IDC_CAPTURE_WINDOW_CURSOR, NULL, NULL);
    SendMessage(GetDlgItem(hWnd, IDC_CAPTURE_WINDOW_CURSOR), WM_SETFONT, (WPARAM)hFont, 0L);
    SendMessage(GetDlgItem(hWnd, IDC_CAPTURE_WINDOW_CURSOR), BM_SETCHECK, capture_window_with_the_cursor, 0L);


    SendMessage(hWnd, WM_TIMER, (WPARAM)IDT_TIMER1, 0L);
    SetTimer(hWnd, IDT_TIMER1, 1000, NULL);

    CreateThread(NULL, NULL, Thread, (LPVOID)hWnd, 0, NULL);

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
        MessageBox(hWnd,
            _T("#-> WiBeX <-#\n")
            _T("Author: fastb1t\n")
            _T("Feedback:\n")
            _T("     Jabber: fastb1t@exploit.im\n")
            _T("     Telegram: @fastb1t\n")
            _T("Compiled: ") __DATE__ _T(" ") __TIME__,
            _T("Information"), MB_OK | MB_ICONINFORMATION);
    }
    break;

    case IDC_FIXED_WINDOW:
    {
        if (SendMessage(GetDlgItem(hWnd, IDC_FIXED_WINDOW), BM_GETCHECK, 0, 0L))
            SetWindowPos(hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
        else
            SetWindowPos(hWnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
    }
    break;

    case IDC_CAPTURE_WINDOW_CURSOR:
    {
        capture_window_with_the_cursor = SendMessage(GetDlgItem(hWnd, IDC_CAPTURE_WINDOW_CURSOR), BM_GETCHECK, 0, 0L) ? true : false;
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

    SelectFont(hMemDC, hFont);

    DrawEmptyRectangle(hMemDC, 5, 5, iWindowWidth - 5, iWindowHeight - 155);
    if (!img_current_window.HasImage())
    {
        lstrcpy(szText, _T("Window"));
        DrawHeadlineForPart(hMemDC, 7, 7, iWindowWidth - 7 - 7, szText, hHeadLineBrush, &img_dot_red);
    }
    else
    {
        size = img_current_window.GetOriginalSize();
        wsprintf(szText, _T("Window (%d x %d)"), size.cx, size.cy);
        DrawHeadlineForPart(hMemDC, 7, 7, iWindowWidth - 7 - 7, szText, hHeadLineBrush, &img_dot_blue);
    }

    const int xcenter = iWindowWidth >> 1;

    DrawEmptyRectangle(hMemDC, 5, iWindowHeight - 151, xcenter - 5, iWindowHeight - 5);
    DrawHeadlineForPart(hMemDC, 7, iWindowHeight - 151 + 2, xcenter - 7 - 7, _T("Log"), hHeadLineBrush, &img_log);

    GetTextExtentPoint32(hMemDC, szCurrentTime, lstrlen(szCurrentTime), &size);
    TextOut(hMemDC, xcenter - size.cx - 10, iWindowHeight - 151 + 4, szCurrentTime, lstrlen(szCurrentTime));

    DrawEmptyRectangle(hMemDC, xcenter + 5, iWindowHeight - 151, iWindowWidth - 5, iWindowHeight - 5);
    DrawHeadlineForPart(hMemDC, xcenter + 7, iWindowHeight - 151 + 2, xcenter - 7 - 6, _T("Settings"), hHeadLineBrush, &img_settings);


    DrawEmptyRectangle(hMemDC, 7, 29, 420 + 7, 220 + 27);
    if (img_current_window.HasImage())
    {
        size = img_current_window.GetCurrentSize();
        int x = 200 - (size.cx >> 1);
        int y = 100 - (size.cy >> 1);
        img_current_window.Draw(hMemDC, 17 + x, 39 + y);
    }
    else
    {
        lstrcpy(szText, _T("Тут буде зображення захопленого вікна..."));
        GetTextExtentPoint32(hMemDC, szText, lstrlen(szText), &size);
        TextOut(hMemDC, 8 + 210 - (size.cx >> 1), 30 + 110 - size.cy, szText, lstrlen(szText));
    }


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
    case IDT_TIMER1:
    {
        SYSTEMTIME st;
        GetLocalTime(&st);
        wsprintf(szCurrentTime, _T("%s%d.%s%d.%d %s%d:%s%d:%s%d"),
            st.wDay <= 9 ? _T("0") : _T(""), st.wDay,
            st.wMonth <= 9 ? _T("0") : _T(""), st.wMonth,
            st.wYear,
            st.wHour <= 9 ? _T("0") : _T(""), st.wHour,
            st.wMinute <= 9 ? _T("0") : _T(""), st.wMinute,
            st.wSecond <= 9 ? _T("0") : _T(""), st.wSecond
        );
        SetRect(&rc, 0, iWindowHeight - 156 + 2, (iWindowWidth >> 1), iWindowHeight - 156 + 2 + 20);
        InvalidateRect(hWnd, &rc, FALSE);
    }
    break;
    }
}
// [/OnTimer]


// [OnDestroy]: WM_DESTROY
static void OnDestroy(HWND hWnd)
{
    img_current_window.Clear();

    img_dot_red.Clear();
    img_dot_blue.Clear();
    img_settings.Clear();
    img_log.Clear();
    img_minimize.Clear();
    img_maximize.Clear();
    img_close.Clear();
    img_fixed.Clear();

    DeleteBrush(hHeadLineBrush);
    DeleteFont(hFont);

    KillTimer(hWnd, IDT_TIMER1);
    PostQuitMessage(0);
}
// [/OnDestroy]
