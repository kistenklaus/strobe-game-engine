
embed_slang_spirv(example-deferred-light-vert-spv
  PREFIX ${CMAKE_CURRENT_SOURCE_DIR}/src
  SOURCE light.vert.slang
  SLANG_FLAGS
    -entry main
    -stage vertex
    -target spirv
    -profile spirv_1_6
    -capability spvDescriptorHeapEXT
    -spirv-unified-descriptor-heap-stride
)

embed_slang_spirv(example-deferred-light-frag-spv
  PREFIX ${CMAKE_CURRENT_SOURCE_DIR}/src
  SOURCE light.frag.slang
  SLANG_FLAGS
    -entry main
    -stage fragment
    -target spirv
    -profile spirv_1_6
    -capability spvDescriptorHeapEXT
    -spirv-unified-descriptor-heap-stride
)



embed_slang_spirv(example-deferred-taa-spv
  PREFIX ${CMAKE_CURRENT_SOURCE_DIR}/src
  SOURCE taa.slang
  SLANG_FLAGS
    -entry main
    -stage compute
    -target spirv
    -profile spirv_1_6
    -capability spvDescriptorHeapEXT
    -spirv-unified-descriptor-heap-stride
)

embed_slang_spirv(example-deferred-tone-mapping-spv
  PREFIX ${CMAKE_CURRENT_SOURCE_DIR}/src
  SOURCE tone_mapping.slang
  SLANG_FLAGS
    -entry main
    -stage compute
    -target spirv
    -profile spirv_1_6
    -capability spvDescriptorHeapEXT
    -spirv-unified-descriptor-heap-stride
)


add_executable(example-deferred EXCLUDE_FROM_ALL
  ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
)

target_include_directories(example-deferred 
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(example-deferred
  PRIVATE strobe-platform
  PRIVATE strobe-core-color
  PRIVATE strobe::rhi
  PRIVATE example-deferred-light-vert-spv
  PRIVATE example-deferred-light-frag-spv
  PRIVATE example-deferred-taa-spv
  PRIVATE example-deferred-tone-mapping-spv
)

target_link_fmt(example-deferred PRIVATE)
