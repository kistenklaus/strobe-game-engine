#version 420 core

layout(location=0) in vec3 in_position;
layout(location=1) in vec2 in_uv;
layout(location=2) in vec3 in_normal;

layout(std140, binding = 0) uniform cameraUbo{
    mat4 view;
    mat4 proj;
    vec3 viewPos;
};

uniform mat4 model;

out vec2 uv;
out vec3 normal;
out vec3 fragPosition;

void main(){
    uv = in_uv;
    //inefficency!!!
    normal = mat3(transpose(inverse(model))) * in_normal;
    vec4 worldPos = model * vec4(in_position, 1.0f);
    fragPosition = worldPos.xyz;
    gl_Position = proj * view * worldPos;
}