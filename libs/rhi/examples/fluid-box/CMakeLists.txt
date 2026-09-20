
embed_slang_spirv(fluid-box-splat-vertex-shader
  PREFIX ${CMAKE_CURRENT_SOURCE_DIR}/shaders
  SOURCE splat-vert.slang
  SLANG_FLAGS
    -entry main
    -stage vertex
    -target spirv
    -profile spirv_1_6
    -capability spvDescriptorHeapEXT
    -spirv-unified-descriptor-heap-stride
)

embed_slang_spirv(fluid-box-splat-fragment-shader
  PREFIX ${CMAKE_CURRENT_SOURCE_DIR}/shaders
  SOURCE splat-frag.slang
  SLANG_FLAGS
    -entry main
    -stage fragment
    -target spirv
    -profile spirv_1_6
    -capability spvDescriptorHeapEXT
    -spirv-unified-descriptor-heap-stride
)

embed_slang_spirv(fluid-box-tf-vertex-shader
  PREFIX ${CMAKE_CURRENT_SOURCE_DIR}/shaders
  SOURCE tf-vert.slang
  SLANG_FLAGS
    -entry main
    -stage vertex
    -target spirv
    -profile spirv_1_6
    -capability spvDescriptorHeapEXT
    -spirv-unified-descriptor-heap-stride
)

embed_slang_spirv(fluid-box-tf-fragment-shader
  PREFIX ${CMAKE_CURRENT_SOURCE_DIR}/shaders
  SOURCE tf-frag.slang
  SLANG_FLAGS
    -entry main
    -stage fragment
    -target spirv
    -profile spirv_1_6
    -capability spvDescriptorHeapEXT
    -spirv-unified-descriptor-heap-stride
)

embed_slang_spirv(fluid-box-apply-force-compute-shader
  PREFIX ${CMAKE_CURRENT_SOURCE_DIR}/shaders
  SOURCE apply-force.slang
  SLANG_FLAGS
    -entry main
    -stage compute
    -target spirv
    -profile spirv_1_6
    -capability spvDescriptorHeapEXT
    -spirv-unified-descriptor-heap-stride
)

add_executable(fluid-box EXCLUDE_FROM_ALL
  ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
)

target_link_libraries(fluid-box 
  PRIVATE strobe-platform
  PRIVATE strobe::rhi
  PRIVATE fluid-box-splat-fragment-shader
  PRIVATE fluid-box-splat-vertex-shader
  PRIVATE fluid-box-tf-fragment-shader
  PRIVATE fluid-box-tf-vertex-shader
  PRIVATE fluid-box-apply-force-compute-shader
)

target_link_fmt(fluid-box PRIVATE)
