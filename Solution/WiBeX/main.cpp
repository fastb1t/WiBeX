#include <tchar.h>
#include <windows.h>
#include <windowsx.h>
#include <shlobj.h>

#include "wnd_proc.h"
#include "resource.h"
#include "algorithms.h"

#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "ws2_32.lib")

#define ShowErrorMessage(str) MessageBox(NULL, (str), _T("Error!"), MB_OK | MB_ICONERROR | MB_TOPMOST);


// [_tWinMain]: entry point.
int WINAPI _tWinMain(
    _In_        HINSTANCE hInstance,
    _In_        HINSTANCE hPrevInstance,
    _In_opt_    TCHAR *lpCmdLine,
    _In_        int nShowCmd)
{
    InitCommonControls();

    if (!IsUserAnAdmin())
    {
        ShowErrorMessage(_T("Запустіть програму від імені адміністратора"));
        return -1;
    }

    const TCHAR szTitle[] = _T("WiBeX");
    const TCHAR szClassName[] = _T("__wibex__class__");
    const TCHAR szMutexName[] = _T("__wibex__mutex__");

    HANDLE hMutex = NULL;
    if (MutexIsExist(szMutexName))
    {
        HWND hWnd = FindWindow(szClassName, NULL);
        if (hWnd)
        {
            if (IsIconic(hWnd))
                ShowWindow(hWnd, SW_RESTORE);
            SetForegroundWindow(hWnd);
        }
        return 0;
    }
    else
    {
        hMutex = CreateMutex(NULL, FALSE, szMutexName);
        if (!hMutex || WaitForSingleObject(hMutex, 0) != WAIT_OBJECT_0)
        {
            ShowErrorMessage(_T("Could not create mutex!")); // Не удалось создать мьютекс.
            if (!hMutex)
                CloseHandle(hMutex);
        }
    }

    LoadLibrary(_T("riched32.dll"));

    HANDLE hProcessToken = NULL;
    OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hProcessToken);
    if (!hProcessToken || !SetPrivilege(hProcessToken, _T("SeDebugPrivilege"), TRUE))
    {
        ShowErrorMessage(_T("Не вдалося встановити привілегію SeDebugPrivilege"));
        ReleaseMutex(hMutex);
        CloseHandle(hMutex);
        return -1;
    }

    WSADATA wd;
    if (WSAStartup(MAKEWORD(2, 0), &wd) != 0)
    {
        ShowErrorMessage(_T("WSAStartup failed"));
        SetPrivilege(hProcessToken, _T("SeDebugPrivilege"), FALSE);
        ReleaseMutex(hMutex);
        CloseHandle(hMutex);
        return -1;
    }

    DWORD dwStyle = WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX & ~WS_THICKFRAME;
    DWORD dwExStyle = WS_EX_APPWINDOW | WS_EX_TOPMOST;

    RECT rc;
    SetRect(&rc, 0, 0, 875, 560);
    AdjustWindowRectEx(&rc, dwStyle, FALSE, dwExStyle);

    const int iWindowWidth = rc.right - rc.left;
    const int iWindowHeight = rc.bottom - rc.top;

    WNDCLASSEX wcex;
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WindowProcedure;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = MAKEINTRESOURCE(IDM_MENU);
    wcex.lpszClassName = szClassName;
    wcex.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

    if (!RegisterClassEx(&wcex))
    {
        ShowErrorMessage(_T("Window Registration Failed"));
        SetPrivilege(hProcessToken, _T("SeDebugPrivilege"), FALSE);
        ReleaseMutex(hMutex);
        CloseHandle(hMutex);
        WSACleanup();
        return -1;
    }

    HWND hWnd = CreateWindowEx(
        dwExStyle,
        szClassName,
        szTitle,
        dwStyle,
        (GetSystemMetrics(SM_CXSCREEN) >> 1) - (iWindowWidth >> 1),
        (GetSystemMetrics(SM_CYSCREEN) >> 1) - (iWindowHeight >> 1),
        iWindowWidth,
        iWindowHeight,
        HWND_DESKTOP,
        NULL,
        hInstance,
        NULL);

    if (!hWnd)
    {
        ShowErrorMessage(_T("Window Creation Failed"));
        UnregisterClass(szClassName, hInstance);
        SetPrivilege(hProcessToken, _T("SeDebugPrivilege"), FALSE);
        ReleaseMutex(hMutex);
        CloseHandle(hMutex);
        WSACleanup();
        return -1;
    }

    ShowWindow(hWnd, SW_SHOWNORMAL);
    UpdateWindow(hWnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        DispatchMessage(&msg);
        TranslateMessage(&msg);
    }
    
    UnregisterClass(szClassName, hInstance);
    SetPrivilege(hProcessToken, _T("SeDebugPrivilege"), FALSE);
    ReleaseMutex(hMutex);
    CloseHandle(hMutex);
    WSACleanup();
    return (int)msg.wParam;
}
// [/_tWinMain]
