#ifndef _DC_H_
#define _DC_H_

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
    DC() {
        Clear(true);
    }

    ~DC() {
        Clear();
    }

    void Clear(bool init = false);

    bool CreateFromBitmap(HBITMAP hBitmap);
    bool ReplaceColors(COLORREF clrOldColor, COLORREF clrNewColor);
    bool MakeScreenShot(HWND hWnd);
    bool TransformImage(int max_width, int max_height);
    bool Draw(HDC hDC, int x, int y);
    bool RestoreSize();
    
    bool HasImage() const {
        return bIsOK;
    }

    HDC GetHDC() const {
        return bIsOK ? hDC : NULL;
    }

    SIZE GetCurrentSize() const {
        return size;
    }

    SIZE GetOriginalSize() const {
        return original_size;
    }
};

#endif
