#version 420 core

layout(location=0) in vec2 position;
layout(location=1) in vec2 textureCoords;

out vec2 uv;

void main(){
    uv = textureCoords;
    gl_Position = vec4(position.xy, 0.0f, 1.0f);
}