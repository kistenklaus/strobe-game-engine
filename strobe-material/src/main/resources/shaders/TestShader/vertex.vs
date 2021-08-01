#version 420 core

layout(location=0) in vec3 in_position;

layout(std140, binding=0) uniform camera{
    mat4 view;
    mat4 proj;
    vec3 cameraPosition;
};

void main(){
    gl_Position = proj * view * vec4(in_position, 1.0f);
}