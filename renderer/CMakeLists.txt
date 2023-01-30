cmake_minimum_required(VERSION 3.22)
project(renderer)

set(CMAKE_CXX_STANDARD 20)
add_compile_options(-Wall -g -O0)

add_library(${PROJECT_NAME}
        src/Renderer.cpp
        src/Renderer.h
        ../src/main.cpp)

target_link_libraries(${PROJECT_NAME} PUBLIC include)
