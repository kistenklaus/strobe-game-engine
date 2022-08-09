#version 420 core

layout(location=0) in vec3 position;

layout (std140, binding=0) uniform cameraUBO
{
	mat4 view;
};

uniform mat4 transform;

void main(){
    gl_Position = view * transform * vec4(position, 1.0f);
}