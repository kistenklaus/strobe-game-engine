# Clean and structured CMake setup for Vulkan + shaderc + spirv-tools
cmake_minimum_required(VERSION 3.14)
project(strobe CXX)

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


add_subdirectory(libs/core)

# add_subdirectory(subsystems/window)
add_subdirectory(sandbox)


