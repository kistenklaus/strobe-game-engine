
slangc triangle.slang \
    -entry vertexMain \
    -target spirv \
    -profile glsl_460 \
    -o vertex.spv

slangc triangle.slang \
    -entry fragmentMain \
    -target spirv \
    -profile glsl_460 \
    -o fragment.spv
