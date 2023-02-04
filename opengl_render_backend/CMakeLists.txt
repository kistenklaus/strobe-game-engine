
include_directories(lib/embed-resource)
add_subdirectory(lib/embed-resource)

embed_resources(ShaderResources shaders/simple.vs.glsl shaders/simple.fs.glsl)

find_package(glfw3 REQUIRED)
find_package(OpenGL REQUIRED)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/lib/glad")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/lib/glm")

add_library(${PROJECT_NAME}_render_backend
        ${ShaderResources}
        #src/internal/Buffer.cpp
        #src/internal/Buffer.h
        #src/internal/Geometrie.cpp
        #src/internal/Geometrie.h
        #src/internal/IndexAttribute.cpp
        #src/internal/IndexAttribute.h
        #src/internal/Shader.cpp
        #src/internal/Shader.h
        #src/internal/ShaderProgram.cpp
        #src/internal/ShaderProgram.h
        #src/internal/Texture2D.cpp
        #src/internal/Texture2D.h
        #src/internal/UboBindingConventions.cpp
        #src/internal/UboBindingConventions.h
        #src/internal/VertexAttribute.cpp
        #src/internal/VertexAttribute.h
        src/GlfwWindow.cpp
        src/GlfwWindow.h
        include/strobe/renderer_backend.h
        src/RenderBackendRuntime.cpp
        src/RenderBackendRuntime.h
        src/RenderBackend.cpp
        src/RenderBackend.h
        src/buffer/Buffer.cpp
        src/buffer/Buffer.h
        src/resource_id.h
        src/buffer/BufferManager.h
        src/buffer/BufferManager.cpp
        src/geometrie/Geometrie.cpp
        src/geometrie/Geometrie.h
        src/geometrie/GeometrieManager.cpp
        src/geometrie/GeometrieManager.h
        src/geometrie/GeometrieAttribute.cpp
        src/geometrie/GeometrieAttribute.h
        src/geometrie/GeometrieIndices.cpp
        src/geometrie/GeometrieIndices.h
        src/renderobj/RenderObject.cpp
        src/renderobj/RenderObject.h
        src/renderobj/RenderObjectQueue.cpp
        src/renderobj/RenderObjectQueue.h

        src/deprecated/ShaderProgram.cpp
        src/deprecated/Shader.cpp
        src/shader/ShaderProgram.cpp src/shader/ShaderProgram.h src/shader/ShaderProgramManager.cpp src/shader/ShaderProgramManager.h)

target_link_libraries(${PROJECT_NAME}_render_backend
        PRIVATE
        glfw
        OpenGL
        glad
        PUBLIC
        glm::glm
        ${PROJECT_NAME}_assets
        ${PROJECT_NAME}_threading
        )
target_include_directories(${PROJECT_NAME}_render_backend
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        )

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/test/test_executable)