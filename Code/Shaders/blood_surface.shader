#include "perlin.shader.inc"
#include "common.shader.inc"

cbuffer ps_params : register(b0)
{
    float4 perlinNoiseParams;
    float4 surfaceParams;
    float4 surfaces[24]; // [color, params0, params1] x 8
};

float Surface(int i, float2 p, inout float seed)
{
    float k            = surfaces[i * 3 + 0].w;
    float4 params0     = surfaces[i * 3 + 1];
    float4 params1     = surfaces[i * 3 + 2];
    float2 offs        = params0.xy;
    float width        = params0.z;
    float surfaceK     = params0.w;
    float upAngle      = -HALF_PI;
    float gravityAngle = params1.x;
    float step         = params1.y;
    float lineLen      = params1.z;
    float t            = params1.w;
    return sdSurface(p - offs, width, upAngle, gravityAngle, step, k, lineLen, surfaceK, ++seed, t);
}

float4 PS(PS_INPUT input) : SV_Target
{
    float pv = PerlinNoise2D(input.texCoord.x * perlinNoiseParams.z, input.texCoord.y * perlinNoiseParams.z);
    float pn = 1. + perlinNoiseParams.x * (pv - 1.);

    float2 uv = input.texCoord;
    uv.x = uv.x - 0.5;
    uv.y = (1. - uv.y) - 0.5;

    int surfacesCount = int(surfaceParams.x);
    float seed        = surfaceParams.y;
    float scale       = surfaceParams.z;
    float alpha       = surfaceParams.w;
    float needFrame   = perlinNoiseParams.w;

    uv *= scale;

    float2 gv = frac(uv) - 0.5;
    float2 id = floor(uv);

    float4 finalColor = float4(0., 0., 0., 1.);
    float finalPuddle = 1e3;

    float3 puddleColor  = float3(0., 0., 0.);
    float  overlapCount = 0.;

    for (int i = 0; i < surfacesCount; ++i)
    {
        float3 color = surfaces[i * 3].rgb;
        float puddle = Surface(i, uv - gv, seed);

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

    int surfacesCount = int(surfaceParams.x);
    float seed        = surfaceParams.y;
    float scale       = surfaceParams.z;
    float alpha       = surfaceParams.w;
    float needFrame   = perlinNoiseParams.w;

    uv *= scale;

    float2 gv = frac(uv) - 0.5;
    float2 id = floor(uv);

    float4 color = float4(surfaces[0].rgb * pn, alpha) * PixelSmooth(Surface(0, uv - gv, seed));

    if (needFrame > 0.)
    {
        color = BorderFrame(input.texCoord, color);
    }

    return color;
}