#include <tchar.h>
#include <windows.h>
#include <shlobj.h>

#include "wnd_proc.h"
#include "resource.h"
#include "algorithms.h"

#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "psapi.lib")


// [_tWinMain]: entry point.
int WINAPI _tWinMain(
    _In_        HINSTANCE hInstance,
    _In_opt_    HINSTANCE hPrevInstance,
    _In_        TCHAR* lpCmdLine,
    _In_        int nShowCmd)
{
    if (!IsUserAnAdmin())
    {
        MessageBox(NULL, _T("Запустите программу от имени администратора."), _T("Ошибка!"), MB_OK | MB_ICONERROR | MB_TOPMOST);
        return 0;
    }

    const TCHAR szTitle[] = _T("WiBeX");
    const TCHAR szClassName[] = _T("__wibex__class__");
    const TCHAR szMutexName[] = _T("__wibex__mutex__");

    HANDLE hMutex = CreateMutex(NULL, FALSE, szMutexName);
    if (hMutex == NULL)
    {
        MessageBox(NULL, _T("Could not create mutex."), _T("Error!"), MB_OK | MB_ICONERROR | MB_TOPMOST);
        return 0;
    }
    else
    {
        if (GetLastError() == ERROR_ALREADY_EXISTS)
        {
            //MessageBox(NULL, _T("Экземпляр программы уже запущен."), _T("Ошибка!"), MB_OK | MB_ICONERROR | MB_TOPMOST);

            CloseHandle(hMutex);

            HWND hWnd = FindWindow(szClassName, NULL);
            if (hWnd != NULL)
            {
                if (IsIconic(hWnd))
                {
                    ShowWindow(hWnd, SW_RESTORE);
                }
                SetForegroundWindow(hWnd);
            }
            return 0;
        }
    }

    InitCommonControls();

    LoadLibrary(_T("riched32.dll"));

    HANDLE hProcessToken = NULL;
    OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hProcessToken);
    if (!hProcessToken || !SetPrivilege(hProcessToken, _T("SeDebugPrivilege"), TRUE))
    {
        MessageBox(NULL, _T("Не удалось установить привилегию SeDebugPrivilege."), _T("Ошибка!"), MB_OK | MB_ICONERROR | MB_TOPMOST);
        ReleaseMutex(hMutex);
        CloseHandle(hMutex);
        return 0;
    }

    WSADATA wd;
    if (WSAStartup(MAKEWORD(2, 2), &wd) != 0)
    {
        MessageBox(NULL, _T("WSAStartup failed."), _T("Error!"), MB_OK | MB_ICONERROR | MB_TOPMOST);
        SetPrivilege(hProcessToken, _T("SeDebugPrivilege"), FALSE);
        ReleaseMutex(hMutex);
        CloseHandle(hMutex);
        return 0;
    }

    if (LOBYTE(wd.wVersion) != 2 || HIBYTE(wd.wVersion) != 2)
    {
        MessageBox(NULL, _T("Could not find a usable version of Winsock.dll"), _T("Error!"), MB_OK | MB_ICONERROR | MB_TOPMOST);
        WSACleanup();
        SetPrivilege(hProcessToken, _T("SeDebugPrivilege"), FALSE);
        ReleaseMutex(hMutex);
        CloseHandle(hMutex);
        return 1;
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
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON));
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = MAKEINTRESOURCE(IDM_MENU);
    wcex.lpszClassName = szClassName;
    wcex.hIconSm = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON));

    if (!RegisterClassEx(&wcex))
    {
        MessageBox(NULL, _T("Window registration Failed."), _T("Error!"), MB_OK | MB_ICONERROR | MB_TOPMOST);
        WSACleanup();
        SetPrivilege(hProcessToken, _T("SeDebugPrivilege"), FALSE);
        ReleaseMutex(hMutex);
        CloseHandle(hMutex);
        return 0;
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
        MessageBox(NULL, _T("Window creation failed."), _T("Error!"), MB_OK | MB_ICONERROR | MB_TOPMOST);
        UnregisterClass(szClassName, hInstance);
        WSACleanup();
        SetPrivilege(hProcessToken, _T("SeDebugPrivilege"), FALSE);
        ReleaseMutex(hMutex);
        CloseHandle(hMutex);
        return 0;
    }

    ShowWindow(hWnd, SW_SHOWNORMAL);
    UpdateWindow(hWnd);

    MSG msg;
    BOOL bRet;
    while ((bRet = GetMessage(&msg, NULL, 0, 0)) != 0)
    {
        if (bRet == -1)
        {
            break;
        }
        else
        {
            DispatchMessage(&msg);
            TranslateMessage(&msg);
        }
    }
    
    UnregisterClass(szClassName, hInstance);
    WSACleanup();
    SetPrivilege(hProcessToken, _T("SeDebugPrivilege"), FALSE);
    ReleaseMutex(hMutex);
    CloseHandle(hMutex);
    return (int)msg.wParam;
}
// [/_tWinMain]
