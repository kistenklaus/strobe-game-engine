

find_package(glfw3 REQUIRED)
find_package(OpenGL REQUIRED)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/lib/glad")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/lib/glm")

add_library(low_level_renderer
        src/internal/Buffer.cpp
        src/internal/Buffer.h
        src/internal/Geometrie.cpp
        src/internal/Geometrie.h
        src/internal/IndexAttribute.cpp
        src/internal/IndexAttribute.h
        src/internal/Shader.cpp
        src/internal/Shader.h
        src/internal/ShaderProgram.cpp
        src/internal/ShaderProgram.h
        src/internal/Texture2D.cpp
        src/internal/Texture2D.h
        src/internal/UboBindingConventions.cpp
        src/internal/UboBindingConventions.h
        src/internal/VertexAttribute.cpp
        src/internal/VertexAttribute.h
        src/Window.cpp
        src/Window.h
        include/strobe/low_level_renderer.h test/test_executable/main.cpp)

target_link_libraries(low_level_renderer PRIVATE glfw OpenGL glad glm::glm assets)
target_include_directories(low_level_renderer PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/test/test_executable)
