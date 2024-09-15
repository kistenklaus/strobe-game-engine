cmake_minimum_required(VERSION 3.3)
project(lvulkan CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Vulkan REQUIRED)
find_package(glfw3 REQUIRED)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

add_executable(${PROJECT_NAME} 
  src/main.cpp
  src/vk/vk.cpp
  src/vk/ext.cpp
  src/glfw/glfw.cpp
)
target_include_directories(${PROJECT_NAME} 
  PUBLIC 
    ${Vulkan_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIRECTORY}/src
)
target_link_libraries(${PROJECT_NAME} 
  PUBLIC 
    glfw 
    ${Vulkan_LIBRARIES}
)
# target_compile_options(${PROJECT_NAME}
#   PUBLIC
#    -std=c++23
# )
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_26)
