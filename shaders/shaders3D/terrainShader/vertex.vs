#version 400

const float density = 0.002;
const float gradient = 2;

in vec3 position;
in vec2 texCoords;
in vec3 normal;

uniform mat4 transformation;
uniform mat4 projection;
uniform mat4 view;
uniform vec3 lightPosition;

out vec2 uv;
out vec3 surfaceNormal;
out vec3 toLightVector;
out vec3 toCameraVector;
out float visibility;



void main(){
	vec4 worldPosition = transformation * vec4(position, 1.0);
	gl_Position = projection *view *worldPosition;
	uv = texCoords;
	surfaceNormal = (transformation * vec4(normal,0.0)).xyz;
	toLightVector = lightPosition - worldPosition.xyz;
	toCameraVector = (inverse(view) * vec4(0.0,0.0,0.0,0.0)).xyz - worldPosition.xyz;
	
	vec4 positionRelativToCam = view * worldPosition;
	float distance = length(positionRelativToCam.xz);
	visibility = exp(-pow((distance*density), gradient));
	visibility = clamp(visibility, 0.0, 1.0);
}
