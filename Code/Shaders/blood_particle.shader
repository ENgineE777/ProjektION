#include "perlin.shader.inc"
#include "common.shader.inc"

cbuffer ps_params : register(b0)
{
	float4 color;
	float4 params[2];
};

float4 PS(PS_INPUT input) : SV_Target
{
    float pn = 0.3+0.7*PerlinNoise2D(input.texCoord.x * 4., input.texCoord.y * 4.);

    float2 uv = input.texCoord;
    uv.x = uv.x - 0.5;
    uv.y = (1. - uv.y) - 0.5;

    float t          = params[0].x;
    float thickness  = params[0].y;
    float angle      = params[0].z;
    float maxLen     = params[0].w;
    float2 fadeLen   = params[1].xy;
    float2 fadeAlpha = params[1].zw;

    float len  = maxLen * smoothstep(fadeLen.y, fadeLen.x, t);
    float fade = smoothstep(fadeAlpha.y, fadeAlpha.x, t);
    float a = PixelSmooth(sdLineSeg(mul(uv, Rot(angle)), float2(0., 0.), float2(len, 0.)) - thickness);

    return float4(color.rgb * pn, a * fade * color.a);
}