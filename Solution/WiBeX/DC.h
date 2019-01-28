#pragma once

#include <windows.h>

class DC {
    HDC hDC;
    HBITMAP hBitmap;
    HBITMAP hOldBitmap;
    SIZE size;
    bool bIsOK;

public:
    DC();
    ~DC();

    bool CreateFromBitmap(HBITMAP hBitmap);
    bool ReplaceColors(COLORREF clrOldColor, COLORREF clrNewColor);
    bool Draw(HDC hDC, int x, int y);
    
    HDC GetHDC();
    void Clear();
};
