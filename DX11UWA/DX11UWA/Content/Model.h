#pragma once
#include "Defines.h"
#include "DDSTextureLoader.h"

class Model
{
private:
	string filePath;
	string texturePath;

	//const OBJ_VERT *modelData;
	//unsigned int numVerts;

	vector<Vertex> realVertices;
	vector<unsigned int> bufferIndex;

	std::shared_ptr<DX::DeviceResources> m_deviceResources;

	Microsoft::WRL::ComPtr<ID3D11InputLayout>	m_inputLayout;
	Microsoft::WRL::ComPtr<ID3D11Buffer>		m_vertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer>		m_indexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer>		m_constantBuffer;
	Microsoft::WRL::ComPtr<ID3D11VertexShader>	m_vertexShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader>	m_pixelShader;

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_shaderResourceView;
	//Microsoft::WRL::ComPtr<ID3D11SamplerState> m_samplerState;

	uint32	m_indexCount;
	ModelViewProjectionConstantBuffer	m_constantBufferData; //used to make model 3d
	bool m_loadingComplete;

	//instance members
	bool isInstanced = false;
	unsigned int numOfInstances;
	unsigned int instanceWidth;
	Microsoft::WRL::ComPtr<ID3D11Buffer>		m_instanceBuffer;;


public:
	//~Model();
	void ReadFile();
	void SetFilePath(string path);
	void SetTexturePath(string path);
	//void SetHeaderDetails(const OBJ_VERT *data, unsigned int numOfVerts);
	//void ReadFromHeader();
	void CreateDeviceDependentResources(const std::shared_ptr<DX::DeviceResources>& deviceResources);
	void Render();
	void Translate(XMFLOAT3 distance);
	void SetProjection(XMMATRIX projection);
	void SetView(XMMATRIX view);
	void SetInstanceData(unsigned int numInstances, unsigned int widthOfInstance);
};