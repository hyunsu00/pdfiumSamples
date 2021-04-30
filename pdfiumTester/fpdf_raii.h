// fpdf_raii.h
#pragma once
#include <memory> // std::unique_ptr
#include <type_traits> // std::remove_pointer
#include <fpdfview.h> // FPDF_CloseDocument, FPDF_ClosePage, FPDFText_ClosePage, FPDFBitmap_Destroy
#include <fpdf_formfill.h> // FPDFDOC_ExitFormFillEnvironment
#include <fpdf_text.h> // FPDFText_ClosePage

namespace {

	struct FPDFDocumentDeleter 
	{
  		inline void operator()(FPDF_DOCUMENT doc) 
		{ 
			FPDF_CloseDocument(doc); 
		}
	}; // struct FPDFDocumentDeleter 
	using AutoFPDFDocumentPtr = std::unique_ptr<std::remove_pointer<FPDF_DOCUMENT>::type, FPDFDocumentDeleter>;

	struct FPDFFormHandleDeleter 
	{
  		inline void operator()(FPDF_FORMHANDLE form) 
		{
    		FPDFDOC_ExitFormFillEnvironment(form);
  		}
	}; // struct FPDFFormHandleDeleter
	using AutoFPDFFormHandlePtr = std::unique_ptr<std::remove_pointer<FPDF_FORMHANDLE>::type, FPDFFormHandleDeleter>;

	struct FPDFPageDeleter 
	{
  		inline void operator()(FPDF_PAGE page) 
		{ 
			FPDF_ClosePage(page); 
		}
	}; // struct FPDFPageDeleter 
	using AutoFPDFPagePtr = std::unique_ptr<std::remove_pointer<FPDF_PAGE>::type, FPDFPageDeleter>;

	struct FPDFTextPageDeleter 
	{
  		inline void operator()(FPDF_TEXTPAGE text) 
		{ 
			FPDFText_ClosePage(text); 
		}
	}; // struct FPDFTextPageDeleter 
	using AutoFPDFTextPagePtr = std::unique_ptr<std::remove_pointer<FPDF_TEXTPAGE>::type, FPDFTextPageDeleter>;

	struct FPDFBitmapDeleter
	{
		inline void operator()(FPDF_BITMAP bitmap)
		{
			FPDFBitmap_Destroy(bitmap);
		}
	}; // struct FPDFBitmapDeleter
	using AutoFPDFBitmapPtr = std::unique_ptr<std::remove_pointer<FPDF_BITMAP>::type, FPDFBitmapDeleter>;
}
