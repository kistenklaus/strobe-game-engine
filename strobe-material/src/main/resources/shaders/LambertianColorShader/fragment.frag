#version 420 core

const int DIRECTIONAL_LIGHT_COUNT = require(DIRECTIONAL_LIGHT_COUNT);
const int MAX_CASTING_DIR_LIGHT = require(MAX_CASTING_DIR_LIGHTS);
const float shadowAcneBiasFactor = 0.004f;
const float minShadowAcneBias = 0.002f;

in vec2 uv;
in vec3 normal;
in vec3 fragmentPosition;
in vec4[MAX_CASTING_DIR_LIGHT] fragmentPositionsDirLightSpace;

out vec4 fragColor;

layout(std140, binding=0) uniform camera{
    mat4 view;
    mat4 proj;
    vec3 cameraPosition;
};

layout(std140, binding=1) uniform lights{
    int directionalLightCount;
    vec3[DIRECTIONAL_LIGHT_COUNT] directionalLightDir;
    vec3[DIRECTIONAL_LIGHT_COUNT] directionalLightAmbient;
    vec3[DIRECTIONAL_LIGHT_COUNT] directionalLightDiffuse;
    vec3[DIRECTIONAL_LIGHT_COUNT] directionalLightSpecular;
};

layout(std140, binding=2) uniform shadows {
    mat4[MAX_CASTING_DIR_LIGHT] directionalLightLightSpace;
    vec4[MAX_CASTING_DIR_LIGHT] directionalLightShadowDim;
    int[DIRECTIONAL_LIGHT_COUNT] directionalLightIndices;
    int directionalLightCastingCount;
};

uniform sampler2D dirShadowMap;

struct Material{
    vec3 diffuseColor;
};

uniform Material material;

vec3 calcDirLight(vec3 lightDir, vec3 lightDiffuse, vec3 normal, vec3 materialDiffuse){
    float diff = max(dot(normal, normalize(-lightDir)), 0.0);
    return (lightDiffuse * diff) * materialDiffuse;
}

float calcShadow(vec4 dlsFragPos, sampler2D shadowMap, vec4 lightShadowDim){
    //perspective division per pixel.
    vec3 projDLSFragPos = dlsFragPos.xyz / 1.0f;
    //normalize (to NDC)
    vec3 NDCFragPos = projDLSFragPos * 0.5f + vec3(0.5f);
    float shadowDepth = texture(shadowMap, lightShadowDim.xy + NDCFragPos.xy * lightShadowDim.zw).r;
    float dlsDepth = NDCFragPos.z;

    float shadow = dlsDepth > shadowDepth ? 1.0f : 0.0f;

    return shadow;
}

void main(){
    vec3 norm = normalize(normal);

    vec3 combined = vec3(material.diffuseColor*0.1f);

    for (int i=0;i<directionalLightCount;i++){
        vec3 light = calcDirLight(directionalLightDir[i], directionalLightDiffuse[i], norm, material.diffuseColor);
        if (directionalLightIndices[i] != -1){
            int casterIndex = directionalLightIndices[i];
            vec4 shadowDim = directionalLightShadowDim[casterIndex];
            float shadow = calcShadow(fragmentPositionsDirLightSpace[i], dirShadowMap, shadowDim);
            combined += (1.0f-shadow) * light;
        } else {
            combined += light;
        }
    }
    fragColor = vec4(combined, 1.0f);
}