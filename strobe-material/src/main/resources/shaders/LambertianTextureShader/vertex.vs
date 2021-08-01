#version 420 core

layout(location=0) in vec3 in_position;
layout(location=1) in vec2 in_uv;
layout(location=2) in vec3 in_normal;

layout(std140, binding=0) uniform camera{
    mat4 view;
    mat4 proj;
    vec3 cameraPosition;
};

uniform mat4 modelMatrix;

out vec3 normal;
out vec2 uv;
out vec3 fragmentPosition;

void main(){
    uv = in_uv;
    //fixme super inefficient
    normal = mat3(transpose(inverse(modelMatrix))) * in_normal;

    vec4 worldPos = modelMatrix * vec4(in_position, 1.0f);
    fragmentPosition = worldPos.xyz;
    gl_Position = proj * view * worldPos;
}
