# CMakeList.txt: pdfiumSamples에 대한 CMake 프로젝트, 여기에 소스를 포함하고
# 프로젝트 특정 논리를 정의합니다.
#
cmake_minimum_required (VERSION 2.8)

project ("pdfiumSample01")

###
# pfdium 설치위치 설정
set (PDFium_DIR ${CMAKE_SOURCE_DIR}/../pdfium)
# 메시지 출력
message(STATUS "\${CMAKE_SOURCE_DIR} = ${CMAKE_SOURCE_DIR}")
message(STATUS "\${PDFium_DIR} = ${PDFium_DIR}")

###
# 패키지 찾기 - PDFiumConfig.cmake 파일 인클루드
find_package(PDFium REQUIRED)

###
# 이 프로젝트의 실행 파일에 소스를 추가합니다.
add_executable (pdfiumSample01 "pdfiumSample01.cpp" )

###
# 실행파일 생성후에 지정
target_link_libraries(pdfiumSample01 pdfium)

###
# DLL 실행파일 실행파일 위치로 복사
# ${CMAKE_COMMAND} -E copy_if_different : cmake -E copy_if_different
# $<TARGET_FILE:pdfium> : 주 파일 (.exe, .so.1.2, .a)의 전체 경로입니다. 여기서은 pdfium대상의 이름입니다.
# $<TARGET_FILE_DIR:pdfiumSample01> : 기본 파일 (.exe, .so.1.2, .a)의 디렉토리입니다. 여기서은 pdfiumSample01대상의 이름입니다.
add_custom_command(
	TARGET pdfiumSample01 POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:pdfium> $<TARGET_FILE_DIR:pdfiumSample01>)