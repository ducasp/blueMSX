/*****************************************************************************
** $Source: /cygdrive/d/Private/_SVNROOT/bluemsx/blueMSX/Src/Win32/Win32ScreenShot.c,v $
**
** $Revision: 1.10 $
**
** $Date: 2008-05-09 17:21:04 $
**
** More info: http://www.bluemsx.com
**
** Copyright (C) 2003-2006 Daniel Vik, Laurent Halter
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
** 
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
**
******************************************************************************
*/
#include "Win32ScreenShot.h"
#include "Language.h"
#include "FileHistory.h"
#include "Crc32Calc.h"
#include "ziphelper.h"
#include <stdio.h>
#include <direct.h>

static char baseDir[512];
static char basePrefix[512];


// =======================================================================================================================
// PNG Stuff
// =======================================================================================================================
#ifdef __BIG_ENDIAN__
#define ntohul(ul)  (ul)
#else
#define ntohul(ul)  (((ul << 24) & 0xff000000) | ((ul << 8) & 0x00ff0000) | ((ul >> 8) & 0x0000ff00) | ((ul >> 24) & 0x000000ff))
#endif

typedef struct PNGHeader {
    DWORD dwWidth;
    DWORD dwHeight;
    BYTE  bBitDepth;
    BYTE  bColourType;
    BYTE  bCompressionType;
    BYTE  bFilterType;
    BYTE  bInterlaceType;
} PNGHeader;

#define PNG_HEADER_SIZE  13

const BYTE PngSignature[] = { 0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A };
const char PngName[] = "Software\0blueMSX";

#define PNG_IHDR 0x49484452
#define PNG_IDAT 0x49444154
#define PNG_IEND 0x49454E44
#define PNG_TEXT 0x74455874
#define PNG_SRGB 0x73524742
#define PNG_GAMA 0x67414D41
#define PNG_CHRM 0x6348524D
#define PNG_PHYS 0x70485973

static BYTE srgb[] = { 
    0x00
};

static BYTE gama[] = { 
    0x00, 0x00, 0xb1, 0x8f 
};

static BYTE chrm[] = { 
    0x00, 0x00, 0x7a, 0x26, 0x00, 0x00, 0x80, 0x84,
    0x00, 0x00, 0xfa, 0x00, 0x00, 0x00, 0x80, 0xe8,
    0x00, 0x00, 0x75, 0x30, 0x00, 0x00, 0xea, 0x60,
    0x00, 0x00, 0x3a, 0x98, 0x00, 0x00, 0x17, 0x70
};

static BYTE phys[] = { 
    0x00, 0x00, 0x0e, 0xc4, 0x00, 0x00, 0x0e, 0xc4, 0x01
};


int pngAddChunk(BYTE* dest, int type, const void* data, int length)
{
    UInt32 crc;

    *(DWORD*)(dest + 0) = ntohul(length);
    *(DWORD*)(dest + 4) = ntohul(type);
    memcpy(dest + 8, data, length);

    crc = calcCrc32(dest + 4, length + 4);
    *(DWORD*)(dest + 8 + length) = ntohul(crc);


    return length + 12;
}

void* ScreenShotPng(void* src, int srcPitch, int width, int height, int* bitmapSize)
{
    int   compressedSize;
    BYTE* compressedData;
    int   pngSize;
    BYTE* pngData;
    PNGHeader hdr;
    DWORD* srcPtr = (DWORD*)src;
    int w;
    int h;
    int rawSize = (3 * width + 1) * height;
    BYTE* rawData = (BYTE*)malloc(rawSize);
    BYTE* dstPtr = rawData;
    
    for (h = 0; h < height; h++) {
        *dstPtr++ = 0; // Default PNG filter
        for (w = 0; w < width; w++) {
            *dstPtr++ = (BYTE)(srcPtr[w] >> 16);
            *dstPtr++ = (BYTE)(srcPtr[w] >>  8);
            *dstPtr++ = (BYTE)(srcPtr[w] >>  0);
        }
        srcPtr += srcPitch;
    }
    
    compressedSize = 0;
    compressedData = zipCompress(rawData, rawSize, &compressedSize);
    free(rawData);
    if (compressedData == NULL) {
        return NULL;
    }

    pngData = malloc(compressedSize + 200);
    
    hdr.dwWidth          = ntohul(width);
    hdr.dwHeight         = ntohul(height);
    hdr.bBitDepth        = 8;
    hdr.bColourType      = 2; // RGB
    hdr.bCompressionType = 0;
    hdr.bFilterType      = 0;
    hdr.bInterlaceType   = 0;

    pngSize = 0;

    memcpy(pngData, PngSignature, sizeof(PngSignature));
    pngSize += sizeof(PngSignature);
    pngSize += pngAddChunk(pngData + pngSize, PNG_IHDR, &hdr, PNG_HEADER_SIZE);
#if 0
    pngSize += pngAddChunk(pngData + pngSize, PNG_SRGB, srgb, sizeof(srgb));
    pngSize += pngAddChunk(pngData + pngSize, PNG_GAMA, gama, sizeof(gama));
    pngSize += pngAddChunk(pngData + pngSize, PNG_CHRM, chrm, sizeof(chrm));
    pngSize += pngAddChunk(pngData + pngSize, PNG_PHYS, phys, sizeof(phys));
#endif
    pngSize += pngAddChunk(pngData + pngSize, PNG_IDAT, compressedData, compressedSize);
    pngSize += pngAddChunk(pngData + pngSize, PNG_TEXT, PngName, sizeof(PngName)-1);
    pngSize += pngAddChunk(pngData + pngSize, PNG_IEND, NULL, 0);

    free(compressedData);

    *bitmapSize = pngSize;
    return pngData;
}

// =======================================================================================================================
// BMP Stuff
// =======================================================================================================================


typedef struct {
	DWORD Size;
    DWORD Reserved1;
    DWORD OffRaster;
    DWORD OffBits;
    DWORD Width;
    DWORD Height;
	WORD  Planes;
    WORD  BitCount;
	DWORD Compression;
    DWORD SizeImage;
    DWORD XPelsPerMeter;
    DWORD YPelsPerMeter;
    DWORD ClrUsed;
    DWORD ClrImportant;
} BMPHeader;

void* ScreenShotBmp(void* src, int srcPitch, int width, int height, int* bitmapSize)
{
    DWORD* srcPtr = (DWORD*)src;
    BMPHeader hdr;
    int w;
    int h;
    int size = 2 + sizeof(hdr) + 3 * width * height;
    BYTE* bitmap = (BYTE*)malloc(size);
    BYTE* dstPtr = bitmap;

    *bitmapSize = size;

	hdr.BitCount      = 24;
	hdr.Size          = width * height * hdr.BitCount / 8 + 0x36;
	hdr.Reserved1     = 0;
	hdr.OffRaster     = 0x36;
	hdr.OffBits       = 0x28;
	hdr.Width         = width;
	hdr.Height        = height;
	hdr.Planes        = 1;
	hdr.Compression   = 0;
	hdr.SizeImage     = width * height * hdr.BitCount / 8;
	hdr.XPelsPerMeter = 0;
	hdr.YPelsPerMeter = 0;
	hdr.ClrUsed       = 0;
    hdr.ClrImportant  = 0;

    *dstPtr++ = 'B';
    *dstPtr++ = 'M';

    memcpy(dstPtr, &hdr, sizeof(hdr));
    dstPtr += sizeof(hdr);

    for (h = 0; h < height; h++) {
        for (w = 0; w < width; w++) {
            *dstPtr++ = (BYTE)(srcPtr[w] >>  0);
            *dstPtr++ = (BYTE)(srcPtr[w] >>  8);
            *dstPtr++ = (BYTE)(srcPtr[w] >> 16);
        }
        srcPtr += srcPitch;
    }

    return bitmap;
}


// =======================================================================================================================
// Other Stuff
// =======================================================================================================================

static HRESULT SavePngBitmap(char *strFileName, PBITMAPINFO pbi, HBITMAP hBMP, HDC hDC)
{
    PBITMAPINFOHEADER   pbih;       // bitmap info-header
    LPBYTE              lpBits;     // memorypointer
    int   compressedSize;
    BYTE* compressedData;
    int   pngSize;
    BYTE* pngData;
    FILE* file;

    if( pbi == NULL ){
        return 0;
    }

    pbih    = ( PBITMAPINFOHEADER ) pbi;
    if( pbih->biBitCount != 24 ) {
        return 0;
    }

    lpBits  = ( LPBYTE ) GlobalAlloc( GMEM_FIXED, pbih->biSizeImage );

    if( !lpBits ){
        return 0;
    }

    //
    // Retrieve the colortable ( RGBQUAD-array ) and the bits
    // ( array of palette indices ) from the DIB.
    //
    if( !GetDIBits( hDC, hBMP, 0, ( WORD )pbih->biHeight, lpBits, pbi, DIB_RGB_COLORS ) ){
        return 0;
    }

    compressedSize = 0;
    compressedData = zipCompress(lpBits, pbih->biSizeImage, &compressedSize);
    if (compressedData != NULL) {
        PNGHeader h;

        pngData = malloc(compressedSize + 100);
        
        h.dwWidth          = ntohul(pbih->biWidth);
        h.dwHeight         = ntohul(pbih->biHeight);
        h.bBitDepth        = 8;
        h.bColourType      = 2; // RGB
        h.bCompressionType = 0;
        h.bFilterType      = 0;
        h.bInterlaceType   = 0;

        pngSize = 0;

        memcpy(pngData, PngSignature, sizeof(PngSignature));
        pngSize += sizeof(PngSignature);
        pngSize += pngAddChunk(pngData + pngSize, PNG_IHDR, &h, PNG_HEADER_SIZE);
        pngSize += pngAddChunk(pngData + pngSize, PNG_IDAT, compressedData, compressedSize);
        pngSize += pngAddChunk(pngData + pngSize, PNG_TEXT, PngName, sizeof(PngName)-1);
        pngSize += pngAddChunk(pngData + pngSize, PNG_IEND, NULL, 0);

        free(compressedData);

        file = fopen(strFileName, "wb");
        if (file != NULL) {
            fwrite(pngData, 1, pngSize, file);
            fclose(file);
        }

        free(pngData);
    }
    GlobalFree( ( HGLOBAL ) lpBits );

    return 1;
}

static HRESULT SaveBitmap(char *strFileName, PBITMAPINFO pbi, HBITMAP hBMP, HDC hDC)
{
    HANDLE              hf;         // file handle
    BITMAPFILEHEADER    hdr;        // bitmap file-header
    PBITMAPINFOHEADER   pbih;       // bitmap info-header
    LPBYTE              lpBits;     // memorypointer
    DWORD               dwTotal;    // total count of bytes
    DWORD               cb;         // incremental count of bytes
    BYTE                *hp;        // byte pointer
    DWORD               dwTmp;      // temp-variable


    if( pbi == NULL ){
        return 0;
    }

    pbih    = ( PBITMAPINFOHEADER ) pbi;
    lpBits  = ( LPBYTE ) GlobalAlloc( GMEM_FIXED, pbih->biSizeImage );

    if( !lpBits ){
        return 0;
    }

    if( !GetDIBits( hDC, hBMP, 0, ( WORD )pbih->biHeight, lpBits, pbi, DIB_RGB_COLORS ) ){
        return 0;
    }

    hf = CreateFile( strFileName, GENERIC_READ|GENERIC_WRITE, ( DWORD ) 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, ( HANDLE ) NULL );

    if( hf == INVALID_HANDLE_VALUE ){
        return 0;
    }

    hdr.bfType = 0x4D42;    // 0x42 = "B", 0x4D = "M"

    hdr.bfSize = ( DWORD )( sizeof( BITMAPFILEHEADER ) + pbih->biSize + pbih->biClrUsed * sizeof( RGBQUAD ) + pbih->biSizeImage );
    hdr.bfReserved1 = 0;
    hdr.bfReserved2 = 0;

    hdr.bfOffBits = ( DWORD ) sizeof( BITMAPFILEHEADER ) + pbih->biSize + pbih->biClrUsed * sizeof( RGBQUAD );

    if( !WriteFile( hf, ( LPVOID ) &hdr, sizeof( BITMAPFILEHEADER ), ( LPDWORD ) &dwTmp, NULL ) ){
        return 0;
    }

    if( !WriteFile( hf, ( LPVOID ) pbih, sizeof( BITMAPINFOHEADER ) + pbih->biClrUsed * sizeof( RGBQUAD ), ( LPDWORD ) &dwTmp, ( NULL ) ) ){
        return 0;
    }

    dwTotal = cb = pbih->biSizeImage;
    hp      = lpBits;

    if( !WriteFile( hf, ( LPSTR ) hp, ( int ) cb, ( LPDWORD ) &dwTmp, NULL ) ){
        return 0;
    }

    if( !CloseHandle( hf ) ){
        return 0;
    }

    GlobalFree( ( HGLOBAL ) lpBits );

    return 1;
}

static PBITMAPINFO CreateBitmapInfoStructure(HBITMAP hBmp)
{
    BITMAP      bmp;
    PBITMAPINFO pbmi;
    WORD        cClrBits;


    //
    // Get the width, height and the colordepth of the image
    //
    if( !GetObject( hBmp, sizeof( BITMAP ), (LPSTR)&bmp ) ){
        return NULL;
    }


    //
    // Convert the color format to a count of bits
    //
    cClrBits = 24;


    //
    // Allocate memory for the BITMAPINFO-structure. This structure
    // contains a BITMAPINFOHEADER-structure and an array of RGBQUAD
    // datastructures.
    //
    if( cClrBits < 24 )
        pbmi = ( PBITMAPINFO ) LocalAlloc( LPTR, sizeof( BITMAPINFOHEADER ) + sizeof( RGBQUAD ) * ( 1 << cClrBits ) );
    else
        pbmi = ( PBITMAPINFO ) LocalAlloc( LPTR, sizeof( BITMAPINFOHEADER ) );

    //
    // Initialize the fields in the BITMAPINFO-structure.
    //
    pbmi->bmiHeader.biSize      = sizeof( BITMAPINFOHEADER );
    pbmi->bmiHeader.biWidth     = bmp.bmWidth;
    pbmi->bmiHeader.biHeight    = bmp.bmHeight;
    pbmi->bmiHeader.biPlanes    = bmp.bmPlanes;
    pbmi->bmiHeader.biBitCount  = cClrBits;//bmp.bmBitsPixel;

    if( cClrBits < 24 )
        pbmi->bmiHeader.biClrUsed = ( 1 << cClrBits );

    //
    // If the bitmap isn't compressed, set the BI_RGB flag
    //
    pbmi->bmiHeader.biCompression   = BI_RGB;

    //
    // Compute the number of bytes in the array of color indices
    // and store the result in biSizeImage.
    //
    pbmi->bmiHeader.biSizeImage     = ( pbmi->bmiHeader.biWidth + 7 ) / 8 * pbmi->bmiHeader.biHeight * cClrBits;

    //
    // Set biClrImportant to 0 to indicate that all of the device
    // colors are important.
    //
    pbmi->bmiHeader.biClrImportant  = 0;

    //
    // And finally return the info-structure.
    //
    return pbmi;
}

int ScreenShot(Properties* properties, HWND hwnd, int width, int height, int xOffset, int yOffset, int png)
{
    HDC hdcScreen;
    HDC hdcCompatible;
    HBITMAP  hbmScreen;
    PBITMAPINFO pbi;
    HBITMAP hBitmapOrig;
    int rv = 0;

    hdcScreen=GetDC(hwnd);
    if (hdcScreen==NULL)
    {
        return 0;
    }
    hdcCompatible = CreateCompatibleDC(hdcScreen);
    if (hdcCompatible==NULL)
    {
        return 0;
    }

    hbmScreen = CreateCompatibleBitmap(hdcScreen, width, height);
    if (hbmScreen==NULL)
    {
        return 0;
    }

    hBitmapOrig = (HBITMAP)SelectObject(hdcCompatible, hbmScreen);
    BitBlt(hdcCompatible, 0, 0, width, height, hdcScreen, xOffset, yOffset, SRCCOPY);

    pbi = CreateBitmapInfoStructure(hbmScreen);
    rv = SaveBitmap(generateSaveFilename(properties, baseDir, basePrefix, ".bmp", 4), pbi, hbmScreen, hdcCompatible);
    if (pbi != NULL) {
        LocalFree(pbi);
    }

    DeleteObject(SelectObject(hdcCompatible, hBitmapOrig));
    DeleteDC(hdcCompatible);
    ReleaseDC(hwnd, hdcScreen);
    return rv;
}

void* ScreenShot2(void* src, int srcPitch, int width, int height, int* bitmapSize, int png)
{
    if (png) {
        return ScreenShotPng(src, srcPitch, width, height, bitmapSize);
    }
    else {
        return ScreenShotBmp(src, srcPitch, width, height, bitmapSize);
    }
}

void ScreenShot3(Properties* properties, void* src, int srcPitch, int width, int height, int png)
{
    int bitmapSize;
    FILE* file;

    void* bitmap = ScreenShot2(src, srcPitch, width, height, &bitmapSize, png);
    if (bitmap == NULL || bitmapSize <= 0) {
        return;
    }
    file = fopen(generateSaveFilename(properties, baseDir, basePrefix, (png ? ".png" : ".bmp"), 4), "wb");
    if (file != NULL) {
    	fwrite(bitmap, 1, bitmapSize, file);
        fclose(file);
    }
    free(bitmap);
}

void screenshotSetDirectory(char* directory, char* prefix) 
{
    strcpy(baseDir, directory);
    strcpy(basePrefix, prefix);
}


HBITMAP BitmapFromData(void* bmp) {
    char*       bitmap = (char*)bmp;
    BMPHeader*  hdr    = (BMPHeader*)(bitmap + 2);
    char*       data   = (char*)(hdr + 1);
    BITMAPINFO* bmi;
    static HDC  hdc=NULL;
    HBITMAP     hbm;

    if (bitmap[0] != 'B' || bitmap[1] != 'M') {
        return INVALID_HANDLE_VALUE;
    }

    bmi = (BITMAPINFO*)malloc(sizeof(BITMAPINFOHEADER));
    bmi->bmiHeader.biSize         = sizeof(BITMAPINFOHEADER);
    bmi->bmiHeader.biWidth        = hdr->Width;
    bmi->bmiHeader.biHeight       = hdr->Height;
    bmi->bmiHeader.biPlanes       = hdr->Planes;
    bmi->bmiHeader.biBitCount     = hdr->BitCount;
    bmi->bmiHeader.biCompression  = BI_RGB; 
    bmi->bmiHeader.biSizeImage    = hdr->SizeImage;
    bmi->bmiHeader.biClrImportant = 0; 

    if (hdc) { ReleaseDC(NULL,hdc); hdc=NULL; }
    hdc = GetDC(NULL);
    hbm = CreateCompatibleBitmap(hdc, hdr->Width, hdr->Height);

    SetDIBits(hdc, hbm, 0, hdr->Height, data, bmi, DIB_RGB_COLORS);

    return hbm;
}
