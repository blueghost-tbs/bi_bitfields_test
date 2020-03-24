#include <windows.h>

#define BITMAP1WIDTH   64
#define BITMAP1HEIGHT  64
#define CLIENTWIDTH   512
#define CLIENTHEIGHT  384
#define WINDOWSTYLE   WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX

static HBITMAP hbitmap[8];

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

            GetObject(hbitmap[0], sizeof(bm), &bm);
            HDC hdcMem = CreateCompatibleDC(hdc);

            HBITMAP hbmOld = SelectObject(hdcMem, hbitmap[0]);
            BitBlt(hdc, 0, 0, bm.bmWidth, bm.bmHeight, hdcMem, 0, 0, SRCCOPY);

            for (i = 1; i < 8; i++) {
                SelectObject(hdcMem, hbitmap[i]);
                BitBlt(hdc, i * 64, 0, bm.bmWidth, bm.bmHeight, hdcMem, 0, 0, SRCCOPY);
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
    int i, j;

    unsigned char *sampleimage = (unsigned char*)calloc(1, BITMAP1WIDTH * BITMAP1HEIGHT * 4);
    for (i = 0; i < BITMAP1WIDTH; i++) {
        for (j = 0; j < BITMAP1HEIGHT; j++) {
            sampleimage[(i * BITMAP1HEIGHT + j) * 4]     = (i * 4) % 255;
            sampleimage[(i * BITMAP1HEIGHT + j) * 4 + 1] = (j * 4) % 255;
            sampleimage[(i * BITMAP1HEIGHT + j) * 4 + 2] = (i + j) % 255;
            sampleimage[(i * BITMAP1HEIGHT + j) * 4 + 3] = 255;
        }
    }

    /* Create BITMAPINFOHEADER */
    BITMAPINFOHEADER bmih;
    memset(&bmih, 0, sizeof(BITMAPINFOHEADER));
    bmih.biWidth = BITMAP1WIDTH;
    bmih.biHeight = BITMAP1HEIGHT;
    bmih.biBitCount = 32;
    bmih.biCompression = BI_BITFIELDS;
    bmih.biSize = sizeof(BITMAPINFOHEADER);
    bmih.biPlanes = 1;
    bmih.biSizeImage = BITMAP1WIDTH * BITMAP1HEIGHT * 4;

    /* Create BITMAPINFO */
    PBITMAPINFO pbmi = (PBITMAPINFO)calloc(1, sizeof(BITMAPINFOHEADER) + 3 * sizeof(DWORD));
    pbmi->bmiHeader.biWidth = BITMAP1WIDTH;
    pbmi->bmiHeader.biHeight = BITMAP1HEIGHT;
    pbmi->bmiHeader.biBitCount = 32;
    pbmi->bmiHeader.biCompression = BI_BITFIELDS;
    pbmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    pbmi->bmiHeader.biPlanes = 1;
    pbmi->bmiHeader.biSizeImage = BITMAP1WIDTH * BITMAP1HEIGHT * 4;

    /* Create bitmaps */
    HDC hdc = GetDC(hwnd);

    ((DWORD *)pbmi->bmiColors)[0] = 0x00FF0000;
    ((DWORD *)pbmi->bmiColors)[1] = 0x0000FF00;
    ((DWORD *)pbmi->bmiColors)[2] = 0x000000FF;
    hbitmap[0] = CreateDIBitmap(hdc, &bmih, CBM_INIT, sampleimage, pbmi, DIB_RGB_COLORS);

    ((DWORD *)pbmi->bmiColors)[0] = 0x000000FF;
    ((DWORD *)pbmi->bmiColors)[1] = 0x00FF0000;
    ((DWORD *)pbmi->bmiColors)[2] = 0x0000FF00;
    hbitmap[1] = CreateDIBitmap(hdc, &bmih, CBM_INIT, sampleimage, pbmi, DIB_RGB_COLORS);

    ((DWORD *)pbmi->bmiColors)[0] = 0x0000FF00;
    ((DWORD *)pbmi->bmiColors)[1] = 0x000000FF;
    ((DWORD *)pbmi->bmiColors)[2] = 0x00FF0000;
    hbitmap[2] = CreateDIBitmap(hdc, &bmih, CBM_INIT, sampleimage, pbmi, DIB_RGB_COLORS);

    ((DWORD *)pbmi->bmiColors)[0] = 0b00000000111111111000000000000000;
    ((DWORD *)pbmi->bmiColors)[1] = 0b00000000000000000111111100000000;
    ((DWORD *)pbmi->bmiColors)[2] = 0b00000000000000000000000011111111;
    hbitmap[3] = CreateDIBitmap(hdc, &bmih, CBM_INIT, sampleimage, pbmi, DIB_RGB_COLORS);

    ((DWORD *)pbmi->bmiColors)[0] = 0b00000000111111111000000000000000;
    ((DWORD *)pbmi->bmiColors)[1] = 0b00000000000000000111111111000000;
    ((DWORD *)pbmi->bmiColors)[2] = 0b00000000000000000000000000111111;
    hbitmap[4] = CreateDIBitmap(hdc, &bmih, CBM_INIT, sampleimage, pbmi, DIB_RGB_COLORS);

    ((DWORD *)pbmi->bmiColors)[0] = 0b00000000111000001110000011100000;
    ((DWORD *)pbmi->bmiColors)[1] = 0b00000000000111000001110000011100;
    ((DWORD *)pbmi->bmiColors)[2] = 0b00000000000000110000001100000011;
    hbitmap[5] = CreateDIBitmap(hdc, &bmih, CBM_INIT, sampleimage, pbmi, DIB_RGB_COLORS);

    ((DWORD *)pbmi->bmiColors)[0] = 0b00000000110000110000110000110000;
    ((DWORD *)pbmi->bmiColors)[1] = 0b00000000001100001100001100001100;
    ((DWORD *)pbmi->bmiColors)[2] = 0b00000000000011000011000011000011;
    hbitmap[6] = CreateDIBitmap(hdc, &bmih, CBM_INIT, sampleimage, pbmi, DIB_RGB_COLORS);

    ((DWORD *)pbmi->bmiColors)[0] = 0b00000000100100100100100100100100;
    ((DWORD *)pbmi->bmiColors)[1] = 0b00000000010010010010010010010010;
    ((DWORD *)pbmi->bmiColors)[2] = 0b00000000001001001001001001001001;
    hbitmap[7] = CreateDIBitmap(hdc, &bmih, CBM_INIT, sampleimage, pbmi, DIB_RGB_COLORS);

    ReleaseDC(hwnd, hdc);

    /* Free allocated memory */
    free(sampleimage);
    free(pbmi);
}

static void delete_bitmaps(void) {
    for (int i = 0; i < 8; i++)
        DeleteObject(hbitmap[i]);
}
