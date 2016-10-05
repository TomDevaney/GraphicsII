#pragma once
struct SkyBoxPixelShaderInput
{
	float3 uv : TEXCOORD;
	float4 pos: SV_POSITION;
	float3 normal : NORMAL;
	float4 worldPosition : POSITION;
	float3 tangent : TANGENT;
	float3 binormal : BINORMAL;
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
};

TextureCube skyBoxTexture : register(t0);
texture2D normalTexture : register(t1);
SamplerState filters[2] : register(s0);

// A pass-through function for the (interpolated) color data.
float4 main(SkyBoxPixelShaderInput input) : SV_TARGET
{
	float3 color;
	float3 dirColor = 0;
	float3 pointColor = 0;
	float3 spotColor = 0;
	float3 bumpMap = 0;

	color = skyBoxTexture.Sample(filters[0], input.uv);

	bumpMap = normalTexture.Sample(filters[0], input.uv);
	bumpMap = (bumpMap * 2.0f) - 1.0f;
	input.normal = normalize((bumpMap.x * input.tangent) + (bumpMap.y * input.binormal) + (bumpMap.z * input.normal));

	if (typeOfLight.x) //directional Light
	{
		float3 lightColor;
		float lightRatio;
		float3 black = { 0.2f, 0.2f, 0.2f };

		lightRatio = saturate(dot(normalize(dirLightNorm), normalize(input.normal))); //dirLightNorm isn't normalized necessarily, but input.normal should be
		lightColor = (lightRatio + ambientLight) * dirLightColor * black;

		dirColor = lightColor * color;
	}

	if (typeOfLight.y) //point light
	{
		float3 lightColor2;
		float lightRatio2;
		float3 black = { 0.2f, 0.2f, 0.2f };
		float3 lightDir;
		float attenuation;

		lightDir = pointLightPosition - input.worldPosition;
		attenuation = 1 - saturate(length(lightDir) / lightRadius.x);
		lightRatio2 = saturate(dot(normalize(lightDir), normalize(input.normal)));

		lightColor2 = pointLightColor * saturate(lightRatio2 * attenuation) * black;
		pointColor = lightColor2 * color;
	}

	if (typeOfLight.z) //spot light
	{
		float3 lightColor;
		float3 black = { 0.2f, 0.2f, 0.2f };
		float3 lightDir;
		float lightRatio;
		float attenuation;
		float surfaceRatio;
		float spotFactor;

		lightDir = normalize(spotLightPosition - input.worldPosition);

		surfaceRatio = saturate(dot(-lightDir, normalize(coneDirection)));
		spotFactor = (surfaceRatio > coneRatio) ? 1 : 0;

		lightRatio = saturate(dot(lightDir, normalize(input.normal)));

		lightColor = saturate(lightRatio /** attenuation*/) * black * spotLightColor * spotFactor;
		spotColor = lightColor * color;
	}


	return float4(saturate(dirColor + pointColor + spotColor), 1.0f);
}