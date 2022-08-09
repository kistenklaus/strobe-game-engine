#version 420 core

in vec2 uv;

uniform sampler2D TEXTURE_SAMPLER;

out vec4 color;

void main(){
    color = texture(TEXTURE_SAMPLER, uv);
}