#include "Process.h"

// [Process::Process]:
Process::Process(const Process &process)
{
    hWnd = process.hWnd;
    dwPID = process.dwPID;

    memcpy(&window_rect, &process.window_rect, sizeof(RECT));
    
    class_name = process.class_name;
    title = process.title;
    exe_name = process.exe_name;
    full_path = process.full_path;
    execute_time = process.execute_time;
}
// [/Process::Process]


// [Process::Clear]:
void Process::Clear()
{
    hWnd = NULL;
    dwPID = 0;

    RtlZeroMemory(&window_rect, sizeof(RECT));
    
    class_name = _T("");
    title = _T("");
    exe_name = _T("");
    full_path = _T("");
    execute_time = _T("");
}
// [/Process::Clear]


// [Process::isValid]:
bool Process::isValid()
{
    if (!hWnd)
        return false;
    if (!IsWindow(hWnd))
        return false;
    return true;
}
// [/Process::isValid]


// [Process::setWindow]:
bool Process::setWindow(HWND hClientWnd)
{
    Clear();
    if (hClientWnd && IsWindow(hClientWnd))
    {
        hWnd = hClientWnd;

        refresh_WindowRect();
        refresh_WindowClassName();
        refresh_WindowTitle();
        refresh_ProcessID();
        refresh_Path();
        refresh_ExecuteTime();

        return true;
    }
    return false;
}
// [/Process::setWindow]


// [Process::refresh_WindowRect]:
bool Process::refresh_WindowRect()
{
    RtlZeroMemory(&window_rect, sizeof(RECT));

    if (isValid())
    {
        if (GetWindowRect(hWnd, &window_rect))
            return true;
    }
    return false;
}
// [/Process::refresh_WindowRect]


// [Process::refresh_WindowClassName]:
bool Process::refresh_WindowClassName()
{
    class_name = _T("");

    if (isValid())
    {
        TCHAR szTemp[1024];
        memset(szTemp, 0, sizeof(szTemp));
        if (GetClassName(hWnd, szTemp, sizeof(szTemp) / sizeof(TCHAR)))
        {
            class_name = szTemp;
            return true;
        }
    }
    return false;
}
// [/Process::refresh_WindowClassName]


// [Process::refresh_WindowCaption]:
bool Process::refresh_WindowTitle()
{
    title = _T("");

    if (isValid())
    {
        TCHAR szTemp[1024];
        memset(szTemp, 0, sizeof(szTemp));
        if (GetWindowText(hWnd, szTemp, sizeof(szTemp) / sizeof(TCHAR)))
        {
            title = szTemp;
            return true;
        }
    }
    return false;
}
// [/Process::refresh_WindowCaption]


// [Process::refresh_ProcessID]:
bool Process::refresh_ProcessID()
{
    dwPID = 0;

    if (isValid())
    {
        DWORD dwTemp = 0;
        GetWindowThreadProcessId(hWnd, &dwTemp);
        if (dwTemp)
        {
            dwPID = dwTemp;
            return true;
        }
    }
    return false;
}
// [/Process::refresh_ProcessID]


// [Process::refresh_Path]:
bool Process::refresh_Path()
{
    bool bRetVal = false;

    exe_name = _T("");
    full_path = _T("");
    
    if (isValid() && dwPID)
    {
        HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, dwPID);
        if (hProcess)
        {
            TCHAR szTemp[1024] = { 0 };
            DWORD dwSize = sizeof(szTemp) / sizeof(TCHAR) - 1;
            if (QueryFullProcessImageName(hProcess, 0, szTemp, &dwSize))
            {
                exe_name = szTemp;
                full_path = szTemp;

                size_t p = 0;
                p = exe_name.rfind('\\');
                if (p != std::string::npos)
                    exe_name.erase(0, p + 1);

                p = full_path.rfind('\\');
                if (p != std::string::npos)
                    full_path.erase(p, full_path.length());

                bRetVal = true;
            }
            CloseHandle(hProcess);
        }
    }
    return bRetVal;
}
// [/Process::refresh_Path]


// [Process::refresh_ExecuteTime]:
bool Process::refresh_ExecuteTime()
{
    bool bRetVal = false;

    execute_time = _T("");

    if (isValid() && dwPID)
    {
        HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, dwPID);
        if (hProcess)
        {
            FILETIME file_time[4];
            if (GetProcessTimes(hProcess, &file_time[0], &file_time[1], &file_time[2], &file_time[3]))
            {
                SYSTEMTIME system_time_utc[4];
                SYSTEMTIME system_time_local[4];

                for (int i = 0; i < 4; i++)
                {
                    FileTimeToSystemTime(&file_time[i], &system_time_utc[i]);
                    SystemTimeToTzSpecificLocalTime(NULL, &system_time_utc[i], &system_time_local[i]);
                }

                TCHAR szTemp[256];
                memset(szTemp, 0, sizeof(szTemp));
                wsprintf(szTemp, _T("%s%d.%s%d.%d %s%d:%s%d:%s%d"),
                    system_time_local[0].wDay <= 9 ? _T("0") : _T(""), system_time_local[0].wDay,
                    system_time_local[0].wMonth <= 9 ? _T("0") : _T(""), system_time_local[0].wMonth,
                    system_time_local[0].wYear,
                    system_time_local[0].wHour <= 9 ? _T("0") : _T(""), system_time_local[0].wHour,
                    system_time_local[0].wMinute <= 9 ? _T("0") : _T(""), system_time_local[0].wMinute,
                    system_time_local[0].wSecond <= 9 ? _T("0") : _T(""), system_time_local[0].wSecond);
                execute_time = szTemp;
                bRetVal = true;
            }
            CloseHandle(hProcess);
        }
    }
    return bRetVal;
}
// [/Process::refresh_ExecuteTime]
