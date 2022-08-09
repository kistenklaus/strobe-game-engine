#version 420 core

layout(location=0) in vec3 in_position;

layout(std140, binding = 0) uniform cameraUbo{
    mat4 view;
    mat4 proj;
    vec3 viewPos;
};

uniform mat4 model;

void main(){
    vec4 worldPos = model * vec4(in_position, 1.0f);
    gl_Position = proj * view * worldPos;
}