######################################################################
# @author      : karl (karlsasssie@gmail.com)
# @file        : CMakeLists
# @created     : Friday Apr 07, 2023 15:39:05 CEST
######################################################################
cmake_minimum_required(VERSION 3.27)

project(strobe VERSION 0.0.0)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(cmake/easy_test.cmake)
include(cmake/libraries.cmake)

add_compile_definitions(STROBE_LOG_FATAL_ERRORS STROBE_LOG_INFOS)

enable_testing()

set(CMAKE_CXX_COMPILER /usr/bin/g++)
set(CMAKE_C_COMPILER /usr/bin/gcc)

# GoogleTest requires at least C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(pil)
add_subdirectory(lia)
add_subdirectory(logging)
add_subdirectory(profiler)

add_subdirectory(renderer)


