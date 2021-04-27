#include <fpdfview.h> // FPDF_InitLibraryWithConfig, FPDF_DestroyLibrary, FPDF_LoadDocument, FPDF_CloseDocument, FPDF_GetPageCount, FPDF_LoadPage, FPDF_ClosePage
#include <fpdf_text.h> // FPDFText_LoadPage, FPDFText_CountChars, FPDFText_GetText, FPDFText_ClosePage
#include <fpdf_formfill.h> // FPDFDOC_InitFormFillEnvironment, FPDFDOC_ExitFormFillEnvironment
#include <vector> // std::vector
#include <string> // std::string
#include "fpdf_converter.h"

#ifdef _WIN32
#else
#   include <string.h> // strdup
#   include <libgen.h> // dirname
#endif

int main(int argc, char* argv[])
{
#ifdef _WIN32
    ULONG_PTR gdiplusToken;
    Gdiplus::GdiplusStartupInput gdiplusStartupInput = { 0, };
    Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
#endif

    std::string exeDir;
#ifdef _WIN32
    char drive[_MAX_DRIVE] = { 0, }; // 드라이브 명
    char dir[_MAX_DIR] = { 0, }; // 디렉토리 경로
    _splitpath_s(argv[0], drive, _MAX_DRIVE, dir, _MAX_DIR, nullptr, 0, nullptr, 0);
    exeDir = std::string(drive) + dir;
#else
    char* exePath = strdup(argv[0]);
    exeDir = dirname(exePath);
    free(exePath);
    exeDir += "/";
#endif

    std::string pdfName = "sample01";
    std::string samplePath = exeDir + "samples/" + pdfName + ".pdf";
    std::string resultDir = exeDir + "result/";
    std::string resultPath;

	FPDF_LIBRARY_CONFIG config;

	config.version = 3;
	config.m_pUserFontPaths = nullptr;
	config.m_pIsolate = nullptr;
	config.m_v8EmbedderSlot = 0;
	config.m_pPlatform = nullptr;
	::FPDF_InitLibraryWithConfig(&config);
    {
        FPDF_DOCUMENT document = ::FPDF_LoadDocument(samplePath.c_str(), nullptr);
        FPDF_FORMHANDLE form = FPDFDOC_InitFormFillEnvironment(document, nullptr);
        {
            for (int pageIndex = 0; pageIndex < FPDF_GetPageCount(document); pageIndex++) {
                FPDF_PAGE page = ::FPDF_LoadPage(document, pageIndex);
                {
                    FPDF_TEXTPAGE textPage = ::FPDFText_LoadPage(page);
                    {
                        resultPath = resultDir + pdfName + "." + std::to_string(pageIndex) + ".png";
                        // PNG파일 추출
                        fpdf::converter::WritePng(resultPath.c_str(), page, form, 96.F);
                    }
                    ::FPDFText_ClosePage(textPage);
                }
                ::FPDF_ClosePage(page);
            }
        }
        FPDFDOC_ExitFormFillEnvironment(form);
        ::FPDF_CloseDocument(document);
    }
    ::FPDF_DestroyLibrary();

#ifdef _WIN32
    Gdiplus::GdiplusShutdown(gdiplusToken);
#endif

	return 0;
}