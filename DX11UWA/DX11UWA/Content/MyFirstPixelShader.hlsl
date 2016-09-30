// Per-pixel color data passed through the pixel shader.
struct PixelShaderInput
{
	float4 pos : SV_POSITION;
	float2 uv : TEXCOORD;
	float3 normal : NORMAL;
};

cbuffer LightingConstantBuffer : register(b0)
{
	float3 dirLightNorm;
};

texture2D baseTexture : register(t0);
SamplerState filters[2] : register(s0);

// A pass-through function for the (interpolated) color data.
float4 main(PixelShaderInput input) : SV_TARGET
{
	float3 color; //this is temporary. I will eventually calculate color based off of lighting

	color = baseTexture.Sample(filters[0], input.uv);

	return float4(color, 1.0f);
}
