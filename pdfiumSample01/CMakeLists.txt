# CMakeList.txt: pdfiumSamples에 대한 CMake 프로젝트, 여기에 소스를 포함하고
# 프로젝트 특정 논리를 정의합니다.
#
cmake_minimum_required (VERSION 2.8)

project ("pdfiumSamples01")

# pfdium 설치위치 설정
set (PDFium_DIR ${CMAKE_SOURCE_DIR}/../pdfium)

# 패키지 찾기
find_package(PDFium REQUIRED)

# 이 프로젝트의 실행 파일에 소스를 추가합니다.
add_executable (pdfiumSamples01 "pdfiumSamples01.cpp" )

# 실행파일 생성후에 지정
target_link_libraries(pdfiumSamples01 pdfium)

# DLL 실행파일 위치로 복사
add_custom_command(
	TARGET pdfiumSamples01 POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:pdfium> $<TARGET_FILE_DIR:pdfiumSamples01>)