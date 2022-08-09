#version 420 core

layout(binding=0) uniform sampler2D screen;
layout(binding=1) uniform sampler2D depthMap;

in vec2 uv;

out vec4 fragColor;

void main(){
    fragColor = texture(screen, uv)* min(1,(1-texture(depthMap, uv).r)*250);

    //fragColor = vec4(vec3(1-(1-texture(depthMap, uv).r)*100), 1.0f);
}