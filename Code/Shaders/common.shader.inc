static const float PI      = 3.141592;
static const float TWO_PI  = 2. * PI;
static const float HALF_PI = 0.5 * PI;

static const float NoiseMapSize = 256.;

cbuffer vs_params : register(b0)
{
    float4 desc[2];
    matrix trans;
    matrix view_proj;
};

struct VS_INPUT
{
    float2 position : POSITION;
};

struct PS_INPUT
{
    float4 pos : SV_POSITION;
    float2 texCoord : TEXCOORD;
};

Texture2D noiseMap : register(t0);
// SamplerState samLinear : register(s0);
SamplerState samPoint
{
    Filter   = MIN_MAG_MIP_POINT;
    AddressU = Wrap;
    AddressV = Wrap;
};

PS_INPUT VS(VS_INPUT input)
{
    float2 descPos  = desc[0].xy;
    float2 descSize = desc[0].zw;

    float2 descUV  = desc[1].xy;
    float2 descDUV = desc[1].zw;

	float2 posTemp = descPos + descSize * input.position * float2(1., -1.);

	PS_INPUT output = (PS_INPUT)0;

    float4 pos = mul(float4(posTemp.x, posTemp.y, 0., 1.), trans);
	output.pos = mul(pos, view_proj);

    output.texCoord = descUV + descDUV * input.position;

	return output;
}

float4 sampleNoiseMap(float seed)
{
    float  i = floor(fmod(seed, NoiseMapSize * NoiseMapSize));
    float2 p = float2(floor(fmod(i, NoiseMapSize)),
                      floor(i / NoiseMapSize)) / NoiseMapSize;
    return noiseMap.SampleLevel(samPoint, p, 0);
}

// [0.; 1.]
float random1(float seed)
{
    return sampleNoiseMap(seed).r;
}

float2 random2(float seed)
{
    return sampleNoiseMap(seed).rg;
}

float3 random3(float seed)
{
    return sampleNoiseMap(seed).rgb;
}

float4 random4(float seed)
{
    return sampleNoiseMap(seed);
}

// [-1.; 1.]
float srandom1(float seed)
{
    return srandom1(seed) * 2. - 1.;
}

float2 srandom2(float seed)
{
    return srandom2(seed) * 2. - 1.;
}

float3 srandom3(float seed)
{
    return srandom3(seed) * 2. - 1.;
}

float4 srandom4(float seed)
{
    return srandom4(seed) * 2. - 1.;
}

float smin(float a, float b, float k)
{
    float h = clamp(0.5+0.5*(b-a)/k, 0., 1.);
    return lerp(b, a, h) - k*h*(1.0-h);
}

float rad(float deg)
{
    return deg * 3.141592 / 180.;
}

float sdSphere(float2 p, float r)
{
    return length(p) - r;
}

float sdLineSeg(float2 p, float2 a, float2 b)
{
    float2 ap=p-a, ab=b-a;
    float t = clamp(dot(ap, ab)/dot(ab, ab), 0., 1.);
    float2 c = a + ab*t;
    return length(p-c);
}

float2x2 Rot(float a)
{
    float s = sin(a), c = cos(a);
    return float2x2(c, -s, s, c);
}

float2 RandomDir(float from, float to, float seed)
{
    float a = from + random1(seed) * (to - from);
    return float2(cos(a), sin(a));
}

// [-0.5; 0.5]
float2 RandomPosSquare(float seed)
{
    return random2(seed) - 0.5;
}

float2 Polar(float r, float a)
{
    return float2(cos(a), sin(a)) * r;
}

float PixelSmooth(float x)
{
    return (1. - sign(x)) * 0.5;
}

float sdSurface(float2 uv, float width, float upAngle, float gravityAngle, float step, float k, float lineLen, float surfaceK, float seed, float t)
{
    float2 upDir      = float2(cos(upAngle), sin(upAngle));
    float2 gravityDir = float2(cos(upAngle + gravityAngle), sin(upAngle + gravityAngle));
    float2 leftDir    = float2(-upDir.y, upDir.x);

    float puddle = 1.e3;

    float count  = ceil(width / step);
    float center = 0.5 * (count - 1.) * step;

    for (float i = 0.; i < count; ++i)
    {
        float2 n    = random2(++seed);
        float2 offs = leftDir * (step * i - center) + upDir * (n.x * 2. - 1.) * surfaceK;
        float len   = lerp(1., lineLen * n.y, 1. - (1. - t) * (1. - t));
        puddle = smin(puddle, sdLineSeg(uv, offs, offs + gravityDir * len) - 1., k);
    }

    return puddle;
}

float sdPuddle(float2 uv, float2 sphRadius, float count, float puddleRadius, float2 sector, float k, float seed)
{
    float puddle = 1e3;
    for (float i = 0.; i < count; i += 1.)
    {
        float3 n    = random3(++seed);
        float r     = lerp(sphRadius.x, sphRadius.y, n.x);
        float2 offs = Polar(n.y * puddleRadius, lerp(sector.x, sector.y, n.z));

        puddle = smin(puddle, sdSphere(uv - offs, r), k);
    }
    
    return puddle;
}

float4 BorderFrame(float2 uv, float4 color)
{
    uv.x = uv.x - 0.5;
    uv.y = (1. - uv.y) - 0.5;
    if (abs(uv.x) >= 0.47 || abs(uv.y) >= 0.47)
    {
        color.r = 1.;
        color.a = 1.;
    }
    return color;
}