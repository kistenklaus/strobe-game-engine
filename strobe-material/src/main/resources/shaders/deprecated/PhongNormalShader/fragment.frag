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
    vec3 ambientColor;
    vec3 diffuseColor;
    vec3 specularColor;
    float shininess;
    bool useAmbientTexture;
    bool useDiffuseTexture;
    bool useSpecularTexture;
    bool useShininessTexture;
};

uniform Material material;
uniform sampler2D ambientTexture;
uniform sampler2D diffuseTexture;
uniform sampler2D specularTexture;
uniform sampler2D shininessTexture;
uniform sampler2D normalMap;

vec3 calcDirLight(vec3 lightDir, vec3 lightDiffuse, vec3 lightSpecular,
vec3 normal, vec3 viewDir, vec3 materialDiffuse, vec3 materialSpecular, float shininess){
    //diffuse
    float diff = max(dot(normal, -lightDir), 0.0);
    //specular blinn
    vec3 halfway = -normalize(viewDir + lightDir);
    vec3 reflect = reflect(lightDir, normal);
    float spec = pow(max(dot(-viewDir, reflect), 0.0f), shininess);
    //combine
    vec3 diffuse = lightDiffuse * diff * materialDiffuse;
    vec3 specular = lightSpecular * spec * materialSpecular;
    return (diffuse + specular);
}


in vec3 fragPos;
in vec2 uv;
in vec3[DIRECTIONAL_LIGHT_COUNT] tangentDirectionalLightDir;
in vec3 tangentViewPos;
in vec3 tangentFragPos;

out vec4 fragColor;

void main(){
    vec3 norm = normalize(texture(normalMap, uv).xyz * 2.0 - 1.0);

    vec3 viewDir = normalize(tangentFragPos - tangentViewPos);

    vec3 materialAmbient = material.ambientColor;
    if (material.useAmbientTexture)materialAmbient *= texture(ambientTexture, uv).r;
    vec3 materialDiffuse = material.diffuseColor;
    if (material.useDiffuseTexture) materialDiffuse *= texture(diffuseTexture, uv).rgb;
    vec3 materialSpecular = material.specularColor;
    if (material.useSpecularTexture) materialSpecular *= texture(specularTexture, uv).rgb;
    float materialShininess = material.shininess;
    if (material.useShininessTexture) materialShininess *= texture(shininessTexture, uv).r;

    //ambient diffuse interaction
    materialAmbient *= materialDiffuse;

    vec3 combined = vec3(0);

    vec3 ambient = materialAmbient * ambientLight;

    combined += ambient;

    for (int i=0;i<directionalLightCount;i++){
        combined += calcDirLight(normalize(tangentDirectionalLightDir[i]), directionalLightDiffuse[i],
                directionalLightSpecular[i], norm, viewDir, materialDiffuse, materialSpecular, materialShininess);
    }

    //TODO point lights and spot lights

    fragColor = vec4(combined, 1.0f);
}