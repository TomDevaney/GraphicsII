#pragma once
#include "Defines.h"

class Model
{
private:
	string filePath;

	const OBJ_VERT *modelData;
	unsigned int numVerts;

	vector<Vertex> realVertices;
	vector<unsigned int> bufferIndex;

	std::shared_ptr<DX::DeviceResources> m_deviceResources;

	Microsoft::WRL::ComPtr<ID3D11InputLayout>	m_inputLayout;
	Microsoft::WRL::ComPtr<ID3D11Buffer>		m_vertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer>		m_indexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer>		m_constantBuffer;
	Microsoft::WRL::ComPtr<ID3D11VertexShader>	m_vertexShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader>	m_pixelShader;
public:
	//~Model();
	void ReadFile();
	void SetFilePath(string path);
	void SetHeaderDetails(const OBJ_VERT *data, unsigned int numOfVerts);
	void ReadFromHeader();
	void CreateDeviceDependentResources(const std::shared_ptr<DX::DeviceResources>& deviceResources);
};