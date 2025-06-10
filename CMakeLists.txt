# Clean and structured CMake setup for Vulkan + shaderc + spirv-tools
cmake_minimum_required(VERSION 3.15)
project(strobe LANGUAGES CXX)

option(STROBE_BUILD_SANDBOXES ON)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDebug" CACHE STRING "MSVC runtime")

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR
    CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(cmake/colorful.cmake)
include(cmake/flags.cmake)
include(cmake/dependencies.cmake)
include(cmake/configuration.cmake)
include(cmake/testing.cmake)


# Subsystem configuration
select_strobe_window_api()
set_strobe_asset_manifest("assets.toml")


add_subdirectory(libs/core)

add_subdirectory(subsystems/window)
add_subdirectory(subsystems/input)
add_subdirectory(subsystems/assets)

if (STROBE_BUILD_SANDBOXES)
  add_subdirectory(sandbox)
endif()


