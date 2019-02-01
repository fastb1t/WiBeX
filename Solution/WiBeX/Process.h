#pragma once

#include <string>
#include <windows.h>
#include <windowsx.h>

#include "algorithms.h"

class Process {
    HWND hWnd;
    DWORD dwPID;

    RECT window_rect;

    String class_name;
    String title;
    String exe_name;
    String full_path;
    String execute_time;

public:
    Process() {
        this->clear();
    }

    Process(const Process &);

    ~Process() {
        this->clear();
    }

    void clear();
    bool isValid();
    bool setWindow(HWND hClientWnd);

    bool refresh_WindowRect();
    bool refresh_WindowClassName();
    bool refresh_WindowTitle();
    bool refresh_ProcessID();
    bool refresh_Path();
    bool refresh_ExecuteTime();

    HWND getWindow() const {
        return this->hWnd;
    }

    DWORD getPID_D() const {
        return this->dwPID;
    }

    String GetXPos() const {
        return to_String(this->window_rect.left);
    }

    String GetYPos() const {
        return to_String(this->window_rect.top);
    }

    String GetWidth() const {
        return to_String(this->window_rect.right - this->window_rect.left);
    }

    String GetHeight() const {
        return to_String(this->window_rect.bottom - this->window_rect.top);
    }

    String getClassName() const {
        return this->class_name;
    }

    String getTitle() const {
        return this->title;
    }

    String getExeName() const {
        return this->exe_name;
    }

    String getFullPath() const {
        return this->full_path;
    }

    String getPID() const {
        return to_String(this->dwPID);
    }

    String getExecuteTime() const {
        return this->execute_time;
    }

    void showWindow(int nCmdShow) {
        if (this->isValid())
            ShowWindow(this->hWnd, nCmdShow);
    }

    void closeWindow() {
        if (this->isValid())
            SendMessage(this->hWnd, WM_CLOSE, 0, 0);
    }
};
