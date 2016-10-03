//#pragma once
//#include "Content/Defines.h"
//#include "Content/Model.h"
//
//class Scene
//{
//private:
//	std::shared_ptr<DX::DeviceResources> m_deviceResources;
//
//	Microsoft::WRL::ComPtr<ID3D11InputLayout>	m_inputLayout;
//	vector<Microsoft::WRL::ComPtr<ID3D11Buffer>>		m_vertexBuffers;
//	vector<Microsoft::WRL::ComPtr<ID3D11Buffer>>		m_indexBuffers;
//	Microsoft::WRL::ComPtr<ID3D11Buffer>		m_constantBuffer;
//	Microsoft::WRL::ComPtr<ID3D11VertexShader>	m_vertexShader;
//	Microsoft::WRL::ComPtr<ID3D11PixelShader>	m_pixelShader;
//
//	vector<Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>> m_shaderResourceViews;
//
//	uint32	m_indexCount;
//
//	//lighting members
//	Microsoft::WRL::ComPtr<ID3D11Buffer>		m_lightConstantBuffer;
//	LightingConstantBuffer m_lightConstantBufferData;
//	float deltaLight = 1;
//
//	//models vector
//	vector<Model> models;
//public:
//	void CreateDeviceDependentResources(const std::shared_ptr<DX::DeviceResources>& deviceResources);
//	void Render();
//	void SetDirectionalLight(XMFLOAT4 directionalLightDirection, XMFLOAT4 directionalLightColor, XMFLOAT4 ambientRatio);
//	void SetPointLight(XMFLOAT4 pointPosition, XMFLOAT4 pointLightColor, XMFLOAT4 lightRadius);
//	void SetSpotLight(XMFLOAT4 spotPosition, XMFLOAT4 spotLightColor, XMFLOAT4 coneRatio, XMFLOAT4 coneDirection);
//	void UpdateLightRadius(DX::StepTimer const& timer);
//	void UpdateSpotLight(XMFLOAT4X4 camera, XMVECTOR camTarget);
//	void AddModel(Model model);
//};