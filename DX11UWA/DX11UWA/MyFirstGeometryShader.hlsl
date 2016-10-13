cbuffer ModelViewProjectionConstantBuffer : register(b0)
{
	matrix model;
	matrix view;
	matrix projection;
	float4 isInstance;
	float4 isSkyBox;
};

struct GeometryShaderInput
{
	float4 pos : SV_POSITION; //for this shader, i just really change the pos
	float2 uv : TEXCOORD;
	float3 normal : NORMAL;
	float3 tangent : TANGENT;
	float3 binormal : BINORMAL;
};

struct GeometryShaderOutput
{
	float4 pos : SV_POSITION;
	float2 uv : TEXCOORD;
	float3 normal : NORMAL;
	float4 worldPosition : POSITION0;
	float4 localPosition : POSITION1;
	float3 tangent : TANGENT;
	float3 binormal : BINORMAL;
	//float3 pixelColor : COLOR; //I add color in GS
};


//I want to make grass (triangles) out of vertex I send in

[maxvertexcount(3)]
void main(point GeometryShaderInput input[1], inout TriangleStream<GeometryShaderOutput> output)
{
	GeometryShaderOutput triangleVertices[3];

	//left point
	triangleVertices[0].pos = input[0].pos;
	triangleVertices[0].pos.x = input[0].pos.x - 0.05f;
	triangleVertices[0].uv.x = 0;
	triangleVertices[0].uv.y = 1;

	//top point
	triangleVertices[1].pos = input[0].pos;
	triangleVertices[1].pos.y = input[0].pos.y + 0.2f;
	triangleVertices[1].uv.x = 0.5f;
	triangleVertices[1].uv.y = 0;

	//right point
	triangleVertices[2].pos = input[0].pos;
	triangleVertices[2].pos.x = input[0].pos.x + 0.05f;
	triangleVertices[2].uv.x = 1;
	triangleVertices[2].uv.y = 1;

	//triangleVertices[2].pos.y = input[0].pos.z + 0.5f;


	for (int i = 0; i < 3; ++i)
	{
		triangleVertices[i].localPosition = triangleVertices[i].pos;
		triangleVertices[i].normal = mul(input[0].normal, model);

		//apply WVP
		triangleVertices[i].pos = mul(triangleVertices[i].pos, model);
		triangleVertices[i].worldPosition = triangleVertices[i].pos; //Store worldpos
		triangleVertices[i].pos = mul(triangleVertices[i].pos, view);
		triangleVertices[i].pos = mul(triangleVertices[i].pos, projection);

		//apply world to normals
		triangleVertices[i].normal = mul(input[0].normal, model);

		//apply world to tangent and binormal
		triangleVertices[i].tangent = normalize(mul(input[0].tangent, model));
		triangleVertices[i].binormal = normalize(mul(input[0].binormal, model));


		//triangleVertices[i].pixelColor = float3(0, 1, 0); //make grass green
		output.Append(triangleVertices[i]); //wind in clockwise
	}

}