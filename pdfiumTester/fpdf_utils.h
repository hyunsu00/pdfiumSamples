// fpdf_utils.h
#pragma once
#include <memory> // std::unique_ptr
#include <string> // std::string
#include <vector> // std::vector
#include <stdlib.h> // wcstombs, mbstowcs

#ifdef _WIN32
#else
#   include <sys/stat.h> // stat
#	include <unistd.h> // access
#   include <string.h> // strdup
#   include <libgen.h> // dirname, basename
#endif

namespace {

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

	auto _U2A = [](const std::wstring& wstr) -> std::string {
		std::vector<char> strVector(wstr.length() + 1, 0);
		wcstombs(&strVector[0], wstr.c_str(), strVector.size());
		return &strVector[0];
	};

	auto _A2U = [](const std::string& str) -> std::wstring {
		std::vector<wchar_t> wstrVector(str.length() + 1, 0);
		mbstowcs(&wstrVector[0], str.c_str(), wstrVector.size());
		return &wstrVector[0];
	};

	auto pathFileExists = [](const char* const filePath) -> bool {
#ifdef _WIN32
#else
		if (access(filePath, F_OK) == 0) {
			return true;
		}
#endif
		return false;
	};

	auto pathIsDirectory = [](const char* const dirPath) -> bool {
#ifdef _WIN32
#else
		struct stat info;
		if (stat(dirPath, &info) == 0 && S_ISDIR(info.st_mode)) {
			return true;
		}
#endif
		return false;
	};

	auto pathAddSeparator = [](const std::string& dirPath) -> std::string {
#ifdef _WIN32
		const char PATH_SEPARATOR = '\\';
#else
		const char PATH_SEPARATOR = '/';
#endif
		std::string addDirPath = dirPath;
		if (dirPath.back() != PATH_SEPARATOR) {
			addDirPath += PATH_SEPARATOR;
		}

		return addDirPath;
	};

	auto pathFindFilename = [](const std::string& filePath) -> std::string {
#ifdef _WIN32
		std::string fileName;
#else
		std::string fileName = basename(AutoMemoryPtr(strdup(filePath.c_str())).get());
#endif
		return fileName;
	};
	

	auto removeExt = [](const std::string& fileName) -> std::string {
		size_t lastIndex = fileName.find_last_of(".");
		std::string rawName = fileName.substr(0, lastIndex);
		return rawName;
	};
}
