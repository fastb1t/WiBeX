#include "DC.h"

// [DC::DC]:
DC::DC() :
    hDC(NULL),
    hBitmap(NULL),
    hOldBitmap(NULL),
    size({ 0, 0 }),
    bIsOK(false)
{

}
// [/DC::DC]


// [DC::~DC]:
DC::~DC()
{
    Clear();
}
// [/DC::~DC]:


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


bool DC::Draw(HDC hDC, int x, int y)
{
    if (!bIsOK)
        return false;
    BitBlt(hDC, x, y, size.cx, size.cy, this->hDC, 0, 0, SRCCOPY);
    return true;
}


// [DC::GetHDC]:
HDC DC::GetHDC()
{
    return bIsOK ? hDC : NULL;
}
// [/DC::GetHDC]


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
