#version 420 core


uniform vec4 color;

layout(binding = 0) uniform sampler2D texture1;
layout(binding = 2) uniform sampler2D texture2;

in vec2 uv;

out vec4 fragColor;

void main(){
    fragColor = uv.x  * texture(texture1, uv) + (1-uv.x)*texture(texture2, uv) + color;
}