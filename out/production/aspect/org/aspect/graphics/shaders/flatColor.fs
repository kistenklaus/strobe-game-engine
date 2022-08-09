#version 420 core

layout(std140, binding = 3) uniform matUbo{
    vec3 color;
};

out vec4 fragColor;

void main(){
    fragColor = vec4(color, 1.0f);
}
