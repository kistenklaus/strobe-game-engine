#version 450

layout(location = 0) in vec3 fragColor;

layout(location = 0) out vec4 outColor;


layout(binding=0, set=0) uniform uniformObject{
  vec4 color;
  vec3 shit;
} ubo;

void main() {
    outColor = vec4(fragColor, 1.0);
}
