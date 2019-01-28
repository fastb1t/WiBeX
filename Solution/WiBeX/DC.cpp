#include "DC.h"

// [DC::DC]:
DC::DC() :
    hDC(NULL),
    hBitmap(NULL),
    hOldBitmap(NULL),
    size({ 0, 0 }),
    original_size({ 0, 0 }),
    bIsOK(false),
    bIsScaled(false)
{

}
// [/DC::DC]


// [DC::~DC]:
DC::~DC()
{
    Clear();
}
// [/DC::~DC]


// [DC::Clear]:
void DC::Clear()
{
    if (hDC && hOldBitmap)
        SelectObject(hDC, hOldBitmap);
    if (hDC)
        DeleteDC(hDC);
    if (hBitmap)
        DeleteObject(hBitmap);
    hDC = NULL;
    hBitmap = NULL;
    hOldBitmap = NULL;
    size = { 0, 0 };
    bIsOK = false;
}
// [/DC::Clear]


// [DC::CreateFromBitmap]:
bool DC::CreateFromBitmap(HBITMAP hBitmap)
{
    if (!hBitmap)
        return false;

    HDC hParentDC = GetDC(NULL);
    if (hParentDC)
    {
        this->hBitmap = hBitmap;
        hDC = CreateCompatibleDC(hParentDC);
        hOldBitmap = (HBITMAP)SelectObject(hDC, this->hBitmap);

        BITMAP bm;
        if (GetObject(this->hBitmap, sizeof(BITMAP), &bm))
        {
            size = { bm.bmWidth, bm.bmHeight };
            original_size = { bm.bmWidth, bm.bmHeight };
            bIsOK = true;
        }
        ReleaseDC(NULL, hParentDC);
        return true;
    }
    return false;
}
// [/DC::CreateFromBitmap]


// [DC::ReplaceColors]:
bool DC::ReplaceColors(COLORREF clrOldColor, COLORREF clrNewColor)
{
    if (!bIsOK)
        return false;

    BITMAPINFO bi;
    RtlZeroMemory(&bi, sizeof(BITMAPINFO));

    bi.bmiHeader.biSize = sizeof(bi.bmiHeader);

    GetDIBits(hDC, hBitmap, 0, 0, NULL, &bi, DIB_RGB_COLORS);

    bi.bmiHeader.biCompression = BI_RGB;
    
    WORD wColorBits = bi.bmiHeader.biBitCount >> 3;
    if (!wColorBits)
        return false;
    
    BYTE *pPixels = (BYTE *)GlobalAlloc(GPTR, bi.bmiHeader.biSizeImage * wColorBits);
    if (pPixels)
    {
        GetDIBits(hDC, hBitmap, 0, bi.bmiHeader.biHeight, (LPVOID)pPixels, &bi, DIB_RGB_COLORS);

        for (long x = 0; x < size.cx; x++)
        {
            for (long y = 0; y < size.cy; y++)
            {
                COLORREF clrCurrentPixel = RGB(
                    pPixels[wColorBits * ((size.cy - y - 1) * size.cx + x) + 2],
                    pPixels[wColorBits * ((size.cy - y - 1) * size.cx + x) + 1],
                    pPixels[wColorBits * ((size.cy - y - 1) * size.cx + x)]
                );

                if (clrCurrentPixel == clrOldColor)
                    SetPixel(hDC, x, y, clrNewColor);
            }
        }
        GlobalFree(pPixels);
        return true;
    }
    return false;
}
// [/DC::ReplaceColors]


// [DC::MakeScreenShot]:
bool DC::MakeScreenShot(HWND hWnd)
{
    Clear();
    HDC hParentDC = GetDC(hWnd);
    if (hParentDC)
    {
        RECT rc;
        GetClientRect(hWnd, &rc);
        size.cx = rc.right - rc.left;
        size.cy = rc.bottom - rc.top;

        original_size.cx = size.cx;
        original_size.cy = size.cy;

        hDC = CreateCompatibleDC(hParentDC);
        hBitmap = CreateCompatibleBitmap(hParentDC, size.cx, size.cy);
        hOldBitmap = (HBITMAP)SelectObject(hDC, hBitmap);
        
        BitBlt(hDC, 0, 0, size.cx, size.cy, hParentDC, 0, 0, SRCCOPY);

        bIsOK = true;

        ReleaseDC(hWnd, hParentDC);
        return true;
    }
    return false;
}
// [/DC::MakeScreenShot]


// [DC::TransformImage]:
bool DC::TransformImage(long max_width, long max_height)
{
    if (!bIsOK || max_width == 0 || max_height == 0)
        return false;

    if (size.cx > size.cy)
    {
        size.cx = max_width;
        size.cy = original_size.cy * max_width / original_size.cx;
        if (size.cy > max_height)
        {
            size.cx = size.cx * max_height / size.cy;
            size.cy = max_height;
        }
    }
    else if (size.cy > size.cx)
    {
        size.cx = original_size.cx * max_height / original_size.cy;
        size.cy = max_height;
        if (size.cx > max_width)
        {
            size.cx = max_height;
            size.cy = original_size.cy * max_width / size.cx;
        }
    }
    else
    {
        if (max_width > max_height)
        {
            size = { max_height, max_height };
        }
        else if (max_height > max_width)
        {
            size = { max_width, max_width };
        }
        else
        {
            size = { max_width, max_height };
        }
    }
    bIsScaled = true;
    return true;
}
// [/DC::TransformImage]


// [DC::Draw]:
bool DC::Draw(HDC hDC, size_t x, size_t y)
{
    if (!bIsOK || !hDC)
        return false;

    if (!bIsScaled)
        BitBlt(hDC, x, y, size.cx, size.cy, this->hDC, 0, 0, SRCCOPY);
    else
        StretchBlt(hDC, x, y, size.cx, size.cy, this->hDC, 0, 0, original_size.cx, original_size.cy, SRCCOPY);

    return true;
}
// [/DC::Draw]


// [DC::RestoreSize]:
bool DC::RestoreSize()
{
    if (size.cx == original_size.cx && size.cy == original_size.cy)
        return false;

    size.cx = original_size.cx;
    size.cy = original_size.cy;
    return true;
}
// [/DC::RestoreSize]


// [DC::HasImage]:
bool DC::HasImage()
{
    return bIsOK;
}
// [/DC::HasImage]


// [DC::GetHDC]:
HDC DC::GetHDC()
{
    return bIsOK ? hDC : NULL;
}
// [/DC::GetHDC]


// [DC::GetCurrentSize]:
SIZE DC::GetCurrentSize()
{
    return size;
}
// [/DC::GetCurrentSize]


// [DC::GetOriginalSize]:
SIZE DC::GetOriginalSize()
{
    return original_size;
}
// [/DC::GetOriginalSize]
