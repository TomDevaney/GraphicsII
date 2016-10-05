// A constant buffer that stores the three basic column-major matrices for composing geometry.
cbuffer ModelViewProjectionConstantBuffer : register(b0)
{
	matrix model;
	matrix view;
	matrix projection;
	float4 isInstance;
	float4 isSkyBox;
};

// Per-vertex data used as input to the vertex shader.
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

// Per-pixel color data passed through the pixel shader.
struct PixelShaderInput
{
	float4 pos : SV_POSITION;
	float2 uv : TEXCOORD;
	float3 normal : NORMAL;
	float4 worldPosition : POSITION;
	float4 localPosition : POSITION1;
	float3 tangent : TANGENT;
	float3 binormal : BINORMAL;
};

// Simple shader to do vertex processing on the GPU.
PixelShaderInput main(VertexShaderInput input, InstanceData instanceInput)
{
	PixelShaderInput output;

	float4 pos;
	float4 norm;

	if (isInstance.x)
	{
		pos = float4(input.pos + instanceInput.pos, 1.0f);
	}
	else
	{
		pos = float4(input.pos, 1.0f);
	}

	norm = float4(input.normal, 0);

	if (isSkyBox.x)
	{
		output.localPosition.x = pos.x;
		output.localPosition.y = pos.y;
		output.localPosition.z = pos.z;
	}

	// Transform the vertex position into projected space.
	pos = mul(pos, model);
	output.worldPosition = pos;
	pos = mul(pos, view);
	pos = mul(pos, projection);
	output.pos = pos;

	//Transform the normal position into world space
	norm = mul(norm, model);
	output.normal = norm;

	// Pass the color through without modification.
	output.uv = input.uv;
	
	// handle normal mapping
	output.tangent = normalize(mul(input.tangent, model));
	output.binormal = normalize(mul(input.binormal, model));

	return output;
}