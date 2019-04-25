#include <tchar.h>
#include <cstring>
#include <vector>
#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <psapi.h>

#include "dependency_list.h"
#include "algorithms.h"
#include "Process.h"

static BOOL DlgOnInitDialog(HWND, HWND, LPARAM);                            // WM_INITDIALOG
static void DlgOnCommand(HWND, int, HWND, UINT);                            // WM_COMMAND

#define IDC_LIST 100

extern Process process;

void ProcessError(int n, HWND hWnd)
{
    /*
    DWORD dwLastError = GetLastError();
    char msg[128] = { 0 };
    wsprintf(msg, "%d - %d", n, dwLastError);
    MessageBox(GetParent(hWnd), msg, "", MB_OK | MB_ICONERROR | MB_TOPMOST);
    //*/
    //*
    LPVOID lpMsgBuf;
    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        GetLastError(),
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR)&lpMsgBuf,
        0, NULL);

    MessageBox(GetParent(hWnd), (LPTSTR)lpMsgBuf, _T("Error"), MB_OK | MB_ICONERROR | MB_TOPMOST);

    LocalFree(lpMsgBuf);
    //*/
}


// [RefreshList]:
bool RefreshList(HWND hWndParent)
{
    HWND hListWnd = GetDlgItem(hWndParent, IDC_LIST);
    if (!hListWnd)
        return false;

    //HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, process.getProcPID());
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, true, process.getProcPID());
    if (!hProcess)
    {
        ProcessError(1, hWndParent);
        return false;
    }

    WaitForInputIdle(hProcess, 100);

    HMODULE hModules[2048];
    DWORD dwModules = sizeof(hModules);
    if (!EnumProcessModulesEx(hProcess, hModules, dwModules, &dwModules, LIST_MODULES_ALL))
    {
        ProcessError(2, hWndParent);
        return false;
    }

    TCHAR szModuleName[256] = { 0 };
    String str, tmp;
    size_t p = 0;

    for (size_t i = 0; i < (dwModules / sizeof(HMODULE)); i++)
    {
        memset(szModuleName, 0, sizeof(szModuleName));
        if (GetModuleFileNameEx(hProcess, hModules[i], szModuleName, 256))
        {
            LV_ITEM lvi;
            RtlZeroMemory(&lvi, sizeof(lvi));
            lvi.mask = LVIF_TEXT;
            lvi.iItem = (int)i;
            ListView_InsertItem(hListWnd, &lvi);

            str = szModuleName;

            wsprintf(szModuleName, _T("%d"), i);
            ListView_SetItemText(hListWnd, i, 0, szModuleName);

            //ListView_SetItemText(hListWnd, i, 1, (LPSTR)str.c_str());

            if ((p = str.rfind('\\')) != std::string::npos)
            {
                tmp = str.substr(p + 1, str.length() - p - 1);
                ListView_SetItemText(hListWnd, i, 1, (LPSTR)tmp.c_str());

                tmp = str.substr(0, p + 1);
                ListView_SetItemText(hListWnd, i, 2, (LPSTR)tmp.c_str());
            }
        }
    }

    CloseHandle(hProcess);
    return true;
}
// [/RefreshList]


// [DependencyList_DialogProcedure]:
BOOL CALLBACK DependencyList_DialogProcedure(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
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
        EndPaint(hWnd, &ps);
    }
    break;

    case WM_ERASEBKGND:
        break;

    default:
        return FALSE;
    }
    return TRUE;
}
// [/DependencyList_DialogProcedure]


// [DlgOnInitDialog]: WM_INITDIALOG
static BOOL DlgOnInitDialog(HWND hWnd, HWND, LPARAM)
{
    RECT rc;
    GetClientRect(hWnd, &rc);
    const int iWindowWidth = rc.right - rc.left;
    const int iWindowHeight = rc.bottom - rc.top;

    HWND hListWnd = CreateWindowEx(0, WC_LISTVIEW, _T(""), WS_VISIBLE | WS_CHILD | WS_VSCROLL | LVS_REPORT,
        10, 10, iWindowWidth - 20, iWindowHeight - 20, hWnd, (HMENU)IDC_LIST, NULL, NULL);

    LV_COLUMN lvc;
    RtlZeroMemory(&lvc, sizeof(lvc));
    lvc.mask = LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH;
    lvc.fmt = LVCFMT_LEFT;

    lvc.cx = 30;
    lvc.pszText = (LPSTR) "";
    ListView_InsertColumn(hListWnd, 1, &lvc);

    lvc.cx = 200;
    lvc.pszText = (LPSTR) "File";
    ListView_InsertColumn(hListWnd, 2, &lvc);

    lvc.cx = iWindowWidth - (10 + 30 + 200 + 10 + 30);
    lvc.pszText = (LPSTR) "Path to file";
    ListView_InsertColumn(hListWnd, 3, &lvc);

    ListView_SetExtendedListViewStyle(hListWnd, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

    RefreshList(hWnd);
    
    return TRUE;
}
// [/DlgOnInitDialog]


// [DlgOnCommand]: WM_COMMAND
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
