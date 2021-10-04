#version 420 core

const int DIRECTIONAL_LIGHT_COUNT = 3;

in vec3 normal;
in vec3 fragmentPosition;

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


struct Material{
    vec3 diffuseColor;
};

uniform Material material;

vec3 calcDirLight(vec3 lightDir, vec3 lightDiffuse, vec3 normal, vec3 materialDiffuse){
    float diff = max(dot(normal, normalize(-lightDir)), 0.0);
    return (lightDiffuse * diff) * materialDiffuse;
}


void main(){
    vec3 norm = normalize(normal);

    vec3 combined = vec3(material.diffuseColor*0.1f);

    for (int i=0;i<directionalLightCount;i++){
        combined += calcDirLight(-directionalLightDir[i], directionalLightDiffuse[i], norm, material.diffuseColor);
    }
    fragColor = vec4(combined, 1.0f);
    //fragColor = vec4((normal+vec3(1)*0.5f), 1.0f);

}