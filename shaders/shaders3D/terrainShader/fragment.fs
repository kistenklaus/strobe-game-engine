#version 400 

in vec2 uv;
in vec3 surfaceNormal;
in vec3 toLightVector;
in vec3 toCameraVector;

uniform sampler2D sampler;
uniform vec3 lightColor;
uniform float shineDamper;
uniform float reflectivity;

out vec4 fragColor;

void main(){
	vec3 unitNormal = normalize(surfaceNormal);
	vec3 unitLightVector = normalize(toLightVector);
	vec3 unitToCameraVector = normalize(toCameraVector);
	
	//diffuse
	float nDot1 = dot(unitNormal, unitLightVector);
	float brightness = max(nDot1, 0.15f);
	vec3 diffuse = brightness * lightColor;
	
	//specular
	vec3 lightDirection = -unitLightVector;
	vec3 unitReflectedLight = reflect(lightDirection, unitNormal);
	float specFac = dot(unitReflectedLight, unitToCameraVector);
	specFac = max(specFac, 0);
	float dampedFac = pow(specFac, shineDamper);
	vec3 specular = (dampedFac * reflectivity) * lightColor;
	
	fragColor = vec4(diffuse,1.0) * texture(sampler, uv) + vec4(specular, 1.0);;
}