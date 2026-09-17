include_guard(GLOBAL)


embed_slang_spirv(strobe-imgui-vertex-shader
  PREFIX ${CMAKE_CURRENT_SOURCE_DIR}/src
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
  PREFIX ${CMAKE_CURRENT_SOURCE_DIR}/src
  SOURCE strobe/imgui/fragment.slang
  SLANG_FLAGS
    -entry main
    -stage fragment
    -target spirv
    -profile spirv_1_6
    -capability spvDescriptorHeapEXT
    -spirv-unified-descriptor-heap-stride
)

add_library(strobe-imgui EXCLUDE_FROM_ALL
  ${CMAKE_CURRENT_SOURCE_DIR}/src/strobe/imgui/imgui.cpp
)
target_include_directories(strobe-imgui
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
)
target_link_libraries(strobe-imgui
  PUBLIC strobe::rhi
  PUBLIC imgui
  PUBLIC imgui-glfw-backend
  PRIVATE strobe-imgui-vertex-shader
  PRIVATE strobe-imgui-fragment-shader
)

# ====== Sandbox ==========
add_executable(strobe-imgui-sandbox EXCLUDE_FROM_ALL
  ${CMAKE_CURRENT_SOURCE_DIR}/sandbox/main.cpp
)

target_link_libraries(strobe-imgui-sandbox
  PRIVATE strobe-imgui
  PRIVATE strobe-window
  PRIVATE strobe-platform
)

target_include_directories(strobe-imgui-sandbox
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
)

