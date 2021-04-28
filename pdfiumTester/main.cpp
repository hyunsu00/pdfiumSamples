#include <fpdfview.h> // FPDF_InitLibraryWithConfig, FPDF_DestroyLibrary, FPDF_LoadDocument, FPDF_CloseDocument, FPDF_GetPageCount, FPDF_LoadPage, FPDF_ClosePage
#include <fpdf_text.h> // FPDFText_LoadPage, FPDFText_CountChars, FPDFText_GetText, FPDFText_ClosePage
#include <fpdf_formfill.h> // FPDFDOC_InitFormFillEnvironment, FPDFDOC_ExitFormFillEnvironment
#include <vector> // std::vector
#include <string> // std::string
#include <memory> // std::unique_ptr
#include <chrono> // std::chrono
#include <iostream> // std::cout
#include "fpdf_converter.h"
#include "cmdline.h"

#ifdef _WIN32
#   include <ppl.h>
#else
#   include <string.h> // strdup
#   include <sys/stat.h> // stat
#   include <libgen.h> // dirname, basename
#   include <tbb/parallel_for_each.h> // parallel_for_each
namespace concurrency {
    using tbb::parallel_for_each;
}
#endif

int main(int argc, char* argv[])
{
    cmdline::parser parser;
    parser.add<std::string>("source", 's', "PDF absolute file path", true, "");
    parser.add<std::string>("result", 'r', "result absolute dir", true, "");
    parser.add<std::string>("type", 't', "convert type", false, "png", cmdline::oneof<std::string>("png", "txt"));
    parser.add("memory", '\0', "load memory");
    parser.add("ppl", '\0', "use ppl library");
    parser.add("help", 0, "print this message");
    parser.set_program_name("pdfiumTester");

    bool ok = parser.parse(argc, argv);

    if (argc == 1 || parser.exist("help") || ok == false){
        std::cerr << parser.usage();
        return 0;
    }

    std::string source = parser.get<std::string>("source");
    std::string result = parser.get<std::string>("result");
    std::string type = parser.get<std::string>("type");

    struct FreeDeleter 
    {
        inline void operator()(void* ptr) const 
        { 
            free(ptr); 
        }
    }; // struct FreeDeleter 
    using AutoMemoryPtr = std::unique_ptr<char, FreeDeleter>;

    std::string rawFileName;
#ifdef _WIN32

#else
    {
        auto _dirExists = [](const char* const dirPath) -> bool {
            struct stat info;
            if (stat(dirPath, &info) == 0 && S_ISDIR(info.st_mode)) {
                return true;
            }

            return false;
        };
        auto _addDirSeparator = [](const std::string& dirPath) -> std::string {
            std::string addDirPath = dirPath;
            if (dirPath.back() != '/') {
                addDirPath += "/";
            }
            return addDirPath;
        };
        auto _removeExt = [](const std::string& fileName) -> std::string {
            size_t lastIndex = fileName.find_last_of("."); 
            std::string rawName = fileName.substr(0, lastIndex);
            return rawName;
        };

        // PDF 파일이 존재하는지 체크
        if (access(source.c_str(), F_OK)) {
            std::cout << "source file is not valid path" << std::endl;
            return 0;
        }

        // 결과 폴더가 존재하는지 체크
        if (!_dirExists(result.c_str())) {
            std::cout << "result directory is not exist" << std::endl;
            return 0;
        }
        result = _addDirSeparator(result);

        rawFileName = _removeExt(basename(AutoMemoryPtr(strdup(source.c_str())).get()));
    }
#endif

    // 
    const std::string samplePath = source;
    const std::string resultDir= result;
    const std::string pdfName = rawFileName;

#ifdef _WIN32
    ULONG_PTR gdiplusToken;
    Gdiplus::GdiplusStartupInput gdiplusStartupInput = { 0, };
    Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
#endif

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

	FPDF_LIBRARY_CONFIG config;

	config.version = 3;
	config.m_pUserFontPaths = nullptr;
	config.m_pIsolate = nullptr;
	config.m_v8EmbedderSlot = 0;
	config.m_pPlatform = nullptr;
	::FPDF_InitLibraryWithConfig(&config);
    {
        std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
#if 0       
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
                        std::string resultPath = resultDir + pdfName + "." + std::to_string(pageIndex) + ".png";
                        // PNG파일 추출
                        fpdf::converter::WritePng(resultPath.c_str(), page, form, 96.F);
                    }
                    ::FPDFText_ClosePage(textPage);
                }
                ::FPDF_ClosePage(page);
            }
#else
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
                            std::string resultPath = resultDir + pdfName + "." + std::to_string(pageIndex) + ".png";
                            // PNG파일 추출
                            fpdf::converter::WritePng(resultPath.c_str(), page, form, 96.F);
                        }
                        ::FPDFText_ClosePage(textPage);
                    }
                    ::FPDF_ClosePage(page);
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