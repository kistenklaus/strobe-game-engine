#version 420 core

in vec2 uv;
in vec3 normal;
in vec3 fragPosition;


layout(location = 0) out vec3 gPosition;
layout(location = 1) out vec3 gNormal;
layout(location = 2) out vec4 gAlbedoSpec;

layout(binding=0) uniform sampler2D diffuseMap;
layout(binding=1) uniform sampler2D specularMap;

void main(){
    gPosition = fragPosition;
    gNormal = normalize(normal);
    gAlbedoSpec.rgb = texture(diffuseMap, uv).rgb;
    gAlbedoSpec.a = texture(specularMap, uv).a;
}