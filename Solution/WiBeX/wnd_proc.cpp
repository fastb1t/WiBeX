#include <tchar.h>
#include <windows.h>
#include <windowsx.h>
#include <richedit.h>
#include <commctrl.h>

#include "windows_viewer.h"
#include "dependency_list.h"
#include "system_info.h"
#include "about.h"

#include "wnd_proc.h"
#include "resource.h"
#include "algorithms.h"
#include "DC.h"
#include "Process.h"


static BOOL OnCreate(HWND, LPCREATESTRUCT);                                     	// WM_CREATE
static void OnCommand(HWND, int, HWND, UINT);                                      	// WM_COMMAND
static void OnPaint(HWND);                                                        	// WM_PAINT
static void OnDrawItem(HWND, const DRAWITEMSTRUCT *);                               // WM_DRAWITEM
static void OnDestroy(HWND);                                                        // WM_DESTROY

#define LOG_ERROR                       0x01
#define LOG_DEFAULT                     0x02

#define IDC_LOG                         20
#define IDC_FIXED_WINDOW                21
#define IDC_CAPTURE_WINDOW_WITH_CURSOR  22

#define IDC_CLIENT_CLASS_NAME           100
#define IDC_CLIENT_TITLE                101
#define IDC_CLIENT_PROCESS_NAME         102
#define IDC_CLIENT_PROCESS_PID          103
#define IDC_CLIENT_PATH                 104
#define IDC_CLIENT_EXECUTE_TIME         105

#define IDC_CLIENT_MINIMIZE             200
#define IDC_CLIENT_MAXIMIZE             201
#define IDC_CLIENT_CLOSE                202
#define IDC_CLIENT_FIXED_WINDOW         203

#define IDC_CLIENT_X                    204
#define IDC_CLIENT_Y                    205
#define IDC_CLIENT_SIZE_X               206
#define IDC_CLIENT_SIZE_Y               207

#define IDC_CLIENT_MOVE_WINDOW          208
#define IDC_CLIENT_SET_SIZE             209

#define IDC_TERMINATE_PROCESS           210

#define IDC_CLEAR_MEMORY                211

#define IDC_INJECTOR_DLL                212
#define IDC_INJECTOR_BROWSE_DLL         213
#define IDC_INJECTOR_RUN                214

UINT WM_WINDOW_SELECTED = 0;

HFONT hFont;
static HBRUSH hHeadLineBrush;

Process process;
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

static WNDPROC oldRichEditProcedure = NULL;


// [SelectWindow]:
static bool SelectWindow(HWND hMyWnd, HWND hWnd)
{
    if (hWnd &&
        hWnd != hMyWnd &&
        hMyWnd != GetParent(hWnd))
    {
        if (GetWindowLongPtr(hWnd, GWL_EXSTYLE) & WS_EX_TOPMOST)
            SendMessage(GetDlgItem(hMyWnd, IDC_CLIENT_FIXED_WINDOW), BM_SETCHECK, TRUE, 0L);
        else
            SendMessage(GetDlgItem(hMyWnd, IDC_CLIENT_FIXED_WINDOW), BM_SETCHECK, FALSE, 0L);

        img_current_window.MakeScreenShot(hWnd);
        img_current_window.TransformImage(400, 200);
		
        Process tmp_proc(process);

        process.setWindow(hWnd);

        if (process.getClassName() != tmp_proc.getClassName())
            SetWindowText(GetDlgItem(hMyWnd, IDC_CLIENT_CLASS_NAME), process.getClassName().c_str());

        if (process.getTitle() != tmp_proc.getTitle())
            SetWindowText(GetDlgItem(hMyWnd, IDC_CLIENT_TITLE), process.getTitle().c_str());

        if (process.getExeName() != tmp_proc.getExeName())
            SetWindowText(GetDlgItem(hMyWnd, IDC_CLIENT_PROCESS_NAME), process.getExeName().c_str());

        if (process.getPID() != tmp_proc.getPID())
            SetWindowText(GetDlgItem(hMyWnd, IDC_CLIENT_PROCESS_PID), process.getPID().c_str());

        if (process.getFullPath() != tmp_proc.getFullPath())
            SetWindowText(GetDlgItem(hMyWnd, IDC_CLIENT_PATH), process.getFullPath().c_str());

        if (process.getExecuteTime() != tmp_proc.getExecuteTime())
            SetWindowText(GetDlgItem(hMyWnd, IDC_CLIENT_EXECUTE_TIME), process.getExecuteTime().c_str());

        if (process.getXPos() != tmp_proc.getXPos())
            SetWindowText(GetDlgItem(hMyWnd, IDC_CLIENT_X), process.getXPos().c_str());

        if (process.getYPos() != tmp_proc.getYPos())
            SetWindowText(GetDlgItem(hMyWnd, IDC_CLIENT_Y), process.getYPos().c_str());

        if (process.getWidth() != tmp_proc.getWidth())
            SetWindowText(GetDlgItem(hMyWnd, IDC_CLIENT_SIZE_X), process.getWidth().c_str());

        if (process.getHeight() != tmp_proc.getHeight())
            SetWindowText(GetDlgItem(hMyWnd, IDC_CLIENT_SIZE_Y), process.getHeight().c_str());

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
    const int iWindowWidth = rc.right - rc.left;
    const int iWindowHeight = rc.bottom - rc.top;

    while (IsWindow(hWnd))
    {
        if (!IsWindow(process.getWindow()))
        {
            img_current_window.Clear();

            SetWindowText(GetDlgItem(hWnd, IDC_CLIENT_CLASS_NAME), _T("\0"));
            SetWindowText(GetDlgItem(hWnd, IDC_CLIENT_TITLE), _T("\0"));
            SetWindowText(GetDlgItem(hWnd, IDC_CLIENT_PROCESS_NAME), _T("\0"));
            SetWindowText(GetDlgItem(hWnd, IDC_CLIENT_PROCESS_PID), _T("\0"));
            SetWindowText(GetDlgItem(hWnd, IDC_CLIENT_PATH), _T("\0"));
            SetWindowText(GetDlgItem(hWnd, IDC_CLIENT_EXECUTE_TIME), _T("\0"));

            SetWindowText(GetDlgItem(hWnd, IDC_CLIENT_X), _T("0"));
            SetWindowText(GetDlgItem(hWnd, IDC_CLIENT_Y), _T("0"));
            SetWindowText(GetDlgItem(hWnd, IDC_CLIENT_SIZE_X), _T("0"));
            SetWindowText(GetDlgItem(hWnd, IDC_CLIENT_SIZE_Y), _T("0"));

            SendMessage(GetDlgItem(hWnd, IDC_CLIENT_FIXED_WINDOW), BM_SETCHECK, 0, 0L);
        }

        if (SendMessage(GetDlgItem(hWnd, IDC_CAPTURE_WINDOW_WITH_CURSOR), BM_GETCHECK, 0, 0L))
            SelectWindow(hWnd, GetForegroundWindow());
        else
            SelectWindow(hWnd, process.getWindow());

        SetRect(&rc, 0, 0, 420 + 7, 220 + 29);
        InvalidateRect(hWnd, &rc, FALSE);


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
        cf.crTextColor = RGB(0, 0, 0);

    TCHAR szTime[128] = { 0 };
    /*
    TCHAR szDate[128] = { 0 };
    GetTimeFormat(LOCALE_USER_DEFAULT, 0, NULL, NULL, szTime, _countof(szTime));
    GetDateFormat(LOCALE_USER_DEFAULT, 0, NULL, NULL, szDate, _countof(szDate));
    String out = _T("[");
    out += szDate;
    out += _T(" ");
    out += szTime;
    out += _T("] ") + str + _T("\n");
    //*/
    //*
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
    //*/
    SendMessage(hLogWnd, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
    SendMessage(hLogWnd, EM_REPLACESEL, 0, (LPARAM)out.c_str());
    SendMessage(hLogWnd, WM_VSCROLL, SB_BOTTOM, 0L);

    return true;
}
// [/AddTextToLog]


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

    case WM_ERASEBKGND:
        return 0;

    default:
        break;
    }
    return CallWindowProc(oldRichEditProcedure, hWnd, msg, wParam, lParam);
}
// [/NewRichEditProcedure]


// [WindowProcedure]: 
LRESULT CALLBACK WindowProcedure(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (WM_WINDOW_SELECTED != 0 && msg == WM_WINDOW_SELECTED)
    {
        HWND hWindow = (HWND)wParam;
        if (hWindow && IsWindow(hWindow))
        {
            SelectWindow(hWnd, hWindow);
        }
    }

    switch (msg)
    {
        HANDLE_MSG(hWnd, WM_CREATE, OnCreate);
        HANDLE_MSG(hWnd, WM_COMMAND, OnCommand);
        HANDLE_MSG(hWnd, WM_PAINT, OnPaint);
        HANDLE_MSG(hWnd, WM_DRAWITEM, OnDrawItem);
        HANDLE_MSG(hWnd, WM_DESTROY, OnDestroy);

    case WM_ERASEBKGND:
        return 1;

    case WM_CTLCOLORSTATIC:
    {
        HWND hTmpWnd = (HWND)lParam;
        if (hTmpWnd != GetDlgItem(hWnd, IDC_CLIENT_CLASS_NAME) &&
            hTmpWnd != GetDlgItem(hWnd, IDC_CLIENT_TITLE) &&
            hTmpWnd != GetDlgItem(hWnd, IDC_CLIENT_PROCESS_NAME) &&
            hTmpWnd != GetDlgItem(hWnd, IDC_CLIENT_PROCESS_PID) &&
            hTmpWnd != GetDlgItem(hWnd, IDC_CLIENT_PATH) &&
            hTmpWnd != GetDlgItem(hWnd, IDC_CLIENT_EXECUTE_TIME))
        {
            SetBkColor((HDC)wParam, RGB(192, 192, 192));
            return (LRESULT)GetStockBrush(NULL_BRUSH);
        }
    }
    break;
    }
    return DefWindowProc(hWnd, msg, wParam, lParam);
}
// [/WindowProcedure]


// [OnCreate]: WM_CREATE
static BOOL OnCreate(HWND hWnd, LPCREATESTRUCT lpcs)
{
    WM_WINDOW_SELECTED = RegisterWindowMessage(_T("WM_WINDOW_SELECTED"));

    RECT rc;
    GetClientRect(hWnd, &rc);
    const int iWindowWidth = rc.right - rc.left;
    const int iWindowHeight = rc.bottom - rc.top;

    HWND hTmpWnd = NULL;

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
        SendMessage(hLogWnd, WM_SETFONT, (WPARAM)hFont, 0L);

        oldRichEditProcedure = (WNDPROC) SetWindowLongPtr(hLogWnd, GWLP_WNDPROC, (LONG_PTR)NewRichEditProcedure);

        AddTextToLog(hWnd, _T("Програма запущена успішно"), LOG_DEFAULT);
    }


    hTmpWnd = CreateWindowEx(0, _T("button"), _T("Закріпити вікно поверх всіх вікон"), WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
        xcenter + 9, iWindowHeight - 160 + 33, xcenter - 7 - 7 - 1, 20, hWnd, (HMENU)IDC_FIXED_WINDOW, NULL, NULL);
    SendMessage(hTmpWnd, WM_SETFONT, (WPARAM)hFont, 0L);
    SendMessage(hTmpWnd, BM_SETCHECK, true, 0L);

    hTmpWnd = CreateWindowEx(0, _T("button"), _T("Захоплення вікна курсором"), WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
        xcenter + 9, iWindowHeight - 160 + 33 + 20, xcenter - 7 - 7 - 1, 20, hWnd, (HMENU)IDC_CAPTURE_WINDOW_WITH_CURSOR, NULL, NULL);
    SendMessage(hTmpWnd, WM_SETFONT, (WPARAM)hFont, 0L);
    SendMessage(hTmpWnd, BM_SETCHECK, true, 0L);

    
    CreateWindowEx(0, _T("edit"), _T(""), WS_CHILD | WS_VISIBLE | ES_LEFT | ES_AUTOHSCROLL | ES_READONLY, 120, 256, 308, 19, hWnd, (HMENU)IDC_CLIENT_CLASS_NAME, NULL, NULL);
    CreateWindowEx(0, _T("edit"), _T(""), WS_CHILD | WS_VISIBLE | ES_LEFT | ES_AUTOHSCROLL | ES_READONLY, 120, 276, 308, 19, hWnd, (HMENU)IDC_CLIENT_TITLE, NULL, NULL);
    CreateWindowEx(0, _T("edit"), _T(""), WS_CHILD | WS_VISIBLE | ES_LEFT | ES_AUTOHSCROLL | ES_READONLY, 120, 296, 308, 19, hWnd, (HMENU)IDC_CLIENT_PROCESS_NAME, NULL, NULL);
    CreateWindowEx(0, _T("edit"), _T(""), WS_CHILD | WS_VISIBLE | ES_LEFT | ES_AUTOHSCROLL | ES_READONLY, 120, 316, 308, 19, hWnd, (HMENU)IDC_CLIENT_PROCESS_PID, NULL, NULL);
    CreateWindowEx(0, _T("edit"), _T(""), WS_CHILD | WS_VISIBLE | ES_LEFT | ES_AUTOHSCROLL | ES_READONLY, 120, 336, 308, 19, hWnd, (HMENU)IDC_CLIENT_PATH, NULL, NULL);
    CreateWindowEx(0, _T("edit"), _T(""), WS_CHILD | WS_VISIBLE | ES_LEFT | ES_AUTOHSCROLL | ES_READONLY, 120, 356, 308, 19, hWnd, (HMENU)IDC_CLIENT_EXECUTE_TIME, NULL, NULL);
    SendMessage(GetDlgItem(hWnd, IDC_CLIENT_CLASS_NAME), WM_SETFONT, (WPARAM)hFont, 0L);
    SendMessage(GetDlgItem(hWnd, IDC_CLIENT_TITLE), WM_SETFONT, (WPARAM)hFont, 0L);
    SendMessage(GetDlgItem(hWnd, IDC_CLIENT_PROCESS_NAME), WM_SETFONT, (WPARAM)hFont, 0L);
    SendMessage(GetDlgItem(hWnd, IDC_CLIENT_PROCESS_PID), WM_SETFONT, (WPARAM)hFont, 0L);
    SendMessage(GetDlgItem(hWnd, IDC_CLIENT_PATH), WM_SETFONT, (WPARAM)hFont, 0L);
    SendMessage(GetDlgItem(hWnd, IDC_CLIENT_EXECUTE_TIME), WM_SETFONT, (WPARAM)hFont, 0L);


    CreateWindowEx(0, _T("button"), _T(""), WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX, iWindowWidth - 112, 9, 16, 16, hWnd, (HMENU)IDC_CLIENT_FIXED_WINDOW, NULL, NULL);
    CreateWindowEx(0, _T("button"), _T(""), WS_CHILD | WS_VISIBLE | BS_OWNERDRAW, iWindowWidth - 72, 9, 16, 16, hWnd, (HMENU)IDC_CLIENT_MINIMIZE, NULL, NULL);
    CreateWindowEx(0, _T("button"), _T(""), WS_CHILD | WS_VISIBLE | BS_OWNERDRAW, iWindowWidth - 50, 9, 16, 16, hWnd, (HMENU)IDC_CLIENT_MAXIMIZE, NULL, NULL);
    CreateWindowEx(0, _T("button"), _T(""), WS_CHILD | WS_VISIBLE | BS_OWNERDRAW, iWindowWidth - 28, 9, 16, 16, hWnd, (HMENU)IDC_CLIENT_CLOSE, NULL, NULL);

    
    int x = 433;
    int y = 25;

    DWORD dwOwnerDrawButtonStyle = WS_CHILD | WS_VISIBLE | WS_BORDER | BS_OWNERDRAW;

    CreateWindowEx(0, _T("edit"), _T("0"), WS_CHILD | WS_VISIBLE | ES_LEFT | ES_AUTOHSCROLL | ES_NUMBER, x + 10, y + 30, 40, 18, hWnd, (HMENU)IDC_CLIENT_X, NULL, NULL);
    CreateWindowEx(0, _T("edit"), _T("0"), WS_CHILD | WS_VISIBLE | ES_LEFT | ES_AUTOHSCROLL | ES_NUMBER, x + 70, y + 30, 40, 18, hWnd, (HMENU)IDC_CLIENT_Y, NULL, NULL);
    CreateWindowEx(0, _T("edit"), _T("0"), WS_CHILD | WS_VISIBLE | ES_LEFT | ES_AUTOHSCROLL | ES_NUMBER, x + 10, y + 55, 40, 18, hWnd, (HMENU)IDC_CLIENT_SIZE_X, NULL, NULL);
    CreateWindowEx(0, _T("edit"), _T("0"), WS_CHILD | WS_VISIBLE | ES_LEFT | ES_AUTOHSCROLL | ES_NUMBER, x + 70, y + 55, 40, 18, hWnd, (HMENU)IDC_CLIENT_SIZE_Y, NULL, NULL);
    Edit_LimitText(GetDlgItem(hWnd, IDC_CLIENT_X), 4);
    Edit_LimitText(GetDlgItem(hWnd, IDC_CLIENT_Y), 4);
    Edit_LimitText(GetDlgItem(hWnd, IDC_CLIENT_SIZE_X), 4);
    Edit_LimitText(GetDlgItem(hWnd, IDC_CLIENT_SIZE_Y), 4);
    SendMessage(GetDlgItem(hWnd, IDC_CLIENT_X), WM_SETFONT, (WPARAM)hFont, 0L);
    SendMessage(GetDlgItem(hWnd, IDC_CLIENT_Y), WM_SETFONT, (WPARAM)hFont, 0L);
    SendMessage(GetDlgItem(hWnd, IDC_CLIENT_SIZE_X), WM_SETFONT, (WPARAM)hFont, 0L);
    SendMessage(GetDlgItem(hWnd, IDC_CLIENT_SIZE_Y), WM_SETFONT, (WPARAM)hFont, 0L);

    CreateWindowEx(0, _T("button"), _T("Змінити розміщення"), dwOwnerDrawButtonStyle, x + 130, y + 30, 300, 18, hWnd, (HMENU)IDC_CLIENT_MOVE_WINDOW, NULL, NULL);
    CreateWindowEx(0, _T("button"), _T("Змінити розмір"), dwOwnerDrawButtonStyle, x + 130, y + 55, 300, 18, hWnd, (HMENU)IDC_CLIENT_SET_SIZE, NULL, NULL);
    SendMessage(GetDlgItem(hWnd, IDC_CLIENT_MOVE_WINDOW), WM_SETFONT, (WPARAM)hFont, 0L);
    SendMessage(GetDlgItem(hWnd, IDC_CLIENT_SET_SIZE), WM_SETFONT, (WPARAM)hFont, 0L);

    CreateWindowEx(0, _T("button"), _T("Завершити процес"), dwOwnerDrawButtonStyle, x, y + 90, 210, 18, hWnd, (HMENU)IDC_TERMINATE_PROCESS, NULL, NULL);
    SendMessage(GetDlgItem(hWnd, IDC_TERMINATE_PROCESS), WM_SETFONT, (WPARAM)hFont, 0L);
    
    CreateWindowEx(0, _T("button"), _T("Почистити память"), dwOwnerDrawButtonStyle, x + 220, y + 90, 210, 18, hWnd, (HMENU)IDC_CLEAR_MEMORY, NULL, NULL);
    SendMessage(GetDlgItem(hWnd, IDC_CLEAR_MEMORY), WM_SETFONT, (WPARAM)hFont, 0L);

    CreateWindowEx(0, _T("edit"), _T(""), WS_CHILD | WS_VISIBLE | ES_LEFT | ES_AUTOHSCROLL, x + 40, y + 160, 300, 18, hWnd, (HMENU)IDC_INJECTOR_DLL, NULL, NULL);
    CreateWindowEx(0, _T("button"), _T("Вибрати"), dwOwnerDrawButtonStyle, x + 350, y + 160, 80, 18, hWnd, (HMENU)IDC_INJECTOR_BROWSE_DLL, NULL, NULL);
    CreateWindowEx(0, _T("button"), _T("Інжектувати"), dwOwnerDrawButtonStyle, x, y + 185, 430, 18, hWnd, (HMENU)IDC_INJECTOR_RUN, NULL, NULL);
    SendMessage(GetDlgItem(hWnd, IDC_INJECTOR_DLL), WM_SETFONT, (WPARAM)hFont, 0L);
    SendMessage(GetDlgItem(hWnd, IDC_INJECTOR_BROWSE_DLL), WM_SETFONT, (WPARAM)hFont, 0L);
    SendMessage(GetDlgItem(hWnd, IDC_INJECTOR_RUN), WM_SETFONT, (WPARAM)hFont, 0L);

    DWORD dwThreadId = 0;
    HANDLE hThread = CreateThread(NULL, NULL, Thread, (LPVOID)hWnd, 0, &dwThreadId);
    if (hThread != NULL)
    {
        CloseHandle(hThread);
    }

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

    case IDC_WINDOWS_VIEWER:
    {
        DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_WINDOWS_VIEWER), hWnd, (DLGPROC)WindowsViewer_DialogProcedure);
    }
    break;

    case IDC_DEPENDENCY_LIST:
    {
        DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_DEPENDENCY_LIST), hWnd, (DLGPROC)DependencyList_DialogProcedure);
    }
    break;

    case IDC_SYSTEM_INFO:
    {
        DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_SYSTEM_INFO), hWnd, (DLGPROC)SystemInfo_DialogProcedure);
    }
    break;

    case IDC_ABOUT:
    {
        DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_ABOUT), hWnd, (DLGPROC)About_DialogProcedure);
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

    case IDC_CLIENT_FIXED_WINDOW:
    {
        if (process.isValid())
        {
            if (SendMessage(GetDlgItem(hWnd, IDC_CLIENT_FIXED_WINDOW), BM_GETCHECK, 0, 0L))
            {
                if (!SetWindowPos(process.getWindow(), HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE))
                    AddTextToLog(hWnd, _T("Не вдалося поставити захоплене вікно поверх всіх вікон"), LOG_ERROR);
            }
            else
                SetWindowPos(process.getWindow(), HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
        }
        else
            SendMessage(GetDlgItem(hWnd, IDC_CLIENT_FIXED_WINDOW), BM_SETCHECK, 0, 0L);
    }
    break;

    case IDC_CLIENT_MINIMIZE:
    {
        process.showWindow(SW_MINIMIZE);
    }
    break;

    case IDC_CLIENT_MAXIMIZE:
    {
        process.showWindow(SW_MAXIMIZE);
    }
    break;

    case IDC_CLIENT_CLOSE:
    {
        process.closeWindow();
    }
    break;

    case IDC_CLIENT_MOVE_WINDOW:
    {
        TCHAR tx[16] = { 0 };
        TCHAR ty[16] = { 0 };
        GetWindowText(GetDlgItem(hWnd, IDC_CLIENT_X), tx, 16);
        GetWindowText(GetDlgItem(hWnd, IDC_CLIENT_Y), ty, 16);

        int x = _ttoi(tx);
        int y = _ttoi(ty);

        if (process.isValid())
            SetWindowPos(process.getWindow(), NULL, x, y, 0, 0, SWP_NOSIZE);
    }
    break;

    case IDC_CLIENT_SET_SIZE:
    {
        TCHAR tx[16] = { 0 };
        TCHAR ty[16] = { 0 };
        GetWindowText(GetDlgItem(hWnd, IDC_CLIENT_SIZE_X), tx, 16);
        GetWindowText(GetDlgItem(hWnd, IDC_CLIENT_SIZE_Y), ty, 16);

        int x = _ttoi(tx);
        int y = _ttoi(ty);

        if (process.isValid())
            SetWindowPos(process.getWindow(), NULL, 0, 0, x, y, SWP_NOMOVE);
    }
    break;

    case IDC_TERMINATE_PROCESS:
    {
        if (process.getProcPID())
        {
            HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, true, process.getProcPID());
            if (hProcess)
            {
                TerminateProcess(hProcess, 0);
                CloseHandle(hProcess);
            }
            else
                AddTextToLog(hWnd, _T("Не вдалося відкрити процес"), LOG_ERROR);
        }
        else
            AddTextToLog(hWnd, _T("Спочатку виберіть вікно"), LOG_ERROR);
    }
    break;

    case IDC_CLEAR_MEMORY:
    {
        if (process.getProcPID())
        {
            HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, true, process.getProcPID());
            if (hProcess)
            {
                SetProcessWorkingSetSize(hProcess, 0xffffffff, 0xffffffff);
                CloseHandle(hProcess);
            }
            else
                AddTextToLog(hWnd, _T("Не вдалося відкрити процес"), LOG_ERROR);
        }
        else
            AddTextToLog(hWnd, _T("Спочатку виберіть вікно"), LOG_ERROR);
    }
    break;

    case IDC_INJECTOR_BROWSE_DLL:
    {
        TCHAR szFileName[2048] = { 0 };
        OPENFILENAME ofn;
        RtlZeroMemory(&ofn, sizeof(OPENFILENAME));
        ofn.lStructSize = sizeof(OPENFILENAME);
        ofn.hwndOwner = hWnd;
        ofn.lpstrFilter = _T("Library Files (*.dll)\0*.dll\0All Files (*.*)\0*.*\0");
        ofn.lpstrFile = szFileName;
        ofn.nMaxFile = sizeof(szFileName);
        ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
        if (GetOpenFileName(&ofn))
            SetWindowText(GetDlgItem(hWnd, IDC_INJECTOR_DLL), szFileName);
        else
            AddTextToLog(hWnd, _T("Не вдалося получити ім'я файлу для відкриття"), LOG_ERROR);

        SetProcessWorkingSetSize(GetCurrentProcess(), 0xffffffff, 0xffffffff);
    }
    break;

    case IDC_INJECTOR_RUN:
    {
        TCHAR szDllName[2048] = { 0 };
        if (GetWindowText(GetDlgItem(hWnd, IDC_INJECTOR_DLL), szDllName, sizeof(szDllName) / sizeof(TCHAR)))
        {
            if (FileExists(szDllName))
            {
                if (process.getProcPID())
                {
                    std::string dll_name;

#if defined(_UNICODE) || defined(UNICODE)
                    dll_name = wchar_t_to_string(szDllName);
#else
                    dll_name = szDllName;
#endif

                    if (InjectDll(dll_name.c_str(), process.getProcPID()))
                        AddTextToLog(hWnd, _T("Інжектовано успішно"), LOG_DEFAULT);
                    else
                        AddTextToLog(hWnd, _T("Сталася помилка при інжектуванні"), LOG_ERROR);
                }
                else
                    AddTextToLog(hWnd, _T("Спочатку виберіть вікно"), LOG_ERROR);
            }
            else
                AddTextToLog(hWnd, _T("Файл бібліотеки не знайдено"), LOG_ERROR);
        }
        else
            AddTextToLog(hWnd, _T("Сталася невідома помилка"), LOG_ERROR);
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
    
    img_fixed.Draw(hMemDC, iWindowWidth - 130, 9);


    const int xcenter = iWindowWidth >> 1;

    DrawEmptyRectangle(hMemDC, 5, iWindowHeight - 151, xcenter - 5, iWindowHeight - 5);
    DrawHeadlineForPart(hMemDC, 7, iWindowHeight - 151 + 2, xcenter - 7 - 7, _T("Log"), hHeadLineBrush, &img_log);

    GetTextExtentPoint32(hMemDC, szCurrentTime, lstrlen(szCurrentTime), &size);
    TextOut(hMemDC, xcenter - size.cx - 10, iWindowHeight - 151 + 4, szCurrentTime, lstrlen(szCurrentTime));


    DrawEmptyRectangle(hMemDC, xcenter + 5, iWindowHeight - 151, iWindowWidth - 5, iWindowHeight - 5);
    DrawHeadlineForPart(hMemDC, xcenter + 7, iWindowHeight - 151 + 2, xcenter - 7 - 6, _T("Settings"), hHeadLineBrush, &img_settings);


    DrawEmptyRectangle(hMemDC, 7, 29, 420 + 7, 220 + 27);
    if (process.isValid() && img_current_window.HasImage())
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


    lstrcpy(szText, _T("Ім'я класу вікна"));
    TextOut(hMemDC, 7, 256, szText, lstrlen(szText));

    lstrcpy(szText, _T("Заголовок вікна"));
    TextOut(hMemDC, 7, 276, szText, lstrlen(szText));

    lstrcpy(szText, _T("Назва процесу"));
    TextOut(hMemDC, 7, 296, szText, lstrlen(szText));

    lstrcpy(szText, _T("PID процесу"));
    TextOut(hMemDC, 7, 316, szText, lstrlen(szText));

    lstrcpy(szText, _T("Шлях до файлу"));
    TextOut(hMemDC, 7, 336, szText, lstrlen(szText));

    lstrcpy(szText, _T("Дата запуску"));
    TextOut(hMemDC, 7, 356, szText, lstrlen(szText));

    
    int x = 433;
    int y = 25;

    lstrcpy(szText, _T("Дії"));
    TextOut(hMemDC, x + 10, y + 5, szText, lstrlen(szText));
    DrawLine(hMemDC, x, y + 22, iWindowWidth - 13, y + 22);

    TextOut(hMemDC, x, y + 30, _T("x"), 1);
    TextOut(hMemDC, x + 60, y + 30, _T("y"), 1);
    TextOut(hMemDC, x, y + 55, _T("x"), 1);
    TextOut(hMemDC, x + 60, y + 55, _T("y"), 1);

    lstrcpy(szText, _T("Інжектор"));
    TextOut(hMemDC, x + 10, y + 133, szText, lstrlen(szText));
    DrawLine(hMemDC, x, y + 150, iWindowWidth - 13, y + 150);

    lstrcpy(szText, _T("DLL"));
    TextOut(hMemDC, x, y + 160, szText, lstrlen(szText));


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


// [OnDrawItem]: WM_DRAWITEM
static void OnDrawItem(HWND hWnd, const DRAWITEMSTRUCT *lpdis)
{
    if (lpdis->CtlType & ODT_BUTTON)
    {
        if (lpdis->itemState & ODS_SELECTED)
            FillRect(lpdis->hDC, &lpdis->rcItem, (HBRUSH)GetStockObject(GRAY_BRUSH));
        else
            FillRect(lpdis->hDC, &lpdis->rcItem, (HBRUSH)GetStockObject(LTGRAY_BRUSH));

        TCHAR szText[256] = { 0 };
        memset(szText, 0, sizeof(szText));

        RECT rc;
        memcpy(&rc, &lpdis->rcItem, sizeof(RECT));

        int iOldBkMode = SetBkMode(lpdis->hDC, TRANSPARENT);

        TEXTMETRIC tm;
        GetTextMetrics(lpdis->hDC, &tm);

        rc.top += (rc.bottom - rc.top - tm.tmHeight) >> 1;

        if (GetWindowText(lpdis->hwndItem, szText, sizeof(szText) / sizeof(TCHAR)))
            DrawText(lpdis->hDC, szText, lstrlen(szText), &rc, DT_CENTER);

        SetBkMode(lpdis->hDC, iOldBkMode);

        switch (lpdis->CtlID)
        {
        case IDC_CLIENT_MINIMIZE:
            img_minimize.Draw(lpdis->hDC, lpdis->rcItem.left, lpdis->rcItem.top);
            break;
        case IDC_CLIENT_MAXIMIZE:
            img_maximize.Draw(lpdis->hDC, lpdis->rcItem.left, lpdis->rcItem.top);
            break;
        case IDC_CLIENT_CLOSE:
            img_close.Draw(lpdis->hDC, lpdis->rcItem.left, lpdis->rcItem.top);
            break;
        }
    }
}
// [/OnDrawItem]


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

    PostQuitMessage(0);
}
// [/OnDestroy]
