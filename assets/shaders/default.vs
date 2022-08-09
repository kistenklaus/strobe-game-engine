#version 420 core

layout(location=0) in vec3 position;
layout(location=1) in vec2 textureCoords;

layout (std140, binding=0) uniform cameraUBO
{
	mat4 view;
	mat4 proj;
};

out vec2 uv;

void main() {
	uv = textureCoords;
    gl_Position = proj * view * vec4(position, 1.0);
}
