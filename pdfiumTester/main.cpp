// main.cpp
#include <chrono> // std::chrono
#include <iostream> // std::cout
#include <string> // std::string, std::wstring
#include <ctype.h> // tolower 
#include "cmdline.h" // cmdline::parser
#include "PDFiumConverter.h"
#include "pdf_utils.h"

int main(int argc, char* argv[])
{
    // 로케일 설정
    setlocale(LC_ALL, "");

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
   
    {
        // type 문자열 소문자로 변경
        std::transform(type.begin(), type.end(), type.begin(), ::tolower);

        // PDF 파일이 존재하는지 체크
        if (!pathFileExists(source.c_str())) {
            std::cerr << "source file is not valid path";
            return 0;
        }

        // 결과 폴더가 존재하는지 체크
        if (!pathIsDirectory(result.c_str())) {
            std::cerr << "result directory is not exist";
            return 0;
        }
        result = pathAddSeparator(result);
    }

    const std::wstring samplePath = _A2U(source);
    const std::wstring resultDir = _A2U(result);

    {
        bool result = PDF::Converter::PDFium::Init();
        if (!result) {
            std::cerr << "PDFium Init() Failed()";
            return 0;
        }

        // PDF -> PNG, PDF -> TXT 변환
        {
            PDF::Converter::PDFium pdfConverter;

            std::cout << "[Begin] : PDFium pdf to " << type << std::endl;
            std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
            {
                if (type == "png") {
                    result = pdfConverter.ToImage(samplePath.c_str(), resultDir.c_str());
                    if (!result) {
                        std::cout << "PDFium ToImage() Failed()" << std::endl;
                    }
                } else if (type == "txt") {
                    result = pdfConverter.ToText(samplePath.c_str(), resultDir.c_str());
                    if (!result) {
                        std::cerr << "PDFium ToText() Failed()" << std::endl;
                    }
                }
            }
            std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
            std::cout << "    Time difference = " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << "[µs]" << std::endl;
            std::cout << "    tTime difference = " << std::chrono::duration_cast<std::chrono::nanoseconds> (end - begin).count() << "[ns]" << std::endl;
            std::cout << "    Time difference (sec) = " << (std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count()) / 1000000.0 << std::endl;
            std::cout << "[End] : PDFium pdf to " << type << std::endl;
        }

        PDF::Converter::PDFium::Fini();
    }
    
	return 0;
}