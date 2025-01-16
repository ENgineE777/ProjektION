#include "perlin.shader.inc"
#include "common.shader.inc"

cbuffer ps_params : register(b0)
{
    float4 perlinNoiseParams;
    float4 puddleParams;
    float4 puddles[24]; // [color, params0, params1] x 8
};

float Puddle(int i, float2 p, inout float seed)
{
    float k            = puddles[i * 3 + 0].w;
    float4 params0     = puddles[i * 3 + 1];
    float4 params1     = puddles[i * 3 + 2];
    float2 offs        = params0.xy;
    float2 sphRadius   = params0.zw;
    float count        = params1.x;
    float puddleRadius = params1.y;
    float2 sector      = params1.zw;
    return sdPuddle(p - offs, sphRadius, count, puddleRadius, sector, k, ++seed);
}

float4 PS(PS_INPUT input) : SV_Target
{
    float pv = PerlinNoise2D(input.texCoord.x * perlinNoiseParams.z, input.texCoord.y * perlinNoiseParams.z);
    float pn = 1. + perlinNoiseParams.x * (pv - 1.);

    float2 uv = input.texCoord;
    uv.x = uv.x - 0.5;
    uv.y = (1. - uv.y) - 0.5;

    int puddlesCount = int(puddleParams.x);
    float seed       = puddleParams.y;
    float scale      = puddleParams.z;
    float alpha      = puddleParams.w;
    float needFrame  = perlinNoiseParams.w;

    uv *= scale;

    float2 gv = frac(uv) - 0.5;
    float2 id = floor(uv);

    float4 finalColor = float4(0., 0., 0., 1.);
    float finalPuddle = 1e3;

    float3 puddleColor  = float3(0., 0., 0.);
    float  overlapCount = 0.;

    for (int i = 0; i < puddlesCount; ++i)
    {
        float3 color = puddles[i * 3].rgb;
        float puddle = Puddle(i, uv - gv, seed);

        finalPuddle = min(finalPuddle, puddle);

        float a = PixelSmooth(puddle);
        puddleColor += color * a;
        overlapCount += sign(a);
    }
    puddleColor /= overlapCount;

    finalColor.rgb += puddleColor.rgb * PixelSmooth(finalPuddle) * pn;
    finalColor.a = PixelSmooth(finalPuddle) * alpha;

    if (needFrame > 0.)
    {
        finalColor = BorderFrame(input.texCoord, finalColor);
    }

    return finalColor;
}

float4 PS_SINGLE(PS_INPUT input) : SV_Target
{
    float pv = PerlinNoise2D(input.texCoord.x * perlinNoiseParams.z, input.texCoord.y * perlinNoiseParams.z);
    float pn = 1. + perlinNoiseParams.x * (pv - 1.);

    float2 uv = input.texCoord;
    uv.x = uv.x - 0.5;
    uv.y = (1. - uv.y) - 0.5;

    int puddlesCount = int(puddleParams.x);
    float seed       = puddleParams.y;
    float scale      = puddleParams.z;
    float alpha      = puddleParams.w;
    float needFrame  = perlinNoiseParams.w;

    uv *= scale;

    float2 gv = frac(uv) - 0.5;
    float2 id = floor(uv);

    float4 color = float4(puddles[0].rgb * pn, alpha) * PixelSmooth(Puddle(0, uv - gv, seed));

    if (needFrame > 0.)
    {
        color = BorderFrame(input.texCoord, color);
    }

    return color;
}