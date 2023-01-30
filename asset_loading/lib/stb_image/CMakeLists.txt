cmake_minimum_required(VERSION 3.22)

project(stb_image)

include_directories(include)

add_library(stb_image include/stb_image.h stb_image.c)

set_target_properties(stb_image PROPERTIES LINKER_LANGUAGE CXX)
target_include_directories(${PROJECT_NAME} PUBLIC include)
