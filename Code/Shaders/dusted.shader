
Texture2D diffuseMap : register(t0);
SamplerState samLinear : register(s0);

Texture2D dustMap : register(t1);
SamplerState dustLinear : register(s1);

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

struct PS_DUSTED_INPUT
{
    float4 pos : SV_POSITION;
    float2 texCoord : TEXCOORD;
    float2 texCoord2 : TEXCOORD1;
};

PS_DUSTED_INPUT VS(VS_INPUT input)
{
    float4 posTemp = float4(desc[0].x + desc[0].z * input.position.x,
							desc[0].y - desc[0].w * input.position.y, 0, 1.0f);
	
    PS_DUSTED_INPUT output = (PS_DUSTED_INPUT) 0;

    float4 pos = mul(float4(posTemp.x, posTemp.y, 0.0f, 1.0f), trans);   

    output.texCoord2 = float2(pos.x / (4.0f), pos.y / (4.0f));

    output.pos = mul(pos, view_proj);

    output.texCoord = float2(desc[1].x + desc[1].z * input.position.x, desc[1].y + desc[1].w * input.position.y);
    
    
    return output;
}

float4 PS(PS_DUSTED_INPUT input) : SV_Target
{   
    float4 clr = diffuseMap.Sample(samLinear, input.texCoord) * color;     
    float4 dust = dustMap.Sample(dustLinear, input.texCoord2);
    float dustKoef = clamp(clr.a * 2.0f, 0.0f, 1.0f);
    
    return float4(clr.r + dust.r * 0.45f * dustKoef, clr.g + dust.g * 0.5f * dustKoef, clr.b + dust.b * dustKoef, saturate(clr.a + dust.a * 0.2f * dustKoef)); 
}