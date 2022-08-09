#version 420 core

in vec2 uv;
in vec3 normal;
in vec3 fragPosition;

layout(std140, binding = 0) uniform cameraUbo{
    mat4 view;
    mat4 proj;
    vec3 viewPos;
};

layout(std140, binding = 1) uniform lightUbo{
    vec3 lightPosition;
};

layout(binding = 0) uniform sampler2D diffuseMap;
layout(binding = 1) uniform sampler2D specularMap;

out vec4 fragColor;

void main(){
    vec4 texColor = texture(diffuseMap, uv).rgba;
    if(texColor.a == 0.0)discard;
    float spec = texture(specularMap, uv).a;
    vec3 norm = normalize(normal);

    vec3 lightDir = normalize(lightPosition - fragPosition);
    vec3 diffuse = max(dot(norm, lightDir), 0.0f) * texColor.rgb;

    vec3 viewDir = normalize(viewPos - fragPosition);
    vec3 reflectDir = reflect(-lightDir, norm);

    float anglespec = pow(max(dot(viewDir, reflectDir), 0.0f), 32);
    vec3 specular = anglespec * spec * diffuse;

    fragColor = vec4(diffuse + specular + vec3(0.1, 0.1, 0.1)*texColor.rgb, texColor.a);
}