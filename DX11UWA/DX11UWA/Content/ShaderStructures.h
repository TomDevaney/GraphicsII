#pragma once

namespace DX11UWA
{
	// Constant buffer used to send MVP matrices to the vertex shader.
	struct ModelViewProjectionConstantBuffer
	{
		DirectX::XMFLOAT4X4 model;
		DirectX::XMFLOAT4X4 view;
		DirectX::XMFLOAT4X4 projection;
		DirectX::XMFLOAT4 isInstance; //just a bool
	};

	// Used to send per-vertex data to the vertex shader.
	struct VertexPositionColor
	{
		DirectX::XMFLOAT3 pos;
		DirectX::XMFLOAT3 color;
	};

	struct VertexPositionUVNormal
	{
		DirectX::XMFLOAT3 pos;
		DirectX::XMFLOAT3 uv;
		DirectX::XMFLOAT3 normal;
	};

	struct InstanceData
	{
		DirectX::XMFLOAT3 pos;
	};

	struct LightingConstantBuffer
	{
		DirectX::XMFLOAT4 typeOfLight;
		DirectX::XMFLOAT4 dirLightNorm;
		DirectX::XMFLOAT4 dirLightColor;
		DirectX::XMFLOAT4 ambientLight;

		DirectX::XMFLOAT4 pointLightPosition;
		DirectX::XMFLOAT4 pointLightColor;
		DirectX::XMFLOAT4 lightRadius;

		DirectX::XMFLOAT4 spotLightPosition;
		DirectX::XMFLOAT4 spotLightColor;
		DirectX::XMFLOAT4 coneRatio;
		DirectX::XMFLOAT4 coneDirection;

	};
}