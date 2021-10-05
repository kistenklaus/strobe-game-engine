#version 420 core

const int DIRECTIONAL_LIGHT_COUNT = require(DIRECTIONAL_LIGHT_COUNT);
const int MAX_CASTING_DIR_LIGHT = require(MAX_CASTING_DIR_LIGHTS);

layout(location=0) in vec3 in_position;
layout(location=2) in vec3 in_normal;
layout(location=1) in vec2 in_uv;

layout(std140, binding=0) uniform camera{
    mat4 view;
    mat4 proj;
    vec3 cameraPosition;
};

layout(std140, binding=2) uniform shadows {
    mat4[MAX_CASTING_DIR_LIGHT] directionalLightLightSpace;
    vec4[MAX_CASTING_DIR_LIGHT] directionalLightShadowDim;
    int[DIRECTIONAL_LIGHT_COUNT] directionalLightIndices;
    int directionalLightCastingCount;
};

uniform mat4 modelMatrix;

out vec2 uv;
out vec3 normal;
out vec3 fragmentPosition;
out vec4[MAX_CASTING_DIR_LIGHT] fragmentPositionsDirLightSpace;

void main(){
    uv = in_uv;
    //fixme super inefficient
    normal = mat3(transpose(inverse(modelMatrix))) * in_normal;

    vec4 worldPos = modelMatrix * vec4(in_position, 1.0f);
    fragmentPosition = worldPos.xyz;
    gl_Position = proj * view * worldPos;

    vec4[MAX_CASTING_DIR_LIGHT] dlsFragPos;
    //fixme probably a problem with the ubo offset
    for (int i=0;i<directionalLightCastingCount;i++)  {
        dlsFragPos[i] = directionalLightLightSpace[i] * vec4(worldPos.xyz, 1.0f);
    }
    fragmentPositionsDirLightSpace = dlsFragPos;

}
