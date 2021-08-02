#version 420 core

layout(location=0) in vec3 in_position;

uniform mat4 model;
uniform mat4 view;

void main(){
    gl_Position = view * model * vec4(in_position, 1.0f);
}