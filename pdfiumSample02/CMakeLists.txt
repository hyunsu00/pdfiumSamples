# CMakeList.txt: pdfiumSamples에 대한 CMake 프로젝트, 여기에 소스를 포함하고
# 프로젝트 특정 논리를 정의합니다.
#
cmake_minimum_required (VERSION 2.8)

#
# 프로젝트 이름 지정
project ("pdfiumSample02")
# 메시지 출력
message(STATUS "\${PROJECT_NAME} = ${PROJECT_NAME}")

# 빌드 상세 과정을 모두 출력하는 Makefile을 생성
set (CMAKE_VERBOSE_MAKEFILE true)

###
# pfdium 설치위치 설정
set (PDFium_DIR ${CMAKE_SOURCE_DIR}/../pdfium)
# 메시지 출력
message(STATUS "\${CMAKE_SOURCE_DIR} = ${CMAKE_SOURCE_DIR}")
message(STATUS "\${PDFium_DIR} = ${PDFium_DIR}")

###
# 패키지 찾기 - PDFiumConfig.cmake 파일 인클루드
find_package(PDFium REQUIRED)
message(STATUS "\${PDFium_INCLUDE_DIR} = ${PDFium_INCLUDE_DIR}")
message(STATUS "\${CMAKE_CURRENT_LIST_DIR} = ${CMAKE_CURRENT_LIST_DIR}")

###
# 이 프로젝트의 실행 파일에 소스를 추가합니다.
add_executable (${PROJECT_NAME} "pdfiumSample02.cpp")

###
# 실행파일 생성후에 지정
target_link_libraries(${PROJECT_NAME} pdfium)

# LINUX #include <experimental/filesystem> 사용위해
message(STATUS "\${CMAKE_SYSTEM_NAME} = ${CMAKE_SYSTEM_NAME}")
if (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
	message(STATUS "\${CMAKE_COMPILER_IS_GNUCC} = ${CMAKE_COMPILER_IS_GNUCC}")
	message(STATUS "\${CMAKE_CXX_COMPILER_VERSION} = ${CMAKE_CXX_COMPILER_VERSION}")
	set(CMAKE_CXX_FLAGS "-std=c++14 -lstdc++fs")
	target_link_libraries(${PROJECT_NAME} stdc++fs)
endif()

# Windows <filesystem> 사용위해
if (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
	set(CMAKE_CXX_FLAGS "/EHsc /std:c++17")
endif()

###
# DLL 실행파일 실행파일 위치로 복사
# ${CMAKE_COMMAND} -E copy_if_different : cmake -E copy_if_different
# $<TARGET_FILE:pdfium> : 주 파일 (.exe, .so.1.2, .a)의 전체 경로입니다. 여기서은 pdfium대상의 이름입니다.
# $<TARGET_FILE_DIR:pdfiumSample02> : 기본 파일 (.exe, .so.1.2, .a)의 디렉토리입니다. 여기서은 pdfiumSample02대상의 이름입니다.
add_custom_command(
	TARGET pdfiumSample02 POST_BUILD
	COMMENT "pdfium 라이브러리를 복사합니다."
	COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:pdfium> $<TARGET_FILE_DIR:${PROJECT_NAME}>
	COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:${PROJECT_NAME}>/samples/"
	COMMAND ${CMAKE_COMMAND} -E copy_directory "${PROJECT_SOURCE_DIR}/../samples" "$<TARGET_FILE_DIR:${PROJECT_NAME}>/samples/"
	COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:${PROJECT_NAME}>/result/"
)
message(STATUS "\${CMAKE_COMMAND} = ${CMAKE_COMMAND}")


# 나중에 분석용
#message(STATUS "\${CMAKE_SYSTEM_NAME} = ${CMAKE_SYSTEM_NAME}")
#if (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
#	message(STATUS "\${CMAKE_COMPILER_IS_GNUCC} = ${CMAKE_COMPILER_IS_GNUCC}")
#	message(STATUS "\${CMAKE_CXX_COMPILER_VERSION} = ${CMAKE_CXX_COMPILER_VERSION}")
#	if (CMAKE_COMPILER_IS_GNUCC AND CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 6 AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 8)
#		set(CMAKE_CXX_FLAGS "-std=c++14 -lstdc++fs")
#		target_link_libraries(${PROJECT_NAME} /usr/lib/gcc/x86_64-linux-gnu/7/libstdc++fs.a)
#	elseif (CMAKE_COMPILER_IS_GNUCC AND CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 8)
#		set(CMAKE_CXX_FLAGS "-std=c++17")
#	endif()
#else()
#	if (MSVC_VERSION GREATER_EQUAL "1900")
#		include(CheckCXXCompilerFlag)
#		CHECK_CXX_COMPILER_FLAG("/std:c++latest" _cpp_latest_flag_supported)
#		if (_cpp_latest_flag_supported)
#			add_compile_options("/std:c++latest")
#		endif()
#	endif()
#endif()