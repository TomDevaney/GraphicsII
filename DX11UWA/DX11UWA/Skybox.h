#pragma once
#include "../Content/Defines.h"
#include "../Content/DDSTextureLoader.h"

class Skybox
{
private:
	string filePath;
	Microsoft::WRL::ComPtr<ID3D11Texture2D> skyboxTexture;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> shaderResourceView;
	std::shared_ptr<DX::DeviceResources> m_deviceResources;

public:
	void SetFilePath(string path);
	void LoadSkybox(const std::shared_ptr<DX::DeviceResources>& deviceResources);
	void Render();
};