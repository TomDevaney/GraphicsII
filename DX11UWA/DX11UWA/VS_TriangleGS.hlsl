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

// Per-pixel color data passed to the geometry shader.
struct VertexShaderOutput
{
	float4 pos : SV_POSITION; //for this shader, i just really change the pos
	float2 uv : TEXCOORD;
	float3 normal : NORMAL;
	float3 tangent : TANGENT;
	float3 binormal : BINORMAL;
};

// Simple shader to do vertex processing on the GPU.
VertexShaderOutput main(VertexShaderInput input, InstanceData instanceInput)
{
	VertexShaderOutput output;

	float4 pos;

	pos = float4(input.pos, 1.0f);

	output.pos = pos;
	output.normal = input.normal;
	output.tangent = input.tangent;
	output.binormal = input.binormal;

	return output;
}