#pragma once
#include <string>
#include <fstream>
#include <d3d11.h>
#include <DirectXMath.h>
#include <iostream>
#include "..\Common\DeviceResources.h"
#include <vector>
#include "..\Common\DirectXHelper.h"
#include "Content/ShaderStructures.h"
#include "..\Common\StepTimer.h"

using namespace DirectX;
using namespace DX11UWA;
using namespace Windows::Foundation;

#pragma comment(lib, "d3d11.lib")

using namespace std;

struct Vertex
{
	XMFLOAT3 position;
	XMFLOAT2 uv;
	XMFLOAT3 normal;

	bool operator==(const Vertex& other) const
	{
		bool result = false;

		if (this->position.x == other.position.x && this->position.y == other.position.y && this->position.z == other.position.z &&
			this->normal.x == other.normal.x && this->normal.y == other.normal.y && this->normal.z == other.normal.z &&
			this->uv.x == other.uv.x && this->uv.y == other.uv.y)
		{
			result = true;
		}

		return result;
	}
};