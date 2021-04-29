// PDFiumConverter.h
#pragma once

namespace PDF { namespace Converter {

	class PDFium
	{
	public:
		PDFium();
		~PDFium();

	public:
		static bool Init();
		static void Fini();

	public:
		bool ToImage(const wchar_t* sourceFile, const wchar_t* targetDir, int dpi = 96);
		bool ToText(const wchar_t* sourceFile, const wchar_t* targetDir);

	private:
		bool m_bMemory;
		bool m_bPPL;
	}; // class PDFBox

}} // PDF::Converter
