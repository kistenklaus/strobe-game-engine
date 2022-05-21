cmake_minimum_required(VERSION 3.22)
project(lvulkan)

set(CMAKE_CXX_STANDARD 20)
add_compile_options(-Wall -g -O3) #required for x86 instructions

find_package(Vulkan REQUIRED)
find_package(glfw3 REQUIRED)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

file(GLOB_RECURSE SOURCE_FILES "src/*.cpp" "src/*.h" "src/*.hpp" "include/*.h" "include/*.hpp")

include_directories(src/)
add_executable(${PROJECT_NAME} ${SOURCE_FILES})
target_include_directories(${PROJECT_NAME} PUBLIC ${Vulkan_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PUBLIC glfw ${Vulkan_LIBRARIES})
