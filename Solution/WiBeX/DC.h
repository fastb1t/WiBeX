#pragma once

#include <windows.h>

class DC {
    HDC hDC;
    HBITMAP hBitmap;
    HBITMAP hOldBitmap;
    SIZE size;
    SIZE original_size;
    bool bIsOK;
    bool bIsScaled;

public:
    DC();
    ~DC();

    void Clear();

    bool CreateFromBitmap(HBITMAP hBitmap);
    bool ReplaceColors(COLORREF clrOldColor, COLORREF clrNewColor);
    bool MakeScreenShot(HWND hWnd);
    bool TransformImage(long max_width, long max_height);
    bool Draw(HDC hDC, size_t x, size_t y);
    bool RestoreSize();
    
    bool HasImage();
    HDC GetHDC();
    SIZE GetCurrentSize();
    SIZE GetOriginalSize();
};
