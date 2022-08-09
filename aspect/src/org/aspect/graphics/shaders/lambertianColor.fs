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

layout(std140, binding = 3) uniform matUbo{
    vec4 color;
    float specular;
};

out vec4 fragColor;

void main(){
    if(color.a == 0.0)discard;

    vec3 norm = normalize(normal);

    vec3 lightDir = normalize(lightPosition - fragPosition);
    vec3 diffuse = max(dot(norm, lightDir), 0.0f) * color.rgb;

    vec3 viewDir = normalize(viewPos - fragPosition);
    vec3 reflectDir = reflect(-lightDir, norm);

    float anglespec = pow(max(dot(viewDir, reflectDir), 0.0f), 32);
    vec3 spec = anglespec * specular * diffuse;

    fragColor = vec4(diffuse + spec + vec3(0.1, 0.1, 0.1)*color.rgb, color.a);
}