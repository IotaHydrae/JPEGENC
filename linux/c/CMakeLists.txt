cmake_minimum_required(VERSION 3.10)

project(jpegenc LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
SET(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} -Wl,--print-memory-usage")
SET(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} -Wl,--print-memory-usage")

include_directories(${CMAKE_CURRENT_LIST_DIR})

add_executable(${PROJECT_NAME} main.c jpegenc.c)

target_compile_definitions(${PROJECT_NAME} PUBLIC -D__LINUX__)
