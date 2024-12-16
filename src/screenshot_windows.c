#include "screenshot.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

static bool save_bitmap_to_file(HBITMAP h_bitmap, HDC h_dc, const char* file_name)
{
    BITMAP bmp;
    GetObject(h_bitmap, sizeof(BITMAP), &bmp);

    BITMAPINFOHEADER bih;
    memset(&bih, 0, sizeof(bih));
    bih.biSize = sizeof(BITMAPINFOHEADER);
    bih.biWidth = bmp.bmWidth;
    bih.biHeight = -bmp.bmHeight;
    bih.biPlanes = 1;
    bih.biBitCount = 24;
    bih.biCompression = BI_RGB;
    bih.biSizeImage = ((((bmp.bmWidth * bih.biBitCount) + 31) & ~31) >> 3) * bmp.bmHeight;

    if (!GetDIBits(h_dc, h_bitmap, 0, bih.biHeight, NULL, (BITMAPINFO*)&bih, DIB_RGB_COLORS)) {
        fprintf(stderr, "ERROR: failed to get color data\n");
        return false;
    }

    DWORD row_size = ((bmp.bmWidth * 3 + 3) & ~3);
    DWORD data_size = row_size * bmp.bmHeight;

    unsigned char* pixels = (unsigned char*)malloc(data_size);
    memset(pixels, 0, data_size);

    BITMAPINFO bmi;
    memset(&bmi, 0, sizeof(BITMAPINFO));
    bmi.bmiHeader = bih;

    if (!GetDIBits(h_dc, h_bitmap, 0, bmp.bmHeight, pixels, &bmi, DIB_RGB_COLORS)) {
        fprintf(stderr, "ERROR: failed to get bitmap pixels\n");
        free(pixels);
        return false;
    }

    BITMAPFILEHEADER bmf_header;
    memset(&bmf_header, 0, sizeof(bmf_header));
    bmf_header.bfType = 0x4D42;
    bmf_header.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + data_size;
    bmf_header.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

    FILE* file = fopen(file_name, "wb");
    if (!file) {
        fprintf(stderr, "ERROR: failed to open file `%s`: %s\n",
                file_name, strerror(errno));
        free(pixels);
        return false;
    }

    size_t written = 0;
    written += fwrite(&bmf_header, 1, sizeof(BITMAPFILEHEADER), file);
    written += fwrite(&bih, 1, sizeof(BITMAPINFOHEADER), file);
    written += fwrite(pixels, 1, data_size, file);

    fclose(file);
    free(pixels);

    return (written == sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + data_size);
}

bool take_screenshot(const char* screenshot_file_path)
{
    int width = GetSystemMetrics(SM_CXSCREEN);
    int height = GetSystemMetrics(SM_CYSCREEN);

    HDC hdc_desktop = GetDC(NULL);
    if (!hdc_desktop) {
        fprintf(stderr, "ERROR: failed to get desktop device context\n");
        return false;
    }

    HDC hdc_mem = CreateCompatibleDC(hdc_desktop);
    if (!hdc_mem) {
        fprintf(stderr, "ERROR: failed to get compatible device context for the"
                        " desktop device context\n");
        ReleaseDC(NULL, hdc_desktop);
        return false;
    }

    HBITMAP h_bitmap = CreateCompatibleBitmap(hdc_desktop, width, height);
    if (!h_bitmap) {
        fprintf(stderr, "ERROR: failed to get compatible bitmap for the "
                        "desktop device context\n");
        DeleteDC(hdc_mem);
        ReleaseDC(NULL, hdc_desktop);
        return false;
    }

    HBITMAP h_old_bitmap = (HBITMAP)SelectObject(hdc_mem, h_bitmap);
    if (!h_old_bitmap) {
        fprintf(stderr, "ERROR: `SelectObject` call failed\n");
        DeleteObject(h_bitmap);
        DeleteDC(hdc_mem);
        ReleaseDC(NULL, hdc_desktop);
        return false;
    }

    if (!BitBlt(hdc_mem, 0, 0, width, height, hdc_desktop, 0, 0, SRCCOPY)) {
        fprintf(stderr, "ERROR: failed to copy bitmap from the desktop device context\n");
        SelectObject(hdc_mem, h_old_bitmap);
        DeleteObject(h_bitmap);
        DeleteDC(hdc_mem);
        ReleaseDC(NULL, hdc_desktop);
        return false;
    }

    bool success = save_bitmap_to_file(h_bitmap, hdc_mem, screenshot_file_path);

    SelectObject(hdc_mem, h_old_bitmap);
    DeleteObject(h_bitmap);
    DeleteDC(hdc_mem);
    ReleaseDC(NULL, hdc_desktop);

    return success;
}
