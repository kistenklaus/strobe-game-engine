#version 420 core

in vec2 uv;

layout(binding=0) uniform sampler2D screen;

out vec4 fragColor;

void main(){
    fragColor = vec4(texture(screen, uv).rgb, 1.0f);
}