
struct VS_INPUT
{
	float2 position : POSITION;
};

struct PS_INPUT
{
	float4 Pos : SV_POSITION;
	float2 texCoord : TEXCOORD;
};

Texture2D sceneRT : register(t0);
SamplerState sceneRTLinear : register(s0);

Texture2D rt0 : register(t1);
SamplerState rt0Linear : register(s1);

Texture2D rt1 : register(t2);
SamplerState rt1Linear : register(s2);

PS_INPUT VS( VS_INPUT input )
{
	PS_INPUT output = (PS_INPUT)0;

	output.Pos = float4(input.position.x, input.position.y, 0.5f, 1.0);
	output.texCoord = float2(input.position.x, input.position.y);

	return output;
}

float4 AdjustSaturation(float4 color, float saturation)
{
	float grey = dot(float3(color.x, color.y, color.z), float3(0.3, 0.59, 0.11));

	return lerp(grey, color, saturation);
}

float4 PS( PS_INPUT input) : SV_Target
{
	float2 texCoord = input.texCoord;
	texCoord = texCoord * 0.5f + 0.5f;
	texCoord.y = 1 - texCoord.y;

	float4 scene = sceneRT.Sample(sceneRTLinear, texCoord);
	float4 blured = rt0.Sample(rt0Linear, texCoord);
	float4 shadow = rt1.Sample(rt1Linear, texCoord);

	float grey = dot(float3(blured.x, blured.y, blured.z), float3(0.3, 0.59, 0.11));

	float3 saturated = lerp(blured.xyz, float3(grey, grey, grey), 0.8);
	// Adjust color saturation and intensity.
	//bloom = AdjustSaturation(bloom, 2.0f) * 1.0f;
	//base = AdjustSaturation(base, 1.0f) * 1.0f;

	// Darken down the base image in areas where there is a lot of bloom,
	// to prevent things looking excessively burned-out.
	//base *= (1 - saturate(bloom));

	// Combine the two images.

	return float4(lerp(scene.xyz, saturated, shadow.r), 1.0);
	//return shadow;
}