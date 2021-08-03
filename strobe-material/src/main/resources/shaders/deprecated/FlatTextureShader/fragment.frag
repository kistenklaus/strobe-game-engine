#version 420 core

in vec2 uv;

uniform sampler2D texture;

out vec4 fragColor;

void main(){
    fragColor = texture(texture, uv);
}