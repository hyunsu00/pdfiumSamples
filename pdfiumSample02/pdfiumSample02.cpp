#include <fpdfview.h> // FPDF_InitLibraryWithConfig, FPDF_DestroyLibrary, FPDF_LoadDocument, FPDF_CloseDocument, FPDF_GetPageCount, FPDF_LoadPage, FPDF_ClosePage
#include <fpdf_text.h> // FPDFText_LoadPage, FPDFText_CountChars, FPDFText_GetText, FPDFText_ClosePage
#include <vector> // std::vector
#include <string> // std::string

#ifdef _WIN32
    #if (_HAS_CXX17)
        #include <filesystem> // std::filesystem::path
    #endif
#else
#include <string.h> // strdup
#include <libgen.h> // dirname
#include <experimental/filesystem>
namespace std {
    // using namespace std::experimental;
    namespace filesystem = std::experimental::filesystem;
}
#define _HAS_CXX17 1
#endif

int main(int argc, char* argv[])
{
    std::string exeDir;

#if (_HAS_CXX17)
    namespace fs = std::filesystem;
    fs::path exePath(argv[0]);
    exeDir = exePath.remove_filename().string();
#else
    #ifdef _WIN32
        char drive[_MAX_DRIVE] = { 0, }; // 드라이브 명
        char dir[_MAX_DIR] = { 0, }; // 디렉토리 경로
        _splitpath_s(argv[0], drive, _MAX_DRIVE, dir, _MAX_DIR, nullptr, 0, nullptr, 0);
        std::string exeDir = std::string(drive) + dir;
    #else
        char* exePath = strdup(argv[0]);
        std::string exeDir = dirname(exePath);
        free(exePath);
        exeDir += "/";
    #endif
#endif

    std::string samplePath = exeDir + "samples/sample01.pdf";
    std::string resultPath = exeDir + "result/sample01.txt";

	FPDF_LIBRARY_CONFIG config;

	config.version = 3;
	config.m_pUserFontPaths = nullptr;
	config.m_pIsolate = nullptr;
	config.m_v8EmbedderSlot = 0;
	config.m_pPlatform = nullptr;
	::FPDF_InitLibraryWithConfig(&config);
    {
        FPDF_DOCUMENT document = ::FPDF_LoadDocument(samplePath.c_str(), nullptr);
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
    }
    ::FPDF_DestroyLibrary();

	return 0;
}