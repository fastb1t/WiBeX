#include "Process.h"

// [Process::Process]:
Process::Process(const Process &process)
{
    this->hWnd = process.hWnd;
    this->dwPID = process.dwPID;
    this->window_rect = process.window_rect;
    this->class_name = process.class_name;
    this->title = process.title;
    this->exe_name = process.exe_name;
    this->full_path = process.full_path;
    this->execute_time = process.execute_time;
}
// [/Process::Process]


// [Process::clear]: очищаєм всю інформацію.
void Process::clear()
{
    this->hWnd = NULL;
    this->dwPID = 0;
    this->window_rect = { 0, 0, 0, 0 };
    this->class_name = _T("");
    this->title = _T("");
    this->exe_name = _T("");
    this->full_path = _T("");
    this->execute_time = _T("");
}
// [/Process::clear]


// [Process::isValid]: перевіряєм на валідність.
bool Process::isValid()
{
    if (!this->hWnd)
        return false;
    if (!IsWindow(hWnd))
        return false;
    return true;
}
// [/Process::isValid]


// [Process::setWindow]: встановлюєм дескриптор вікна і оновлюєм всю інформацію про процес.
bool Process::setWindow(HWND hClientWnd)
{
    this->clear();
    if (hClientWnd && IsWindow(hClientWnd))
    {
        this->hWnd = hClientWnd;

        this->refresh_WindowRect();
        this->refresh_WindowClassName();
        this->refresh_WindowTitle();
        this->refresh_ProcessID();
        this->refresh_Path();
        this->refresh_ExecuteTime();

        return true;
    }
    return false;
}
// [/Process::setWindow]


// [Process::refresh_WindowRect]: получаєм розмір вікна.
bool Process::refresh_WindowRect()
{
    this->window_rect = { 0, 0, 0, 0 };

    if (this->isValid())
        if (GetWindowRect(this->hWnd, &this->window_rect))
            return true;
    return false;
}
// [/Process::refresh_WindowRect]


// [Process::refresh_WindowClassName]: получаєм ім'я класу вікна.
bool Process::refresh_WindowClassName()
{
    this->class_name = _T("");

    if (this->isValid())
    {
        TCHAR szTemp[1024];
        memset(szTemp, 0, sizeof(szTemp));
        if (GetClassName(this->hWnd, szTemp, sizeof(szTemp) / sizeof(TCHAR)))
        {
            this->class_name = szTemp;
            return true;
        }
    }
    return false;
}
// [/Process::refresh_WindowClassName]


// [Process::refresh_WindowCaption]: получаєм заголовок вікна.
bool Process::refresh_WindowTitle()
{
    this->title = _T("");

    if (this->isValid())
    {
        TCHAR szTemp[1024];
        memset(szTemp, 0, sizeof(szTemp));
        if (GetWindowText(hWnd, szTemp, sizeof(szTemp) / sizeof(TCHAR)))
        {
            this->title = szTemp;
            return true;
        }
    }
    return false;
}
// [/Process::refresh_WindowCaption]


// [Process::refresh_ProcessID]: получаєм PID процесу.
bool Process::refresh_ProcessID()
{
    this->dwPID = 0;

    if (this->isValid())
    {
        DWORD dwTemp = 0;
        GetWindowThreadProcessId(this->hWnd, &dwTemp);
        if (dwTemp)
        {
            this->dwPID = dwTemp;
            return true;
        }
    }
    return false;
}
// [/Process::refresh_ProcessID]


// [Process::refresh_Path]: получаєм повнний шлях до процесу і його назву.
bool Process::refresh_Path()
{
    bool bRetVal = false;

    this->exe_name = _T("");
    this->full_path = _T("");

    if (this->isValid() && this->dwPID)
    {
        HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, this->dwPID);
        if (hProcess)
        {
            TCHAR szTemp[1024] = { 0 };
            DWORD dwSize = sizeof(szTemp) / sizeof(TCHAR) - 1;
            if (QueryFullProcessImageName(hProcess, 0, szTemp, &dwSize))
            {
                this->exe_name = szTemp;
                this->full_path = szTemp;

                size_t p = 0;
                p = this->exe_name.rfind('\\');
                if (p != std::string::npos)
                    this->exe_name.erase(0, p + 1);

                p = this->full_path.rfind('\\');
                if (p != std::string::npos)
                    this->full_path.erase(p, this->full_path.length());

                bRetVal = true;
            }
            CloseHandle(hProcess);
        }
    }
    return bRetVal;
}
// [/Process::refresh_Path]


// [Process::refresh_RunDate]: получаєм дату запуску процесу.
bool Process::refresh_ExecuteTime()
{
    bool bRetVal = false;

    this->execute_time = _T("");

    if (this->isValid() && this->dwPID)
    {
        HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, this->dwPID);
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
                memset(szTemp, 0, 256);
                wsprintf(szTemp, _T("%s%d.%s%d.%d %s%d:%s%d:%s%d"),
                    system_time_local[0].wDay <= 9 ? _T("0") : _T(""), system_time_local[0].wDay,
                    system_time_local[0].wMonth <= 9 ? _T("0") : _T(""), system_time_local[0].wMonth,
                    system_time_local[0].wYear,
                    system_time_local[0].wHour <= 9 ? _T("0") : _T(""), system_time_local[0].wHour,
                    system_time_local[0].wMinute <= 9 ? _T("0") : _T(""), system_time_local[0].wMinute,
                    system_time_local[0].wSecond <= 9 ? _T("0") : _T(""), system_time_local[0].wSecond);
                this->execute_time = szTemp;
                bRetVal = true;
            }
            CloseHandle(hProcess);
        }
    }
    return bRetVal;
}
// [/Process::refresh_RunDate]
