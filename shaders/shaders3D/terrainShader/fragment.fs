#version 400 

in vec2 uv;
in vec3 surfaceNormal;
in vec3 toLightVector;
in vec3 toCameraVector;
in float visibility;

uniform sampler2D bgTexture;
uniform sampler2D rTexture;
uniform sampler2D gTexture;
uniform sampler2D bTexture;
uniform sampler2D blendMap;
uniform vec3 lightColor;
uniform float shineDamper;
uniform float reflectivity;
uniform vec3 skyColor;

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
	
	//Multitexuring;
	vec4 blendColor = texture(blendMap, uv);
	float bgTexAmount = 1-(blendColor.r + blendColor.g + blendColor.b);
	vec2 tiledUV = uv * 40.0;
	vec4 bgTexColor = texture(bgTexture, tiledUV) * bgTexAmount;
	vec4 rTexColor = texture(rTexture, tiledUV) * blendColor.r; 
	vec4 gTexColor = texture(gTexture, tiledUV) * blendColor.g; 
	vec4 bTexColor = texture(bTexture, tiledUV) * blendColor.b; 
	
	vec4 totalTexColor = bgTexColor + rTexColor + gTexColor + bTexColor;
	
	fragColor = vec4(diffuse,1.0) * totalTexColor + vec4(specular, 1.0);
		fragColor = mix(vec4(skyColor,1.0), fragColor, visibility);
}