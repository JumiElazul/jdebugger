cmake_minimum_required(VERSION 3.25)

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>" CACHE STRING "MSVC runtime" FORCE)

project(debugger VERSION 0.0.1 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(DEBUGGER_SOURCES
    "src/main.cpp"

    "src/debugger.cpp"
    "src/debugger.h"
)

include(FetchContent)

FetchContent_Declare(linenoise
    GIT_REPOSITORY "https://github.com/antirez/linenoise.git"
    SYSTEM
)

FetchContent_Declare(libelfin
    GIT_REPOSITORY "https://github.com/TartanLlama/libelfin.git"
    SYSTEM
)

message(STATUS "Calling FetchContent_MakeAvailable on linenoise...")
FetchContent_MakeAvailable(linenoise libelfin)

file(GLOB LIBELFIN_SOURCES
    "${libelfin_SOURCE_DIR}/elf/*.cc"
    "${libelfin_SOURCE_DIR}/dwarf/*.cc"
)

add_library(linenoise STATIC ${linenoise_SOURCE_DIR}/linenoise.c)
add_library(libelfin STATIC ${LIBELFIN_SOURCES})

target_include_directories(linenoise PUBLIC
    $<BUILD_INTERFACE:${linenoise_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include>
)

target_include_directories(libelfin PUBLIC
    $<BUILD_INTERFACE:${libelfin_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include>
)

add_executable(debugger ${DEBUGGER_SOURCES})
target_link_libraries(debugger PRIVATE linenoise libelfin)
target_compile_options(debugger PRIVATE
    -ggdb -g
    -Wall -Wextra -Wconversion -Wsign-conversion
    -Wno-unused-parameter -Wno-switch
    -pedantic-errors
    -fsanitize=address -fsanitize=undefined
)
target_link_options(debugger PRIVATE
    -fsanitize=address -fsanitize=undefined
)

set(TEST_SOURCES
    "src/test_main.cpp"
)

add_executable(test ${TEST_SOURCES})
