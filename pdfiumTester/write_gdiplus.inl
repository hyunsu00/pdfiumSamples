﻿// write_gdiplus.inl
#pragma once

#include <fpdfview.h> // FPDF_GetPageWidthF, FPDF_GetPageHeightF, FPDFBitmap_Create, 
                      // FPDFBitmap_Create, FPDFBitmap_FillRect, FPDF_RenderPageBitmap, 
                      // FPDFBitmap_GetStride, FPDFBitmap_GetBuffer
#include <fpdf_edit.h> // FPDFPage_HasTransparency
#include <fpdf_formfill.h> // FPDF_FFLDraw
#include <gdiplus.h>
#include <atlbase.h>
#include <atlconv.h>

namespace gdiplus {

/*
    std::vector<uint8_t> EncodePng(
        const std::span<const uint8_t>& input,
        int width,
        int height,
        int stride,
        int format
    ) {
        std::vector<uint8_t> output;

        CLSID pngClsid = { 0, };
        int result = _getEncoderClsid(L"image/png", &pngClsid);
        _ASSERTE(result != -1 && "_getEncoderClsid() Failed");
        if (result == -1) {
            return output;
        }

        Gdiplus::Bitmap bitmap(width, height, stride, format, (BYTE*)input.data());

        //write to IStream
        IStream* istream = nullptr;
        CreateStreamOnHGlobal(NULL, TRUE, &istream);

        Gdiplus::Status status = bitmap.Save(istream, &pngClsid);
        if (status != Gdiplus::Status::Ok) {
            return output;
        }

        //get memory handle associated with istream
        HGLOBAL hGlobal = NULL;
        ::GetHGlobalFromStream(istream, &hGlobal);

        //copy IStream to buffer
        int bufsize = ::GlobalSize(hGlobal);
        output.resize(bufsize);

        //lock & unlock memory
        LPVOID pImage = ::GlobalLock(hGlobal);
        ::memcpy(&output[0], pImage, bufsize);
        ::GlobalUnlock(hGlobal);

        istream->Release();

        return output;
    }
*/
    inline bool WritePng(const char* pathName, FPDF_PAGE page, FPDF_FORMHANDLE form = nullptr, float dpi = 96.F)
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

        {
            auto _getEncoderClsid = [](const wchar_t* format, CLSID* pClsid) -> int {
                using namespace ::Gdiplus;

                UINT  num = 0;          // number of image encoders
                UINT  size = 0;         // size of the image encoder array in bytes

                ImageCodecInfo* pImageCodecInfo = NULL;

                GetImageEncodersSize(&num, &size);
                if (size == 0)
                    return -1;  // Failure

                pImageCodecInfo = (ImageCodecInfo*)(malloc(size));
                if (pImageCodecInfo == NULL)
                    return -1;  // Failure

                GetImageEncoders(num, size, pImageCodecInfo);

                for (UINT j = 0; j < num; ++j) {
                    if (wcscmp(pImageCodecInfo[j].MimeType, format) == 0) {
                        *pClsid = pImageCodecInfo[j].Clsid;
                        free(pImageCodecInfo);
                        return j;  // Success
                    }
                }

                free(pImageCodecInfo);
                return -1;  // Failure
            };

            Gdiplus::Bitmap oBitmap(width, height, stride, PixelFormat32bppARGB, (BYTE*)buffer);
            CLSID pngClsid = { 0, };
            int result = _getEncoderClsid(L"image/png", &pngClsid);
            if (result == -1) {
                return false;
            }
            Gdiplus::Status status = oBitmap.SetResolution(dpi, dpi);
            if (status != Gdiplus::Status::Ok) {
                return false;
            }

            oBitmap.Save(ATL::CA2W(pathName), &pngClsid, NULL);
        }

        return true;
    }

} // gdiplus
