# Clean and structured CMake setup for Vulkan + shaderc + spirv-tools
cmake_minimum_required(VERSION 3.14)
project(strobe CXX)

option(STROBE_BUILD_TESTS "Build the test suite" OFF)
option(STROBE_BUILD_SANDBOXES OFF)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(cmake/colorful.cmake)
include(cmake/dependencies.cmake)
include(cmake/configuration.cmake)


# Subsystem configuration
select_strobe_window_api()
set_strobe_asset_manifest("assets.toml")


add_subdirectory(libs/core)

add_subdirectory(subsystems/window)
add_subdirectory(subsystems/input)
add_subdirectory(subsystems/assets)

add_subdirectory(test)


if (STROBE_BUILD_SANDBOXES)
  add_subdirectory(sandbox)
endif()


