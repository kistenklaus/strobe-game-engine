include_guard(GLOBAL)

embed_slang_spirv(strobe-imgui-vertex-shader
  PREFIX ${CMAKE_CURRENT_SOURCE_DIR}/shaders
  SOURCE strobe/imgui/vertex.slang
  SLANG_FLAGS
    -entry main
    -stage vertex
    -target spirv
    -profile spirv_1_6
    -capability spvDescriptorHeapEXT
    -spirv-unified-descriptor-heap-stride
)

embed_slang_spirv(strobe-imgui-fragment-shader
  PREFIX ${CMAKE_CURRENT_SOURCE_DIR}/shaders
  SOURCE strobe/imgui/fragment.slang
  SLANG_FLAGS
    -entry main
    -stage fragment
    -target spirv
    -profile spirv_1_6
    -capability spvDescriptorHeapEXT
    -spirv-unified-descriptor-heap-stride
)

add_library(strobe-imgui-renderer EXCLUDE_FROM_ALL
  ${CMAKE_CURRENT_SOURCE_DIR}/src/strobe/imgui/renderer/draw.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/strobe/imgui/renderer/frame.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/strobe/imgui/renderer/samplers.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/strobe/imgui/renderer/shaders.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/strobe/imgui/renderer/texture.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/strobe/imgui/renderer/viewport.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/strobe/imgui/renderer.cpp
)
target_include_directories(strobe-imgui-renderer
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(strobe-imgui-renderer
  PUBLIC strobe::rhi
  PUBLIC imgui
  PRIVATE strobe-imgui-vertex-shader
  PRIVATE strobe-imgui-fragment-shader
)



