#version 420 core

const int POINT_LIGHT_COUNT = 10;
const int DIRECTIONAL_LIGHT_COUNT = 3;


in vec3 normal;
in vec3 fragmentPosition;

out vec4 fragColor;

layout(std140,binding=0) uniform camera{
    mat4 view;
    mat4 proj;
    vec3 cameraPosition;
};

layout(std140,binding=1) uniform lights{
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
};

uniform Material material;

vec3 calcDirLight(vec3 lightDir, vec3 lightDiffuse, vec3 lightSpecular,
        vec3 normal, vec3 viewDir, vec3 materialDiffuse, vec3 materialSpecular, float shininess){
    //diffuse
    float diff = max(dot(normal, -lightDir), 0.0);
    //specular blinn
    vec3 halfway = -normalize(viewDir + lightDir);
    float spec = pow(max(dot(normal, halfway), 0.0f), shininess);
    //combine
    vec3 diffuse = lightDiffuse * diff * materialDiffuse;
    vec3 specular = lightSpecular * spec * materialSpecular;
    return (diffuse + specular);
}


void main(){
    vec3 norm = normalize(normal);
    vec3 viewDir = normalize(fragmentPosition - cameraPosition);

    vec3 combined = vec3(0);

    vec3 materialAmbient = material.ambientColor * material.diffuseColor;

    vec3 ambient = ambientLight * materialAmbient;
    combined += ambient;

    for(int i=0;i<directionalLightCount;i++){
        combined += calcDirLight(normalize(directionalLightDir[i]), directionalLightDiffuse[i], directionalLightSpecular[i],
                norm, viewDir, material.diffuseColor, material.specularColor, material.shininess);
    }

    //TODO point lights and spot lights

    fragColor = vec4(combined, 1.0f);
}