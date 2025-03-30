# Clean and structured CMake setup for Vulkan + shaderc + spirv-tools
cmake_minimum_required(VERSION 3.14)
project(lvulkan CXX)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)

set(EXECUTABLE ${PROJECT_NAME})

file(GLOB_RECURSE SRC_SOURCE_FILES CONFIGURE_DEPENDS src/*.cpp)
file(GLOB_RECURSE SRC_HEADER_FILES CONFIGURE_DEPENDS src/*.hpp)

add_executable(${EXECUTABLE} ${SRC_SOURCE_FILES} ${SRC_HEADER_FILES})

target_include_directories(${PROJECT_NAME}
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

include(cmake/dependencies.cmake)
include(cmake/compile_shaders.cmake)
