// fpdf_raii.h
#pragma once
#include <memory> // std::unique_ptr
#include <type_traits> // std::remove_pointer
#include <fpdfview.h> // FPDFBitmap_Destroy

namespace {

	struct FreeDeleter
	{
		inline void operator()(void* ptr) const
		{
			free(ptr);
		}
	}; // struct FreeDeleter 
	using AutoMemoryPtr = std::unique_ptr<char, FreeDeleter>;

	struct FPDFBitmapDeleter
	{
		inline void operator()(FPDF_BITMAP bitmap)
		{
			FPDFBitmap_Destroy(bitmap);
		}
	};
	using AutoFPDFBitmapPtr = std::unique_ptr<std::remove_pointer<FPDF_BITMAP>::type, FPDFBitmapDeleter>;
}