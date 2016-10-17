#pragma once
#include "Defines.h"
#include "DDSTextureLoader.h"
#include "..\Common\StepTimer.h"

class Model
{
private:
	string filePath;
	string texturePath;
	string normalPath;

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
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_normalShaderResourceView;;
	//Microsoft::WRL::ComPtr<ID3D11SamplerState> m_samplerState;

	uint32	m_indexCount;
	ModelViewProjectionConstantBuffer	m_constantBufferData; //used to make model 3d
	bool m_loadingComplete;

	//instance members
	bool isInstanced = false;
	unsigned int numOfInstances;
	unsigned int instanceWidth;
	Microsoft::WRL::ComPtr<ID3D11Buffer>		m_instanceBuffer;;

	//lighting members
	Microsoft::WRL::ComPtr<ID3D11Buffer>		m_lightConstantBuffer;
	LightingConstantBuffer m_lightConstantBufferData;
	float deltaLight = 1;
	float deltaPosition = 1;

	//normal members
	bool isNormalMap;

	//Skybox members
	bool isSkybox = false;
	Microsoft::WRL::ComPtr<ID3D11VertexShader>	m_skyBoxVertexShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader>	m_skyBoxPixelShader;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_skyBoxShaderResourceView;;

	//Geometry members
	bool isGeometry;
	Microsoft::WRL::ComPtr<ID3D11Buffer>		m_geometryVertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11GeometryShader> m_geometryShader;
	//vector<Vertex> geometryPoints;

	//Splitscreen members
	XMFLOAT4X4 bottomScreenView;
	XMFLOAT3 secondCamPosition;

	//Render to Texture members
	bool isSecondRTV;
	Microsoft::WRL::ComPtr<ID3D11SamplerState>	m_samplerState;

	//Transparency members
	Microsoft::WRL::ComPtr<ID3D11BlendState> transparency;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> CCWcullMode;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> CWcullMode;

	//Helper functions
	void CalculateTangentBinormal(Vertex v1, Vertex v2, Vertex v3, XMFLOAT3 &tangent, XMFLOAT3 &binormal); //helper function
	void CalculateNewNormal(XMFLOAT3 tangent, XMFLOAT3 binormal, XMFLOAT3 &normal);
public:
	//~Model();
	void ReadFile();
	//void SetHeaderDetails(const OBJ_VERT *data, unsigned int numOfVerts);
	//void ReadFromHeader();
	void CreateDeviceDependentResources(const std::shared_ptr<DX::DeviceResources>& deviceResources);
	void Render();
	void Translate(XMFLOAT3 distance);
	void Rotate(float radians);
	void TranslateRotate(XMFLOAT3 distance, float radians);
	void CalculateNewNormalsTangentsNormals();

	//Getters

	//Setters
	void SetFilePath(string path);
	void SetTexturePath(string path);
	void SetProjection(XMMATRIX projection);
	void SetView(XMMATRIX view);
	void SetSecondView(XMMATRIX view);
	void SetInstanceData(unsigned int numInstances, unsigned int widthOfInstance);
	void SetDirectionalLight(XMFLOAT4 directionalLightDirection, XMFLOAT4 directionalLightColor, XMFLOAT4 ambientRatio);
	void SetPointLight(XMFLOAT4 pointPosition, XMFLOAT4 pointLightColor, XMFLOAT4 lightRadius);
	void SetSpotLight(XMFLOAT4 spotPosition, XMFLOAT4 spotLightColor, XMFLOAT4 coneRatio, XMFLOAT4 coneDirection);
	void SetNormalPath(string path);
	void SetIsSkybox(bool toggle);
	void SetIdentityMatrix();
	void SetScaleMatrix(float x, float y, float z);
	void SetGeometryShader(vector<Vertex> points);
	void SetShaderResourceView(ID3D11ShaderResourceView* srv, bool isRenderToTexture);
	void SetVerticesAndIndices(Vertex* realVertices, unsigned int* bufferIndex, unsigned int verticesSize, unsigned int indicesSize);
	void SetIsSceneTexture(bool toggle);
	void SetIsTransparent(bool toggle);
	void SetCamPosition(XMFLOAT3 position);
	void SetSecondCamPosition(XMFLOAT3 position);

	//Updaters
	void UpdateLightRadius(DX::StepTimer const& timer, float min, float max);
	void UpdateSpotLight(XMFLOAT4X4 camera);
	void UpdatePointLightPosition(DX::StepTimer const& timer, float min, float max);
	void UpdateDirectionalLight(DX::StepTimer const& timer, bool left);
	vector<Model*> UpdateTransparentObjects(vector<Model*> transparentModels);

	//void SetShaderResourceView(Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv);
};