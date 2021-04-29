// PDFicumConverter.cpp
#include "PDFiumConverter.h"
#include <fpdfview.h> // FPDF_InitLibraryWithConfig, FPDF_DestroyLibrary, FPDF_LoadDocument, FPDF_CloseDocument, FPDF_GetPageCount, FPDF_LoadPage, FPDF_ClosePage
#include <fpdfview.h> // FPDF_InitLibraryWithConfig, FPDF_DestroyLibrary, FPDF_LoadDocument, FPDF_CloseDocument, FPDF_GetPageCount, FPDF_LoadPage, FPDF_ClosePage
#include <fpdf_text.h> // FPDFText_LoadPage, FPDFText_CountChars, FPDFText_GetText, FPDFText_ClosePage
#include <fpdf_formfill.h> // FPDFDOC_InitFormFillEnvironment, FPDFDOC_ExitFormFillEnvironment
#include <vector> // std::vector
#include <string> // std::string
#include "fpdf_utils.h"
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
	: m_bMemory(false)
	, m_bPPL(false)
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

		FPDF_DOCUMENT document = nullptr;
		AutoMemoryPtr memoryFile;

		if (m_bMemory) {
			size_t buffer_len = 0;
			memoryFile = getFileContents(_U2A(sourceFile).c_str(), &buffer_len);
			document = ::FPDF_LoadMemDocument(memoryFile.get(), buffer_len, nullptr);
		} else {
			document = ::FPDF_LoadDocument(_U2A(sourceFile).c_str(), nullptr);
		}

		FPDF_FORMHANDLE form = FPDFDOC_InitFormFillEnvironment(document, nullptr);
		{
			if (m_bPPL) {
				_ASSERTE(!"PPL은 현재 디거깅이 필요함 - 실패를 반환한다.");

				// !!!!! Pdfium은 쓰레드 쎄이프 하지 않다. --> 실망
            	// https://groups.google.com/g/pdfium/c/HeZSsM_KEUk
				std::vector<int> vIndex(FPDF_GetPageCount(document));
				for (size_t i = 0; i < vIndex.size(); i++) vIndex[i] = i;

				concurrency::parallel_for_each(
					vIndex.begin(),
					vIndex.end(),
					[&](int pageIndex) {
						FPDF_PAGE page = ::FPDF_LoadPage(document, pageIndex);
						{
							FPDF_TEXTPAGE textPage = ::FPDFText_LoadPage(page);
							{
								std::string resultPath = _U2A(targetDir) + std::to_string(pageIndex) + ".png";
								// PNG파일 추출
								fpdf::converter::WritePng(resultPath.c_str(), page, form, dpi);
							}
							::FPDFText_ClosePage(textPage);
						}
						::FPDF_ClosePage(page);
					}
				);
			} else {
				for (int pageIndex = 0; pageIndex < FPDF_GetPageCount(document); pageIndex++) {
					FPDF_PAGE page = ::FPDF_LoadPage(document, pageIndex);
					{
						FPDF_TEXTPAGE textPage = ::FPDFText_LoadPage(page);
						{
							std::string resultPath = _U2A(targetDir) + std::to_string(pageIndex) + ".png";
							// PNG파일 추출
							fpdf::converter::WritePng(resultPath.c_str(), page, form, dpi);
						}
						::FPDFText_ClosePage(textPage);
					}
					::FPDF_ClosePage(page);
				}
			}
		}
		FPDFDOC_ExitFormFillEnvironment(form);
		::FPDF_CloseDocument(document);

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

		FPDF_DOCUMENT document = nullptr;
		AutoMemoryPtr memoryFile;

		if (m_bMemory) {
			size_t buffer_len = 0;
			memoryFile = getFileContents(samplePath.c_str(), &buffer_len);
			document = ::FPDF_LoadMemDocument(memoryFile.get(), buffer_len, nullptr);
		} else {
			document = ::FPDF_LoadDocument(samplePath.c_str(), nullptr);
		}

		{
			// 텍스트파일 추출
			FILE* fp = fopen(resultPath.c_str(), "wb");
			unsigned short bom = 0xFEFF;
			fwrite(&bom, sizeof(bom), 1, fp);
			for (int pageIndex = 0; pageIndex < FPDF_GetPageCount(document); pageIndex++) {
				FPDF_PAGE page = ::FPDF_LoadPage(document, pageIndex);
				{
					FPDF_TEXTPAGE textPage = ::FPDFText_LoadPage(page);
					{
						int countChars = ::FPDFText_CountChars(textPage);
						std::vector<unsigned short> result(countChars + 1, 0);
						::FPDFText_GetText(textPage, 0, static_cast<int>(result.size()), &result[0]);
						fwrite(&result[0], countChars * sizeof(unsigned short), 1, fp);
						unsigned short CRLF[2] = { '\r', '\n' };
						fwrite(CRLF, sizeof(CRLF), 1, fp);
					}
					::FPDFText_ClosePage(textPage);
				}
				::FPDF_ClosePage(page);
			}
			fclose(fp);
		}

		::FPDF_CloseDocument(document);

		return true;
	}

}} // PDF::Converter
