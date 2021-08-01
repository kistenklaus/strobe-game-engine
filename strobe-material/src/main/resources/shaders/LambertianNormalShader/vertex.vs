#version 420 core

const int POINT_LIGHT_COUNT = 10;
const int DIRECTIONAL_LIGHT_COUNT = 3;


layout(location=0) in vec3 in_position;
layout(location=1) in vec2 in_uv;
layout(location=2) in vec3 in_normal;
layout(location=3) in vec3 in_tangent;
layout(location=4) in vec3 in_bitangent;

layout(std140, binding=0) uniform camera{
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


uniform mat4 modelMatrix;

out vec3 fragPos;
out vec2 uv;
out vec3[DIRECTIONAL_LIGHT_COUNT] tangentDirectionalLightDir;

void main(){
    uv = in_uv;

    vec4 worldPos = modelMatrix * vec4(in_position, 1.0f);
    fragPos = worldPos.xyz;

    //fixme super inefficient
    mat3 normalMatrix = transpose(inverse(mat3(modelMatrix)));

    vec3 T = normalize(normalMatrix * in_tangent);
    vec3 N = normalize(normalMatrix * in_normal);
    T = normalize(T - dot(T,N) * N);//GrammSchmitt
    vec3 B = cross(N,T);
    mat3 TBN = transpose(mat3(T,B,N));
    for(int i=0;i<directionalLightCount;i++){
        tangentDirectionalLightDir[i] = TBN * directionalLightDir[i];
    }

    gl_Position = proj * view * worldPos;
}
