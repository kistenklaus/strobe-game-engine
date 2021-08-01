#version 420 core

layout(location=0) in vec3 in_position;
layout(location=1) in vec2 in_uv;

layout(std140, binding=0) uniform camera{
   mat4 view;
   mat4 proj;
   vec3 position;
};

uniform mat4 modelMatrix;

out vec2 uv;

void main(){
    uv = in_uv;
    gl_Position = proj * view * modelMatrix * vec4(in_position, 1.0f);
}