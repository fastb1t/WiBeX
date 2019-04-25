#include <vector>
#include <tchar.h>
#include <windows.h>
#include <windowsx.h>

#include "windows_viewer.h"
#include "algorithms.h"
#include "Process.h"
#include "DC.h"

static BOOL DlgOnInitDialog(HWND, HWND, LPARAM);                            // WM_INITDIALOG
static void DlgOnCommand(HWND, int, HWND, UINT);                            // WM_COMMAND
static void DlgOnPaint(HWND);                                               // WM_PAINT
static void DlgOnMeasureItem(HWND, MEASUREITEMSTRUCT *);                    // WM_MEASUREITEM
static void DlgOnDrawItem(HWND, const DRAWITEMSTRUCT *);                    // WM_DRAWITEM
static HBRUSH DlgOnCtlColorListBox(HWND, HDC, HWND, int);                   // WM_CTLCOLORLISTBOX

#define IDC_LISTBOX 1000
#define IDC_REFRESH 1001

struct WINDOW_INFO {
    HWND hWnd;
    DC *dc;

    WINDOW_INFO() : hWnd(NULL) {
        dc = new (std::nothrow) DC;
    }

    ~WINDOW_INFO() {
        delete dc;
    }
};

static std::vector<WINDOW_INFO*> windows;
extern HFONT hFont;


// [AddItem]: add new item to listbox.
static void AddItem(HWND hWnd, const TCHAR *szText, WINDOW_INFO *wi)
{
    LRESULT iItem = SendMessage(hWnd, LB_ADDSTRING, 0, (LPARAM)szText);
    SendMessage(hWnd, LB_SETITEMDATA, (WPARAM)iItem, (LPARAM)wi);
}
// [/AddItem]


// [EnumWindowsCallback]:
static BOOL CALLBACK EnumWindowsCallback(HWND hWnd, LPARAM lParam)
{
    TCHAR szClientClassName[256] = { 0 };
    TCHAR szMyClassName[256] = { 0 };
    if (!GetClassName(hWnd, szClientClassName, sizeof(szClientClassName) / sizeof(TCHAR)) ||
        !GetClassName(GetParent((HWND)lParam), szMyClassName, sizeof(szMyClassName) / sizeof(TCHAR)))
        return TRUE;

    if (lstrcmp(szMyClassName, szClientClassName))
    {
        RECT rc;
        GetClientRect(hWnd, &rc);
        const int iWindowWidth = rc.right - rc.left;
        const int iWindowHeight = rc.bottom - rc.top;
        if (hWnd &&
            IsWindow(hWnd) &&
            IsWindowVisible(hWnd) &&
            GetParent(hWnd) == HWND_DESKTOP &&
            iWindowWidth > 1 &&
            iWindowHeight > 1)
        {
            WINDOW_INFO *wi = new WINDOW_INFO;
            if (wi)
            {
                wi->hWnd = hWnd;
                wi->dc->MakeScreenShot(hWnd);

                windows.push_back(wi);
                AddItem(GetDlgItem((HWND)lParam, IDC_LISTBOX), _T(""), wi);
            }
        }
    }
    return TRUE;
}
// [/EnumWindowsCallback]


// [WindowsViewer_DialogProcedure]:
BOOL CALLBACK WindowsViewer_DialogProcedure(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
        HANDLE_MSG(hWnd, WM_INITDIALOG, DlgOnInitDialog);
        HANDLE_MSG(hWnd, WM_COMMAND, DlgOnCommand);
        HANDLE_MSG(hWnd, WM_PAINT, DlgOnPaint);
        HANDLE_MSG(hWnd, WM_MEASUREITEM, DlgOnMeasureItem);
        HANDLE_MSG(hWnd, WM_DRAWITEM, DlgOnDrawItem);
        HANDLE_MSG(hWnd, WM_CTLCOLORLISTBOX, DlgOnCtlColorListBox);

    case WM_ERASEBKGND:
        return 1;

    default:
        return FALSE;
    }
}
// [/WindowsViewer_DialogProcedure]


// [DlgOnInitDialog]:
static BOOL DlgOnInitDialog(HWND hWnd, HWND, LPARAM)
{
    RECT rc;
    GetClientRect(hWnd, &rc);
    const int iWindowWidth = rc.right - rc.left;
    const int iWindowHeight = rc.bottom - rc.top;

    HWND hListBox = CreateWindowEx(0, _T("listbox"), _T(""), WS_CHILD | WS_VISIBLE | WS_BORDER | WS_VSCROLL | LBS_OWNERDRAWFIXED | LBS_HASSTRINGS | LBS_SORT,
        10, 10, iWindowWidth - 20, iWindowHeight - 40, hWnd, (HMENU)IDC_LISTBOX, GetModuleHandle(NULL), NULL);
    SendMessage(hListBox, WM_SETFONT, (WPARAM)hFont, (LPARAM)0L);

    HWND hOk = CreateWindowEx(0, _T("button"), _T("Select"), WS_CHILD | WS_VISIBLE | WS_BORDER | BS_OWNERDRAW,
        (iWindowWidth >> 1) + 5, iWindowHeight - 30, 100, 20, hWnd, (HMENU)IDOK, NULL, NULL);
    SendMessage(hOk, WM_SETFONT, (WPARAM)hFont, (LPARAM)0L);

    HWND hClear = CreateWindowEx(0, _T("button"), _T("Refresh"), WS_CHILD | WS_VISIBLE | WS_BORDER | BS_OWNERDRAW,
        (iWindowWidth >> 1) - 105, iWindowHeight - 30, 100, 20, hWnd, (HMENU)IDC_REFRESH, NULL, NULL);
    SendMessage(hClear, WM_SETFONT, (WPARAM)hFont, (LPARAM)0L);

    EnumWindows(&EnumWindowsCallback, (LPARAM)hWnd);
    return TRUE;
}
// [/DlgOnInitDialog]


// [DlgOnCommand]:
static void DlgOnCommand(HWND hWnd, int id, HWND, UINT)
{
    switch (id)
    {
    case IDC_REFRESH:
    {
        HWND hListBox = GetDlgItem(hWnd, IDC_LISTBOX);
        if (hListBox)
        {
            SendMessage(hListBox, LB_RESETCONTENT, 0, 0L);
            for (std::vector<WINDOW_INFO *>::iterator p = windows.begin(); p != windows.end(); p++)
                delete *p;
            windows.clear();
            EnumWindows(&EnumWindowsCallback, (LPARAM)hWnd);
        }
    }
    break;

    case IDOK:
    {
        LRESULT iItem = SendMessage(GetDlgItem(hWnd, IDC_LISTBOX), LB_GETCURSEL, 0, 0);
        if (iItem == -1)
        {
            MessageBox(hWnd, _T("Спочатку виберіть вікно!"), _T("Помилка"), MB_OK | MB_ICONERROR | MB_TOPMOST);
            break;
        }
        WINDOW_INFO *wi = (WINDOW_INFO *)SendMessage(GetDlgItem(hWnd, IDC_LISTBOX), LB_GETITEMDATA, iItem, 0);
        if (wi)
            SendMessage(GetParent(hWnd), WM_COMMAND, (WPARAM)IDC_WINDOW_SELECTED, (LPARAM)wi->hWnd);
    }
    //break;

    case IDCANCEL:
    {
        for (std::vector<WINDOW_INFO *>::iterator p = windows.begin(); p != windows.end(); p++)
            delete *p;
        windows.clear();
        EndDialog(hWnd, 0);
    }
    break;
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
    FillRect(hMemDC, &ps.rcPaint, (HBRUSH)GetStockObject(LTGRAY_BRUSH));

    BitBlt(hDC, 0, 0, iWindowWidth, iWindowHeight, hMemDC, 0, 0, SRCCOPY);
    SelectObject(hMemDC, hOldBitmap);
    DeleteObject(hBitmap);
    DeleteDC(hMemDC);
    EndPaint(hWnd, &ps);
}
// [/DlgOnPaint]


// [DlgOnMeasureItem]: WM_MEASUREITEM
static void DlgOnMeasureItem(HWND hWnd, MEASUREITEMSTRUCT *lpMeasureItem)
{
    if (lpMeasureItem->CtlType & ODT_LISTBOX)
    {
        lpMeasureItem->itemHeight = 103;
    }
}
// [/DlgOnMeasureItem]


// [DlgOnDrawItem]: WM_DRAWITEM
static void DlgOnDrawItem(HWND hWnd, const DRAWITEMSTRUCT *lpDrawItem)
{
    if (lpDrawItem->CtlType & ODT_LISTBOX)
    {
        if (lpDrawItem->itemID == -1)
            return;

        switch (lpDrawItem->itemAction)
        {
        case ODA_SELECT:
        case ODA_DRAWENTIRE:
        {
            FillRect(lpDrawItem->hDC, &lpDrawItem->rcItem, (HBRUSH)GetStockObject(LTGRAY_BRUSH));

            RECT rc;
            memcpy(&rc, &lpDrawItem->rcItem, sizeof(RECT));
            InflateRect(&rc, -2, -2);

            HBRUSH hBrush;
            if (lpDrawItem->itemState & ODS_SELECTED)
                hBrush = CreateSolidBrush(RGB(150, 170, 150));
            else
                hBrush = CreateSolidBrush(RGB(150, 150, 150));
            FillRect(lpDrawItem->hDC, &rc, hBrush);
            DeleteObject(hBrush);

            const int iImageMaxWidth = 250;
            const int iImageMaxHeight = 100;

            WINDOW_INFO *wi = (WINDOW_INFO *)SendMessage(lpDrawItem->hwndItem, LB_GETITEMDATA, lpDrawItem->itemID, 0);
            if (!wi || !wi->dc->HasImage())
                return;

            if (wi->dc->GetCurrentSize().cx == wi->dc->GetOriginalSize().cx && wi->dc->GetCurrentSize().cy ==  wi->dc->GetOriginalSize().cy)
                wi->dc->TransformImage(iImageMaxWidth, iImageMaxHeight);

            wi->dc->Draw(lpDrawItem->hDC, lpDrawItem->rcItem.left + 4, lpDrawItem->rcItem.top + 4);

            Process process;
            process.setWindow(wi->hWnd);

            int iOldBkMode = SetBkMode(lpDrawItem->hDC, TRANSPARENT);

            TextOut(lpDrawItem->hDC, lpDrawItem->rcItem.left + iImageMaxWidth + 6, lpDrawItem->rcItem.top + 10, _T("Заголовок:"), lstrlen(_T("Заголовок:")));
            TextOut(lpDrawItem->hDC, lpDrawItem->rcItem.left + iImageMaxWidth + 6, lpDrawItem->rcItem.top + 30, _T("Клас:"), lstrlen(_T("Клас:")));
            TextOut(lpDrawItem->hDC, lpDrawItem->rcItem.left + iImageMaxWidth + 6, lpDrawItem->rcItem.top + 50, _T("Процес:"), lstrlen(_T("Процес:")));
            TextOut(lpDrawItem->hDC, lpDrawItem->rcItem.left + iImageMaxWidth + 6, lpDrawItem->rcItem.top + 70, _T("PID:"), lstrlen(_T("PID:")));

            rc.left = lpDrawItem->rcItem.left + iImageMaxWidth + 6 + 80;
            rc.right = lpDrawItem->rcItem.right - 5;

            rc.top = lpDrawItem->rcItem.top + 10;
            DrawText(lpDrawItem->hDC, process.getTitle().c_str(), (int)process.getTitle().size(), &rc, DT_LEFT);

            rc.top = lpDrawItem->rcItem.top + 30;
            DrawText(lpDrawItem->hDC, process.getClassName().c_str(), (int)process.getClassName().size(), &rc, DT_LEFT);

            rc.top = lpDrawItem->rcItem.top + 50;
            DrawText(lpDrawItem->hDC, process.getExeName().c_str(), (int)process.getExeName().size(), &rc, DT_LEFT);

            rc.top = lpDrawItem->rcItem.top + 70;
            DrawText(lpDrawItem->hDC, process.getPID().c_str(), (int)process.getPID().size(), &rc, DT_LEFT);

            SetBkMode(lpDrawItem->hDC, iOldBkMode);
        }
        break;

        case ODA_FOCUS:
            break;
        }
    }
    else if (lpDrawItem->CtlType & ODT_BUTTON)
    {
        if (lpDrawItem->itemState & ODS_SELECTED)
            FillRect(lpDrawItem->hDC, &lpDrawItem->rcItem, (HBRUSH)GetStockObject(GRAY_BRUSH));
        else
            FillRect(lpDrawItem->hDC, &lpDrawItem->rcItem, (HBRUSH)GetStockObject(LTGRAY_BRUSH));

        TCHAR szText[256] = { 0 };
        memset(szText, 0, sizeof(szText));

        RECT rc = lpDrawItem->rcItem;

        int iOldBkMode = SetBkMode(lpDrawItem->hDC, TRANSPARENT);

        TEXTMETRIC tm;
        GetTextMetrics(lpDrawItem->hDC, &tm);

        rc.top += (rc.bottom - rc.top - tm.tmHeight) >> 1;

        if (GetWindowText(lpDrawItem->hwndItem, szText, sizeof(szText) / sizeof(TCHAR)))
            DrawText(lpDrawItem->hDC, szText, lstrlen(szText), &rc, DT_CENTER);

        SetBkMode(lpDrawItem->hDC, iOldBkMode);
    }
}
// [/DlgOnDrawItem]


// [DlgOnCtlColorListBox]: WM_CTLCOLORLISTBOX
static HBRUSH DlgOnCtlColorListBox(HWND hWnd, HDC hDC, HWND hWndChild, int type)
{
    return (HBRUSH)GetStockObject(LTGRAY_BRUSH);
}
// [/DlgOnCtlColorListBox]
