#version 420 core

in vec2 uv;

layout(std140, binding=0) uniform cameraUbo{
    mat4 view;
    mat4 proj;
    vec3 viewPos;
};

layout(std140, binding=1) uniform lightUbo{
    vec3 lightPosition;
};

layout(binding=0) uniform sampler2D gPositions;
layout(binding=1) uniform sampler2D gNormals;
layout(binding=2) uniform sampler2D gAlbedoSpec;

out vec4 fragColor;

void main(){
    vec3 fragPos = texture(gPositions, uv).rgb;
    vec3 normal = texture(gNormals, uv).rgb;
    vec3 albedo = texture(gAlbedoSpec, uv).rgb;
    float spec = texture(gAlbedoSpec, uv).a;


    vec3 lightDir = normalize(lightPosition - fragPos);
    vec3 diffuse = max(dot(normal, lightDir), 0.0f) * albedo;

    vec3 viewDir = normalize(viewPos - fragPos);
    vec3 reflectDir = reflect(-lightDir, normal);

    float anglespec = pow(max(dot(viewDir, reflectDir), 0.0f), 32);
    vec3 specular = anglespec * spec * diffuse;

    fragColor = vec4(diffuse + specular + vec3(0.1, 0.1, 0.1)*albedo, 1.0f);
}