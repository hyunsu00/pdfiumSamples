#include <fpdfview.h> // FPDF_InitLibraryWithConfig, FPDF_DestroyLibrary, FPDF_LoadDocument, FPDF_CloseDocument, FPDF_GetPageCount, FPDF_LoadPage, FPDF_ClosePage
#include <fpdf_text.h> // FPDFText_LoadPage, FPDFText_CountChars, FPDFText_GetText, FPDFText_ClosePage
#include <fpdf_formfill.h> // FPDFDOC_InitFormFillEnvironment, FPDFDOC_ExitFormFillEnvironment
#include <vector> // std::vector
#include <string> // std::string
#include <memory> // std::unique_ptr
#include "fpdf_converter.h"
#include <chrono>
#include <iostream>

#ifdef _WIN32
#else
#   include <string.h> // strdup
#   include <libgen.h> // dirname
#   include <tbb/parallel_for_each.h>
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

    struct FreeDeleter 
    {
        inline void operator()(void* ptr) const 
        { 
            free(ptr); 
        }
    }; // struct FreeDeleter 
    using AutoMemoryPtr = std::unique_ptr<char, FreeDeleter>;

    auto getFileContents = [](const char* filename, size_t* retlen) -> AutoMemoryPtr {
        FILE* file = fopen(filename, "rb");
        if (!file) {
            fprintf(stderr, "Failed to open: %s\n", filename);
            return nullptr;
        }
        (void)fseek(file, 0, SEEK_END);
        size_t file_length = ftell(file);
        if (!file_length) {
            return nullptr;
        }
        (void)fseek(file, 0, SEEK_SET);
        AutoMemoryPtr buffer(static_cast<char*>(malloc(file_length)));
        if (!buffer) {
            return nullptr;
        }
        size_t bytes_read = fread(buffer.get(), 1, file_length, file);
        (void)fclose(file);
        if (bytes_read != file_length) {
            fprintf(stderr, "Failed to read: %s\n", filename);
            return nullptr;
        }
        *retlen = bytes_read;
        return buffer;
    };

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
        std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
#if 1       
        FPDF_DOCUMENT document = ::FPDF_LoadDocument(samplePath.c_str(), nullptr);
#else
        size_t buffer_len = 0;
        AutoMemoryPtr memoryFile = getFileContents(samplePath.c_str(), &buffer_len);
        FPDF_DOCUMENT document = ::FPDF_LoadMemDocument(memoryFile.get(), buffer_len, nullptr);
#endif
        FPDF_FORMHANDLE form = FPDFDOC_InitFormFillEnvironment(document, nullptr);
        {
#if 1            
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
#else
            // Pdfium은 쓰레드 쎄이프 하지 않다. --> 실망
            std::vector<int> vIndex(FPDF_GetPageCount(document));
            for (size_t i = 0; i < vIndex.size(); i++) vIndex[i] = i;

            tbb::parallel_for_each(
                vIndex.begin(),
                vIndex.end(),
                [&](int pageIndex) {
                    std::cout << "begin pageIndex = " << pageIndex << std::endl;
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
                    std::cout << "end pageIndex = " << pageIndex << std::endl;
                }
            );
#endif
        }
        FPDFDOC_ExitFormFillEnvironment(form);
        ::FPDF_CloseDocument(document);

        std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
        std::cout << "Time difference = " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << "[µs]" << std::endl;
        std::cout << "Time difference = " << std::chrono::duration_cast<std::chrono::nanoseconds> (end - begin).count() << "[ns]" << std::endl;
        std::cout << "Time difference (sec) = " <<  (std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count()) /1000000.0  << std::endl;
    }
    ::FPDF_DestroyLibrary();

#ifdef _WIN32
    Gdiplus::GdiplusShutdown(gdiplusToken);
#endif

	return 0;
}