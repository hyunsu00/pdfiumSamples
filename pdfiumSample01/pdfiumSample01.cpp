// pdfiumSample01.cpp : 애플리케이션의 진입점을 정의합니다.
//
#include <fpdfview.h>

using namespace std;

int main()
{
	FPDF_LIBRARY_CONFIG config;
	config.version = 2;
	config.m_pUserFontPaths = NULL;
	config.m_pIsolate = NULL;
	config.m_v8EmbedderSlot = 0;

	FPDF_InitLibraryWithConfig(&config);

	FPDF_DestroyLibrary();

	return 0;
}
