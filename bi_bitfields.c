#include <windows.h>

#include "circle_bmp.h"
#include "ozora_bmp.h"

#define CLIENTWIDTH    512
#define CLIENTHEIGHT   254
#define WINDOWSTYLE   WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX

static HBITMAP cbitmap[8];
static HBITMAP obitmap[4];

static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
static void init_bitmaps(HWND hwnd);
static void delete_bitmaps(void);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR szCmdLine, int iCmdShow) {
    static TCHAR szAppName[] = TEXT("testclass");
    HWND         hwnd;
    MSG          msg;
    WNDCLASS     wndclass;
    wndclass.style         = CS_HREDRAW | CS_VREDRAW;
    wndclass.lpfnWndProc   = WndProc;
    wndclass.cbClsExtra    = 0;
    wndclass.cbWndExtra    = 0;
    wndclass.hInstance     = hInstance;
    wndclass.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
    wndclass.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wndclass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wndclass.lpszMenuName  = NULL;
    wndclass.lpszClassName = szAppName;

    if (!RegisterClass(&wndclass)) {
        MessageBox(NULL, TEXT("This program requires Windows NT!"), szAppName, MB_ICONERROR);
        return 0;
    }

    RECT rect = {0, 0, CLIENTWIDTH, CLIENTHEIGHT};
    if (!AdjustWindowRect(&rect, WINDOWSTYLE, 0)) {
        MessageBox(NULL, TEXT("AdjustWindowRect() failed!"), szAppName, MB_ICONERROR);
        return 0;
    }

    hwnd = CreateWindow(szAppName,                         // window class name
                        TEXT("BI_BITFIELDS test program"), // window caption
                        WINDOWSTYLE,                       // window style
                        CW_USEDEFAULT,                     // initial x position
                        CW_USEDEFAULT,                     // initial y position
                        rect.right - rect.left,            // initial x size
                        rect.bottom - rect.top,            // initial y size
                        NULL,                              // parent window handle
                        NULL,                              // window menu handle
                        hInstance,                         // program instance handle
                        NULL);                             // creation parameters

    init_bitmaps(hwnd);

    ShowWindow(hwnd, iCmdShow);
    UpdateWindow(hwnd);

    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    delete_bitmaps();

    return msg.wParam;
}

static LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
    HDC         hdc;
    PAINTSTRUCT ps;
    RECT        rect;
    BITMAP      bm;
    int         i;

    switch (message) {
        case WM_CREATE:
            return 0;

        case WM_PAINT:
            hdc = BeginPaint(hwnd, &ps);
            GetClientRect(hwnd, &rect);

            HDC hdcMem = CreateCompatibleDC(hdc);
            HBITMAP hbmOld = SelectObject(hdcMem, cbitmap[0]);

            GetObject(cbitmap[0], sizeof(bm), &bm);
            for (i = 0; i < 8; i++) {
                SelectObject(hdcMem, cbitmap[i]);
                BitBlt(hdc, i * 64, 0, bm.bmWidth, bm.bmHeight, hdcMem, 0, 0, SRCCOPY);
            }

            GetObject(obitmap[0], sizeof(bm), &bm);
            for (i = 0; i < 4; i++) {
                SelectObject(hdcMem, obitmap[i]);
                BitBlt(hdc, i * 128, 64, bm.bmWidth, bm.bmHeight, hdcMem, 0, 0, SRCCOPY);
            }

            SelectObject(hdcMem, hbmOld);
            DeleteDC(hdcMem);

            EndPaint(hwnd, &ps);
            return 0;

        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }

    return DefWindowProc(hwnd, message, wParam, lParam);
}

static void init_bitmaps(HWND hwnd) {
    /* Create and initialite BITMAPINFOHEADER for the 32-bit image */
    BITMAPINFOHEADER bmih;
    memset(&bmih, 0, sizeof(BITMAPINFOHEADER));
    bmih.biWidth = circle_bmp.width;
    bmih.biHeight = circle_bmp.height;
    bmih.biBitCount = 32;
    bmih.biCompression = BI_BITFIELDS;
    bmih.biSize = sizeof(BITMAPINFOHEADER);
    bmih.biPlanes = 1;
    bmih.biSizeImage = sizeof(circle_bmp.pixel_data) - 1;

    /* Create and initialite BITMAPINFO for the 32-bit image */
    PBITMAPINFO pbmi = (PBITMAPINFO)calloc(1, sizeof(BITMAPINFOHEADER) + 3 * sizeof(DWORD));
    pbmi->bmiHeader.biWidth = circle_bmp.width;
    pbmi->bmiHeader.biHeight = circle_bmp.height;
    pbmi->bmiHeader.biBitCount = 32;
    pbmi->bmiHeader.biCompression = BI_BITFIELDS;
    pbmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    pbmi->bmiHeader.biPlanes = 1;
    pbmi->bmiHeader.biSizeImage = sizeof(circle_bmp.pixel_data) - 1;

    HDC hdc = GetDC(hwnd);

    /* Create the 32-bit bitmaps with different bitmasks */
    ((DWORD *)pbmi->bmiColors)[0] = 0x000000FF;
    ((DWORD *)pbmi->bmiColors)[1] = 0x0000FF00;
    ((DWORD *)pbmi->bmiColors)[2] = 0x00FF0000;
    cbitmap[0] = CreateDIBitmap(hdc, &bmih, CBM_INIT, circle_bmp.pixel_data, pbmi, DIB_RGB_COLORS);

    ((DWORD *)pbmi->bmiColors)[0] = 0x00FF0000;
    ((DWORD *)pbmi->bmiColors)[1] = 0x000000FF;
    ((DWORD *)pbmi->bmiColors)[2] = 0x0000FF00;
    cbitmap[1] = CreateDIBitmap(hdc, &bmih, CBM_INIT, circle_bmp.pixel_data, pbmi, DIB_RGB_COLORS);

    ((DWORD *)pbmi->bmiColors)[0] = 0x0000FF00;
    ((DWORD *)pbmi->bmiColors)[1] = 0x00FF0000;
    ((DWORD *)pbmi->bmiColors)[2] = 0x000000FF;
    cbitmap[2] = CreateDIBitmap(hdc, &bmih, CBM_INIT, circle_bmp.pixel_data, pbmi, DIB_RGB_COLORS);

    ((DWORD *)pbmi->bmiColors)[0] = 0x00FFFFFF;
    ((DWORD *)pbmi->bmiColors)[1] = 0x00FFFFFF;
    ((DWORD *)pbmi->bmiColors)[2] = 0x00FFFFFF;
    cbitmap[3] = CreateDIBitmap(hdc, &bmih, CBM_INIT, circle_bmp.pixel_data, pbmi, DIB_RGB_COLORS);

    ((DWORD *)pbmi->bmiColors)[0] = 0b00000000000000000000000011000000;
    ((DWORD *)pbmi->bmiColors)[1] = 0b00000000000000001100000000000000;
    ((DWORD *)pbmi->bmiColors)[2] = 0b00000000110000000000000000000000;
    cbitmap[4] = CreateDIBitmap(hdc, &bmih, CBM_INIT, circle_bmp.pixel_data, pbmi, DIB_RGB_COLORS);

    ((DWORD *)pbmi->bmiColors)[0] = 0b00000000111000001110000011100000;
    ((DWORD *)pbmi->bmiColors)[1] = 0b00000000000111000001110000011100;
    ((DWORD *)pbmi->bmiColors)[2] = 0b00000000000000110000001100000011;
    cbitmap[5] = CreateDIBitmap(hdc, &bmih, CBM_INIT, circle_bmp.pixel_data, pbmi, DIB_RGB_COLORS);

    ((DWORD *)pbmi->bmiColors)[0] = 0b00000000110000110000110000110000;
    ((DWORD *)pbmi->bmiColors)[1] = 0b00000000001100001100001100001100;
    ((DWORD *)pbmi->bmiColors)[2] = 0b00000000000011000011000011000011;
    cbitmap[6] = CreateDIBitmap(hdc, &bmih, CBM_INIT, circle_bmp.pixel_data, pbmi, DIB_RGB_COLORS);

    ((DWORD *)pbmi->bmiColors)[0] = 0b00000000100100100100100100100100;
    ((DWORD *)pbmi->bmiColors)[1] = 0b00000000010010010010010010010010;
    ((DWORD *)pbmi->bmiColors)[2] = 0b00000000001001001001001001001001;
    cbitmap[7] = CreateDIBitmap(hdc, &bmih, CBM_INIT, circle_bmp.pixel_data, pbmi, DIB_RGB_COLORS);

    /* Initialite BITMAPINFOHEADER for the 16-bit RGB565 image */
    bmih.biWidth = ozora_bmp.width;
    bmih.biHeight = ozora_bmp.height;
    bmih.biBitCount = 16;
    bmih.biCompression = BI_BITFIELDS;
    bmih.biSize = sizeof(BITMAPINFOHEADER);
    bmih.biPlanes = 1;
    bmih.biSizeImage = sizeof(ozora_bmp.pixel_data) - 1;

    /* Initialite BITMAPINFO for the 16-bit RGB565 image */
    pbmi->bmiHeader.biWidth = ozora_bmp.width;
    pbmi->bmiHeader.biHeight = ozora_bmp.height;
    pbmi->bmiHeader.biBitCount = 16;
    pbmi->bmiHeader.biCompression = BI_BITFIELDS;
    pbmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    pbmi->bmiHeader.biPlanes = 1;
    pbmi->bmiHeader.biSizeImage = sizeof(ozora_bmp.pixel_data) - 1;

    /* Create the 16-bit RGB565 bitmaps with different bitmasks */
    ((DWORD *)pbmi->bmiColors)[0] = 0b1111100000000000;
    ((DWORD *)pbmi->bmiColors)[1] = 0b0000011111100000;
    ((DWORD *)pbmi->bmiColors)[2] = 0b0000000000011111;
    obitmap[0] = CreateDIBitmap(hdc, &bmih, CBM_INIT, ozora_bmp.pixel_data, pbmi, DIB_RGB_COLORS);
    ((DWORD *)pbmi->bmiColors)[0] = 0xffff;
    ((DWORD *)pbmi->bmiColors)[1] = 0xffff;
    ((DWORD *)pbmi->bmiColors)[2] = 0xffff;
    obitmap[1] = CreateDIBitmap(hdc, &bmih, CBM_INIT, ozora_bmp.pixel_data, pbmi, DIB_RGB_COLORS);
    ((DWORD *)pbmi->bmiColors)[0] = 0b1111100000000000;
    ((DWORD *)pbmi->bmiColors)[1] = 0b0000011111111111;
    ((DWORD *)pbmi->bmiColors)[2] = 0b0000011111111111;
    obitmap[2] = CreateDIBitmap(hdc, &bmih, CBM_INIT, ozora_bmp.pixel_data, pbmi, DIB_RGB_COLORS);
    ((DWORD *)pbmi->bmiColors)[0] = 0b1100000000000000;
    ((DWORD *)pbmi->bmiColors)[1] = 0b0000011000000000;
    ((DWORD *)pbmi->bmiColors)[2] = 0b0000000000011000;
    obitmap[3] = CreateDIBitmap(hdc, &bmih, CBM_INIT, ozora_bmp.pixel_data, pbmi, DIB_RGB_COLORS);

    ReleaseDC(hwnd, hdc);

    /* Free allocated memory */
    free(pbmi);
}

static void delete_bitmaps(void) {
    for (int i = 0; i < 8; i++)
        DeleteObject(cbitmap[i]);
    for (int i = 0; i < 4; i++)
        DeleteObject(obitmap[i]);
}
