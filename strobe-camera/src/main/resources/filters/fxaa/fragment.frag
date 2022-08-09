#version 420 core

const vec3 luma = vec3(0.299, 0.598, 0.114);
const float fxaaMaxSpan = 8;
const float fxaaReduceMin = 1.0/128.0;
const float fxaaReduceMul = 1.0/8.0;

uniform sampler2D source;
uniform vec2 texelSize;

in vec2 uv;

out vec4 fragColor;

void main(){

    float lumaM = dot(luma, texture(source, uv.xy).rgb);
    float lumaTL = dot(luma, texture(source, uv.xy + vec2(-1,-1)*texelSize.xy).rgb);
    float lumaTR = dot(luma, texture(source, uv.xy + vec2(1,-1)*texelSize.xy).rgb);
    float lumaBL = dot(luma, texture(source, uv.xy + vec2(-1,1)*texelSize.xy).rgb);
    float lumaBR = dot(luma, texture(source, uv.xy + vec2(1,1)*texelSize.xy).rgb);

    vec2 dir;
    dir.x = -((lumaTL + lumaTR) - (lumaBL + lumaBR));
    dir.y = ((lumaTL + lumaBL) - (lumaTR + lumaBR));

    float dirReduce = max((lumaTL + lumaTR + lumaBL + lumaBR) * 0.25 * fxaaReduceMul, fxaaReduceMin);
    float dirAdj = 1.0/(min(abs(dir.x),abs(dir.y)) + dirReduce);

    dir = min(vec2(fxaaMaxSpan, fxaaMaxSpan), max(vec2(-fxaaMaxSpan, -fxaaMaxSpan), dir * dirAdj)) * texelSize.xy;

    vec3 s1 = 0.5 * (texture(source,uv.xy + dir * (0.33-0.5)).xyz + texture(source,uv.xy + dir * (0.66-0.5)).xyz);
    vec3 s2 = s1 * 0.5  + 0.25 * (texture(source,uv.xy + dir * (-0.5)).xyz +texture(source,uv.xy + dir * 0.5).xyz);

     float lumaMin = min(lumaTL, min(min(lumaTR, lumaBL), min(lumaBR, lumaM)));
     float lumaMax = max(lumaTL, max(max(lumaTR, lumaBL), max(lumaBR, lumaM)));
     float l2 = dot(luma, s2);

    if(l2 < lumaMin || l2 > lumaMax){
        fragColor = vec4(s1,1);
    }else{
        fragColor = vec4(s2,1);
    }
}