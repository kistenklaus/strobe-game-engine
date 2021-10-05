#version 420 core

const int DIRECTIONAL_LIGHT_COUNT = require(DIRECTIONAL_LIGHT_COUNT);
const int SHADOW_CASTER_COUNT = require(MAX_CASTING_DIR_LIGHTS);

layout(location=0) in vec3 in_position;

layout(std140, binding=0) uniform camera{
    mat4 view;
    mat4 proj;
    vec3 cameraPosition;
    int cameraIndex;
};

layout(std140, binding=1) uniform lights{
    int directionalLightCount;
    vec3[DIRECTIONAL_LIGHT_COUNT] directionalLightDir;
    vec3[DIRECTIONAL_LIGHT_COUNT] directionalLightAmbient;
    vec3[DIRECTIONAL_LIGHT_COUNT] directionalLightDiffuse;
    vec3[DIRECTIONAL_LIGHT_COUNT] directionalLightSpecular;
};

layout(std140, binding=2) uniform shadows{
    mat4[SHADOW_CASTER_COUNT] directionalLightLightSpace;
    vec4[SHADOW_CASTER_COUNT] directionalLightShadowDim;
    int[DIRECTIONAL_LIGHT_COUNT] directionalLightIndex;
    int directionalLightCastingCount;
};

uniform int lightIndex;

uniform mat4 modelMatrix;

void main(){
    gl_Position = directionalLightLightSpace[lightIndex] * modelMatrix * vec4(in_position, 1.0f);
}
