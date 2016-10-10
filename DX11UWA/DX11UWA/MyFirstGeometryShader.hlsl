struct PixelShaderInput
{
	float4 pos : SV_POSITION;
	float2 uv : TEXCOORD;
	float3 normal : NORMAL;
	float4 worldPosition : POSITION0;
	float4 localPosition : POSITION1;
	float3 tangent : TANGENT;
	float3 binormal : BINORMAL;
	float3 pixelColor : COLOR;
};

struct GeometryShaderInput
{
	float4 pos : SV_POSITION;
	float2 uv : TEXCOORD;
	float3 normal : NORMAL;
	float4 worldPosition : POSITION0;
	float4 localPosition : POSITION1;
	float3 tangent : TANGENT;
	float3 binormal : BINORMAL;
};

//I have already applied WVP to input
//I want to make grass (triangles) out of vertice(s) I send in

[maxvertexcount(3)]
void main(point GeometryShaderInput input[1], inout TriangleStream<PixelShaderInput> output)
{
	PixelShaderInput triangleVertices[3];

	triangleVertices[0].pos = input[0].pos;
	triangleVertices[0].pos.x = input[0].pos.x - 0.5f;

	triangleVertices[1].pos = input[0].pos;
	triangleVertices[1].pos.x = input[0].pos.x + 0.5f;

	triangleVertices[2].pos = input[0].pos;
	triangleVertices[2].pos.y = input[0].pos.y + 1.0f;

	for (int i = 0; i < 3; ++i)
	{
		triangleVertices[i].pixelColor = float3(0, 1, 0); //make grass green
		output.Append(triangleVertices[0]);
	}

}