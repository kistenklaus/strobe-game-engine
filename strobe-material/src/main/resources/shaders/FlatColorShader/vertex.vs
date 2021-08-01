#version 420 core

layout(std140, binding=0) uniform camera{
    mat4 view;
    mat4 proj;
    vec3 position;
};

layout(location=0) in vec3 in_position;

uniform mat4 modelMatrix;

void main(){
    gl_Position = proj * view * modelMatrix * vec4(in_position, 1.0f);
}