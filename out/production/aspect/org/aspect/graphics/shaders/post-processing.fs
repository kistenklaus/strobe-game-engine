#version 420 core


const vec3 luma = vec3(0.299, 0.587, 0.114);

const float FXAA_SPAN = 8.0;
const float FXAA_REDUCE_MIN = 1.0/128.0;
const float FXAA_REDUCE_MUL = 1.0/8.0;

layout(binding=0) uniform sampler2D screen;
layout(binding=1) uniform sampler2D depthMap;
uniform vec2 invScreenSize;

in vec2 uv;

out vec4 fragColor;

void main(){
    fragColor = texture(screen, uv);//* min(1,(1-texture(depthMap, uv).r)*250);

    float lumaM = dot(luma, texture(screen, uv).rgb);
    float lumaTL = dot(luma, texture(screen, uv + vec2(-1.0, -1.0) * invScreenSize).rgb);
    float lumaTR = dot(luma, texture(screen, uv + vec2(1.0, -1.0) * invScreenSize).rgb);
    float lumaBL = dot(luma, texture(screen, uv + vec2(-1.0, 1.0) * invScreenSize).rgb);
    float lumaBR = dot(luma, texture(screen, uv + vec2(1.0, 1.0) * invScreenSize).rgb);

    float lumaMin = min(lumaM, min(lumaTL, min(lumaTR, min(lumaBL, lumaBR))));
    float lumaMax = max(lumaM, max(lumaTL, max(lumaTR, max(lumaBL, lumaBR))));

    vec2 dir;
    dir.x = -((lumaTL + lumaTR) - (lumaBL + lumaBR));
    dir.y = ((lumaTL + lumaBL) - (lumaTR + lumaBR));

    float dirReduce = max((lumaTL + lumaTR + lumaBL + lumaBR) * FXAA_REDUCE_MUL * 0.25, FXAA_REDUCE_MIN);

    float invDirAdj = 1.0/(min(abs(dir.x), abs(dir.y))+dirReduce);

    vec2 texelDir = min(vec2(FXAA_SPAN, FXAA_SPAN),
        max(vec2(-FXAA_SPAN, -FXAA_SPAN), invDirAdj * dir)) * invScreenSize;

    vec3 result1 = (1.0f/2.0f) * (
        texture(screen, uv+ (texelDir*vec2(1.0/3.0 - 0.5))).rgb
        +texture(screen, uv+ (texelDir*vec2(2.0/3.0 - 0.5))).rgb);

    vec3 result2 = result1 * (1.0f/2.0f)  + (1.0f/4.0f) * (
        texture(screen, uv+texelDir*vec2(0.0/3.0 - 0.5f)).rgb
        +texture(screen, uv+texelDir*vec2(3.0/3.0 - 0.5f)).rgb);

    float lumaR2 = dot(luma, result2);
    if (lumaR2 < lumaMin || lumaR2 > lumaMax){
        fragColor = vec4(result1, 1.0f);
    } else {
        fragColor = vec4(result2, 1.0f);
    }
    //fragColor = vec4(vec3(1-(1-texture(depthMap, uv).r)*100), 1.0f);
}
