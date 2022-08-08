#version 400

in vec2 position;
in vec2 textureCoord;

uniform mat3 window;
uniform mat3 transform;

out vec2 uv;

void main(){
	vec3 worldPos = transform * vec3(position, 1.0);
	gl_Position = vec4((window * worldPos).xy, 0.0, 1.0);
	uv = textureCoord;
}