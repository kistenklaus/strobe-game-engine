#version 450

layout(location= 0) in vec3 inPosition;

layout(location = 0) out vec3 fragColor;

layout(binding=0, set=0) uniform uniformObject{
  vec4 color;
  vec3 shit;
} ubo;

void main() {
    gl_Position = vec4(inPosition.xy, 0.0, 1.0);
    fragColor = ubo.color.xyz;
}
