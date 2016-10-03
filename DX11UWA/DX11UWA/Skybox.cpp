#include "pch.h"
#include "Skybox.h"

void Skybox::SetFilePath(string path)
{
	filePath = path;
}

void Skybox::LoadSkybox(const std::shared_ptr<DX::DeviceResources>& deviceResources)
{
	m_deviceResources = deviceResources;

	wstring wideTexturePath = wstring(filePath.begin(), filePath.end());
	HRESULT hr = CreateDDSTextureFromFile(m_deviceResources->GetD3DDevice(), wideTexturePath.c_str(), nullptr, &shaderResourceView);
}

void Skybox::Render()
{

}