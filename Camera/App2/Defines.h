#pragma once
#include <string>
#include <fstream>
#include <d3d11.h>
#include <DirectXMath.h>
#include <iostream>
#include "test pyramid.h"
#include "..\Common\DeviceResources.h"
#include <vector>
#include "..\Common\DirectXHelper.h"
#include "Content/ShaderStructures.h"

using namespace DirectX;

#pragma comment(lib, "d3d11.lib")

using namespace std;

struct Vertex
{
	XMFLOAT3 position;
	XMFLOAT2 uv;
	XMFLOAT3 normal;
};