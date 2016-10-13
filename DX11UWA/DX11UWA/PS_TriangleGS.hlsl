// Per-pixel color data passed through the pixel shader.
struct PixelShaderInput
{
	float4 pos : SV_POSITION;
	float3 pixelColor : COLOR;
};

cbuffer LightingConstantBuffer : register(b0)
{
	float4 typeOfLight;

	float4 dirLightNorm;
	float4 dirLightColor;
	float4 ambientLight;

	float4 pointLightPosition;
	float4 pointLightColor;
	float4 lightRadius; //treat as float

	float4 spotLightPosition;
	float4 spotLightColor;
	float4 coneRatio; //treat as float
	float4 coneDirection;

	float4 isSkyBox;
	float4 isNormalMap;
};

// A pass-through function for the (interpolated) color data.
float4 main(PixelShaderInput input) : SV_TARGET
{
	float4 color;

	color = float4(input.pixelColor, 1);

	return color;
}
