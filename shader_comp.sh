slangc triangle.slang \
    -entry vertexMain \
    -target spirv \
    -profile spirv_1_6 \
    -capability spvDescriptorHeapEXT \
    -spirv-unified-descriptor-heap-stride \
    -o vertex.spv

slangc triangle.slang \
    -entry fragmentMain \
    -target spirv \
    -profile spirv_1_6 \
    -capability spvDescriptorHeapEXT \
    -spirv-unified-descriptor-heap-stride \
    -o fragment.spv
