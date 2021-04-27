// write_libpng.inl
#pragma once

#include <fpdfview.h> // FPDF_GetPageWidthF, FPDF_GetPageHeightF, FPDFBitmap_Create, 
                      // FPDFBitmap_Create, FPDFBitmap_FillRect, FPDF_RenderPageBitmap, 
                      // FPDFBitmap_GetStride, FPDFBitmap_GetBuffer
#include <fpdf_edit.h> // FPDFPage_HasTransparency
#include <fpdf_formfill.h> // FPDF_FFLDraw
#include "image_png.h"
#define _ASSERTE

namespace libpng {

/*
   	// defined fpdfview.h
    // More DIB formats
    // Unknown or unsupported format.
    #define FPDFBitmap_Unknown 0
    // Gray scale bitmap, one byte per pixel.
    #define FPDFBitmap_Gray 1
    // 3 bytes per pixel, byte order: blue, green, red.
    #define FPDFBitmap_BGR 2
    // 4 bytes per pixel, byte order: blue, green, red, unused.
    #define FPDFBitmap_BGRx 3
    // 4 bytes per pixel, byte order: blue, green, red, alpha.
    #define FPDFBitmap_BGRA 4
*/   
    std::vector<uint8_t> EncodePng(
        std::span<const uint8_t> input,
        int width,
        int height,
        int stride,
        int format
    ) {
        std::vector<uint8_t> png;
        switch (format) {
        case FPDFBitmap_Unknown:
            break;
        case FPDFBitmap_Gray:
            png = image::png::EncodeGrayPNG(input, width, height, stride);
            break;
        case FPDFBitmap_BGR:
            png = image::png::EncodeBGRPNG(input, width, height, stride);
            break;
        case FPDFBitmap_BGRx:
            png = image::png::EncodeBGRAPNG(input, width, height, stride,
                /*discard_transparency=*/true);
            break;
        case FPDFBitmap_BGRA:
            png = image::png::EncodeBGRAPNG(input, width, height, stride,
                /*discard_transparency=*/false);
            break;
        default:
            _ASSERTE(!"잘못된 파일 포맷");
        }
        return png;
    }

    bool WritePng(const char* pathName, FPDF_PAGE page, FPDF_FORMHANDLE form = nullptr, float dpi = 96.F)
    {
        float pageWidth = FPDF_GetPageWidthF(page);
        float pageHeight = FPDF_GetPageHeightF(page);
        int width = static_cast<int>(pageWidth / 72.F * dpi);
        int height = static_cast<int>(pageHeight / 72.F * dpi);

        int alpha = FPDFPage_HasTransparency(page) ? 1 : 0;
        FPDF_BITMAP bitmap = FPDFBitmap_Create(width, height, alpha);
        if (!bitmap) {
            return false;
        }

        FPDF_DWORD fill_color = alpha ? 0x00000000 : 0xFFFFFFFF;
        FPDFBitmap_FillRect(bitmap, 0, 0, width, height, fill_color);
        int flags = 0;
        FPDF_RenderPageBitmap(bitmap, page, 0, 0, width, height, 0, flags);
        FPDF_FFLDraw(form, bitmap, page, 0, 0, width, height, 0, flags);

        int stride = FPDFBitmap_GetStride(bitmap);
        void* buffer = FPDFBitmap_GetBuffer(bitmap);

        std::vector<uint8_t> png_encoding = EncodePng(
            std::span<const uint8_t>(static_cast<uint8_t*>(buffer), stride * height),
            width,
            height,
            stride,
            FPDFBitmap_BGRA
        );

        FILE* fp = fopen(pathName, "wb");
        size_t bytes_written = fwrite(&png_encoding.front(), 1, png_encoding.size(), fp);
        (void)fclose(fp);

        return true;
    }
	
} // libpng