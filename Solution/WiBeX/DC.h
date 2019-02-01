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
    bool TransformImage(int max_width, int max_height);
    bool Draw(HDC hDC, int x, int y);
    bool RestoreSize();
    
    bool HasImage();
    HDC GetHDC();
    SIZE GetCurrentSize();
    SIZE GetOriginalSize();
};
