#version 420 core

uniform vec4 color;

out vec4 fragColor;

void main(){
    fragColor = vec4(color.rgb,1);
}