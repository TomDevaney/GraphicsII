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

cbuffer ModelViewProjectionConstantBuffer : register(b0)
{
	matrix model;
	matrix view;
	matrix projection;
	float4 isInstance;
	//float4 hasNormMap;
};

struct VertexShaderInput
{
	float3 pos : POSITION;
	float2 uv : TEXCOORD;
	float3 normal : NORMAL;
	float3 tangent : TANGENT;
	float3 binormal : BINORMAL;
};

struct InstanceData
{
	float3 pos : INSTANCEPOS;
};

SkyBoxPixelShaderInput main(VertexShaderInput input, InstanceData instanceInput)
{
	SkyBoxPixelShaderInput output = (SkyBoxPixelShaderInput)0;

	float4 pos;
	float4 norm;

	//handle pos
	pos = float4(input.pos, 1.0f);

	pos = mul(pos, model);
	output.worldPosition = pos;
	pos = mul(pos, view);
	pos = mul(pos, projection);
	output.pos = pos.xyww;

	//handle uv
	output.uv = input.pos;

	//handle normal
	norm = float4(input.normal, 0);

	norm = mul(norm, model);
	output.normal = norm;

	return output;
}