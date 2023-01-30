cmake_minimum_required(VERSION 3.22)
project(strobe)

set(CMAKE_CXX_STANDARD 20)
add_compile_options(-Wall -g -O0)

add_subdirectory(threading)
add_subdirectory(asset_loading)
add_subdirectory(renderer_opengl)
