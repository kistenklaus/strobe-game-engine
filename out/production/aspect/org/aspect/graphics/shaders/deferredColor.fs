#version 420 core

in vec2 uv;
in vec3 normal;
in vec3 fragPosition;


layout(location = 0) out vec3 gPosition;
layout(location = 1) out vec3 gNormal;
layout(location = 2) out vec4 gAlbedoSpec;


layout(std140, binding = 3) uniform matUbo{
    vec4 color;
    float specular;
};

void main(){
    gPosition = fragPosition;
    gNormal = normalize(normal);
    gAlbedoSpec.rgb = color.rgb;
    gAlbedoSpec.a = specular;
}