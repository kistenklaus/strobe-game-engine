#version 420 core

const int POINT_LIGHT_COUNT = 10;
const int DIRECTIONAL_LIGHT_COUNT = 3;


layout(std140, binding=0) uniform camera{
    mat4 view;
    mat4 proj;
    vec3 cameraPosition;
};

layout(std140, binding=1) uniform lights{
    vec3 ambientLight;
    int directionalLightCount;
    vec3[DIRECTIONAL_LIGHT_COUNT] directionalLightDir;
    vec3[DIRECTIONAL_LIGHT_COUNT] directionalLightDiffuse;
    vec3[DIRECTIONAL_LIGHT_COUNT] directionalLightSpecular;

    vec3[POINT_LIGHT_COUNT] pointLightPositions;
    vec3[POINT_LIGHT_COUNT] pointLightAttenuations;
    vec3[POINT_LIGHT_COUNT] pointLightDiffuses;
    vec3[POINT_LIGHT_COUNT] pointLightSpeculars;
};

struct Material{
    vec3 diffuseColor;
    bool useDiffuseTexture;
};

uniform Material material;
uniform sampler2D diffuseTexture;
uniform sampler2D normalMap;

vec3 calcDirLight(vec3 lightDir, vec3 lightDiffuse, vec3 normal, vec3 materialDiffuse){
    float diff = max(dot(normal, normalize(-lightDir)), 0.0);
    return (lightDiffuse * diff) * materialDiffuse;
}

in vec3 fragPos;
in vec2 uv;
in vec3[DIRECTIONAL_LIGHT_COUNT] tangentDirectionalLightDir;

out vec4 fragColor;


void main(){
    vec3 norm = normalize(texture(normalMap, uv).xyz * 2.0 - 1.0);

    vec3 materialDiffuse = material.diffuseColor;
    if (material.useDiffuseTexture)materialDiffuse *= texture(diffuseTexture, uv).rgb;

    vec3 combined = vec3(0);

    for (int i=0;i<directionalLightCount;i++){
        combined += calcDirLight(normalize(tangentDirectionalLightDir[i]), directionalLightDiffuse[i], norm, materialDiffuse);
    }

    //TODO point lights and spot lights

    fragColor = vec4(combined, 1.0f);
}