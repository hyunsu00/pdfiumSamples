// fpdf_converter.h
#pragma once

#ifdef _WIN32
#include "write_gdiplus.inl"
namespace fpdf { namespace converter {
    using gdiplus::WritePng;
}}
#else
#include "write_libpng.inl"
namespace fpdf { namespace converter {
    using libpng::WritePng;
}}
#endif
