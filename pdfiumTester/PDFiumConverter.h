// PDFiumConverter.h
#pragma once
#include <bitset> // std::bitset

namespace PDF { namespace Converter {

	class PDFium
	{
	public:
		enum Flag {
			FlagMemory = 0,
			FlagPPL,
		}; // enum Flag 

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
		std::bitset<2> m_Flag;
	}; // class PDFBox

}} // PDF::Converter
