// PDFicumConverter.cpp
#include "PDFiumConverter.h"
#include <fpdfview.h> // FPDF_InitLibraryWithConfig, FPDF_DestroyLibrary, FPDF_InitLibraryWithConfig, FPDF_LoadDocument, FPDF_GetPageCount, FPDF_LoadPage
#include <fpdf_text.h> // FPDFText_LoadPage, FPDFText_CountChars, FPDFText_GetText
#include <fpdf_formfill.h> // FPDFDOC_InitFormFillEnvironment
#include <vector> // std::vector
#include <string> // std::string
#include "pdf_utils.h" // AutoMemoryPtr
#include "fpdf_raii.h" // AutoFPDFDocumentPtr, AutoFPDFFormHandlePtr, AutoFPDFPagePtr, AutoFPDFTextPagePtr
#include "fpdf_converter.h"

#ifdef _WIN32
#	include <gdiplus.h>
#   include <ppl.h>
#else
#   include <string.h> // strdup
#   include <libgen.h> // dirname, basename
#   include <tbb/parallel_for_each.h> // parallel_for_each
namespace concurrency {
	using tbb::parallel_for_each;
}
#endif

namespace PDF { namespace Converter {

#ifdef _WIN32
	static ULONG_PTR s_GdiplusToken = 0;
#endif

	PDFium::PDFium()
	: m_Flag()
	{
	}

	PDFium::~PDFium()
	{
	}

	bool PDFium::Init()
	{
#ifdef _WIN32
		Gdiplus::GdiplusStartupInput gdiplusStartupInput = { 0, };
		Gdiplus::GdiplusStartup(&s_GdiplusToken, &gdiplusStartupInput, NULL);
#endif

		FPDF_LIBRARY_CONFIG config;

		config.version = 3;
		config.m_pUserFontPaths = nullptr;
		config.m_pIsolate = nullptr;
		config.m_v8EmbedderSlot = 0;
		config.m_pPlatform = nullptr;
		::FPDF_InitLibraryWithConfig(&config);

		return true;
	}

	void PDFium::Fini()
	{
#ifdef _WIN32
		Gdiplus::GdiplusShutdown(s_GdiplusToken);
#endif

		::FPDF_DestroyLibrary();
	}

	bool PDFium::ToImage(const wchar_t* sourceFile, const wchar_t* targetDir, int dpi /*= 96*/)
	{
		_ASSERTE(sourceFile && "sourceFile is not Null");
		_ASSERTE(targetDir && "sourceFile is not Null");
		if (!sourceFile || !targetDir) {
			return false;
		}

		FPDF_DOCUMENT fpdf_document = nullptr;
		AutoMemoryPtr memoryFile;

		if (m_Flag.test(FlagMemory)) {
			size_t buffer_len = 0;
			memoryFile = getFileContents(_U2A(sourceFile).c_str(), &buffer_len);
			fpdf_document = ::FPDF_LoadMemDocument(memoryFile.get(), static_cast<int>(buffer_len), nullptr);
		} else {
			fpdf_document = ::FPDF_LoadDocument(_U2A(sourceFile).c_str(), nullptr);
		}
		_ASSERTE(fpdf_document && "fpdf_document is not Null");
        if (!fpdf_document) {
            return false;
        }
		AutoFPDFDocumentPtr document(fpdf_document);

		FPDF_FORMHANDLE fpdf_form = FPDFDOC_InitFormFillEnvironment(document.get(), nullptr);
		AutoFPDFFormHandlePtr form(fpdf_form);

		{
			if (m_Flag.test(FlagPPL)) {
				_ASSERTE(!"PPL은 현재 디거깅이 필요함 - 실패를 반환한다.");

				// !!!!! Pdfium은 쓰레드 쎄이프 하지 않다. --> 실망
            	// https://groups.google.com/g/pdfium/c/HeZSsM_KEUk
				std::vector<size_t> vIndex(FPDF_GetPageCount(document.get()));
				for (size_t i = 0; i < vIndex.size(); i++) vIndex[i] = i;

				concurrency::parallel_for_each(
					vIndex.begin(),
					vIndex.end(),
					[&](size_t pageIndex) {
						FPDF_PAGE fpdf_page = ::FPDF_LoadPage(document.get(), static_cast<int>(pageIndex));
						_ASSERTE(fpdf_page && "fpdf_page is not Null");
        				if (!fpdf_page) {
            				return;
        				}
						AutoFPDFPagePtr page(fpdf_page);

						// PNG파일 추출
						{
							std::string resultPath = _U2A(targetDir) + std::to_string(pageIndex) + ".png";
							fpdf::converter::WritePng(resultPath.c_str(), page.get(), form.get(), static_cast<float>(dpi));
						}
					}
				);
			} else {
				for (int pageIndex = 0; pageIndex < FPDF_GetPageCount(document.get()); pageIndex++) {
					FPDF_PAGE fpdf_page = ::FPDF_LoadPage(document.get(), pageIndex);
					_ASSERTE(fpdf_page && "fpdf_page is not Null");
					if (!fpdf_page) {
						continue;
					}
					AutoFPDFPagePtr page(fpdf_page);

					// PNG파일 추출
					{
						std::string resultPath = _U2A(targetDir) + std::to_string(pageIndex) + ".png";
						fpdf::converter::WritePng(resultPath.c_str(), page.get(), form.get(), static_cast<float>(dpi));
					}
				}
			}
		}

		return true;
	}

	bool PDFium::ToText(const wchar_t* sourceFile, const wchar_t* targetDir)
	{
		_ASSERTE(sourceFile && "sourceFile is not Null");
		_ASSERTE(targetDir && "sourceFile is not Null");
		if (!sourceFile || !targetDir) {
			return false;
		}

		const std::string samplePath = _U2A(sourceFile);
		const std::string rawFileName = removeExt(pathFindFilename(samplePath));
		const std::string resultPath = _U2A(targetDir) + rawFileName + ".txt";

		FPDF_DOCUMENT fpdf_document = nullptr;
		AutoMemoryPtr memoryFile;

		if (m_Flag.test(FlagMemory)) {
			size_t buffer_len = 0;
			memoryFile = getFileContents(samplePath.c_str(), &buffer_len);
			fpdf_document = ::FPDF_LoadMemDocument(memoryFile.get(), static_cast<int>(buffer_len), nullptr);
		} else {
			fpdf_document = ::FPDF_LoadDocument(samplePath.c_str(), nullptr);
		}
		_ASSERTE(fpdf_document && "fpdf_document is not Null");
        if (!fpdf_document) {
            return false;

        }
		AutoFPDFDocumentPtr document(fpdf_document);

		{
			// 텍스트파일 추출
			FILE* fp = fopen(resultPath.c_str(), "wb");
			_ASSERTE(fp && "fopen() Failed");
			if (!fp) {
				return false;        
			}
			unsigned short bom = 0xFEFF;
			fwrite(&bom, sizeof(bom), 1, fp);
			for (int pageIndex = 0; pageIndex < FPDF_GetPageCount(document.get()); pageIndex++) {
				FPDF_PAGE fpdf_page = ::FPDF_LoadPage(document.get(), pageIndex);
				_ASSERTE(fpdf_page && "fpdf_page is not Null");
				if (!fpdf_page) {
					continue;
				}
				AutoFPDFPagePtr page(fpdf_page);

				{
					FPDF_TEXTPAGE fpdf_textPage = ::FPDFText_LoadPage(page.get());
					_ASSERTE(fpdf_textPage && "fpdf_textPage is not Null");
					if (!fpdf_textPage) {
						continue;
					}
					AutoFPDFTextPagePtr textPage(fpdf_textPage);

					{
						int countChars = ::FPDFText_CountChars(textPage.get());
						std::vector<unsigned short> result(countChars + 1, 0);
						::FPDFText_GetText(textPage.get(), 0, static_cast<int>(result.size()), &result[0]);
						fwrite(&result[0], countChars * sizeof(unsigned short), 1, fp);
						unsigned short CRLF[2] = { '\r', '\n' };
						fwrite(CRLF, sizeof(CRLF), 1, fp);
					}
				}
			}
			fclose(fp);
		}

		return true;
	}

}} // PDF::Converter
