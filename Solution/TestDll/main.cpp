#include <tchar.h>
#include <windows.h>

static HWND g_hWnd = NULL;
static HANDLE g_hThread = NULL;


// [EnumWindowsCallback]:
static BOOL CALLBACK EnumWindowsCallback(HWND hWnd, LPARAM lParam)
{
    DWORD dwPID = 0;
    if (GetWindowThreadProcessId(hWnd, &dwPID))
    {
        if (dwPID == (DWORD)lParam)
        {
            g_hWnd = hWnd;
            return FALSE;
        }
    }
    return TRUE;
}
// [/EnumWindowsCallback]


// [Init_Thread]:
static DWORD WINAPI Init_Thread(LPVOID lpObject)
{
    HANDLE hProcess = GetCurrentProcess();
    if (hProcess)
    {
        DWORD dwPID = GetProcessId(hProcess);
        if (dwPID)
        {
            if (!EnumWindows(&EnumWindowsCallback, dwPID))
            {
                if (g_hWnd != NULL)
                {
                    HDC hDC = GetDC(g_hWnd);
                    if (hDC)
                    {
                        RECT rc;
                        GetClientRect(g_hWnd, &rc);
                        FillRect(hDC, &rc, (HBRUSH)GetStockObject(LTGRAY_BRUSH));

                        int iOldBkMode = SetBkMode(hDC, TRANSPARENT);
                        COLORREF clrOldColor = SetTextColor(hDC, RGB(255, 0, 0));
                        const TCHAR szText[] = _T("We are hacked!");
                        DrawText(hDC, szText, lstrlen(szText), &rc, DT_CENTER);
                        SetTextColor(hDC, clrOldColor);
                        SetBkMode(hDC, iOldBkMode);
                        
                        ReleaseDC(g_hWnd, hDC);
                    }
                    MessageBox(g_hWnd, _T("Oops..."), _T("Oops..."), MB_OK | MB_ICONINFORMATION | MB_TOPMOST);
                }
            }
        }
    }

    ExitThread(1337);
}
// [/Init_Thread]


// [DllMain]: entry point.
BOOL APIENTRY DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
    switch (dwReason)
    {
    case DLL_PROCESS_ATTACH:
    {
        DWORD dwThreadId = 0;
        g_hThread = CreateThread(NULL, 0, Init_Thread, NULL, 0, &dwThreadId);
    }
    break;

    case DLL_PROCESS_DETACH:
    {
        if (g_hThread)
        {
            CloseHandle(g_hThread);
        }
    }
    break;

    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
        break;
    }
    return TRUE;
}
// [/DllMain]
