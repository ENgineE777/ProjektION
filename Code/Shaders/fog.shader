
Texture2D diffuseMap : register(t0);
SamplerState samLinear : register(s0);

Texture2D fogMap : register(t1);
SamplerState fogLinear : register(s1);

cbuffer vs_params : register( b0 )
{
    float4 desc[2];
    matrix trans;
    matrix view_proj;
};

cbuffer ps_params : register( b0 )
{
	float4 color;
};

struct VS_INPUT
{
    float2 position : POSITION;
};

struct PS_FOG_INPUT
{
    float4 pos : SV_POSITION;
    float2 texCoord : TEXCOORD;
    float2 texCoord2 : TEXCOORD1;
};

PS_FOG_INPUT VS(VS_INPUT input)
{
    float4 posTemp = float4(desc[0].x + desc[0].z * input.position.x,
							desc[0].y - desc[0].w * input.position.y, 0, 1.0f);
	
    PS_FOG_INPUT output = (PS_FOG_INPUT) 0;

    float4 pos = mul(float4(posTemp.x, posTemp.y, 0.0f, 1.0f), trans);   

    output.texCoord2 = float2(pos.x / (12.0f), pos.y / (12.0f));

    output.pos = mul(pos, view_proj);

    output.texCoord = float2(desc[1].x + desc[1].z * input.position.x, desc[1].y + desc[1].w * input.position.y);
    
    
    return output;
}

float4 PS(PS_FOG_INPUT input) : SV_Target
{   
    float4 clr = diffuseMap.Sample(samLinear, input.texCoord);

    float2 texCoord = 0.0f;
    float intensity = 0.0f;
    
    texCoord = input.texCoord2 + color.b * float2(0.1f, 0.025f);
    intensity += fogMap.Sample(fogLinear, texCoord).r;

    texCoord = input.texCoord2 + color.b * float2(0.15f, -0.1f) * 1.1f;
    intensity += fogMap.Sample(fogLinear, texCoord).g;

    texCoord = input.texCoord2 + color.b * float2(-0.25f, 0.075f) * 0.8f;
    intensity += fogMap.Sample(fogLinear, texCoord).b;

    intensity *= 0.333f;

    return float4(1.0f, 1.0f, 1.0f, color.a * clr.a * intensity);    
}