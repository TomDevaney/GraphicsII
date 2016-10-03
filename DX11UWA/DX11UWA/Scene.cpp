#include "pch.h"
//#include "Scene.h"
//
//void Scene::CreateDeviceDependentResources(const std::shared_ptr<DX::DeviceResources>& deviceResources)
//{
//	m_deviceResources = deviceResources;
//
//	// Load shaders asynchronously.
//	auto loadVSTask = DX::ReadDataAsync(L"MyFirstVertexShader.cso");
//	auto loadPSTask = DX::ReadDataAsync(L"MyFirstPixelShader.cso");
//
//	// After the vertex shader file is loaded, create the shader and input layout.
//	auto createVSTask = loadVSTask.then([this](const std::vector<byte>& fileData) {
//		DX::ThrowIfFailed(
//			m_deviceResources->GetD3DDevice()->CreateVertexShader(
//				&fileData[0],
//				fileData.size(),
//				nullptr,
//				&m_vertexShader
//			)
//		);
//
//		static const D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
//		{
//			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
//			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
//			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
//
//			{ "INSTANCEPOS", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
//		};
//
//		DX::ThrowIfFailed(
//			m_deviceResources->GetD3DDevice()->CreateInputLayout(
//				vertexDesc,
//				ARRAYSIZE(vertexDesc),
//				&fileData[0],
//				fileData.size(),
//				&m_inputLayout
//			)
//		);
//	});
//
//	//Create pixel shader
//	auto createPSTask = loadPSTask.then([this](const std::vector<byte>& fileData) {
//		DX::ThrowIfFailed(
//			m_deviceResources->GetD3DDevice()->CreatePixelShader(
//				&fileData[0],
//				fileData.size(),
//				nullptr,
//				&m_pixelShader
//			)
//		);
//
//		//Create constant buffer to matrices and isInstance
//		CD3D11_BUFFER_DESC constantBufferDesc(sizeof(ModelViewProjectionConstantBuffer), D3D11_BIND_CONSTANT_BUFFER);
//		DX::ThrowIfFailed(
//			m_deviceResources->GetD3DDevice()->CreateBuffer(
//				&constantBufferDesc,
//				nullptr,
//				&m_constantBuffer
//			)
//		);
//
//		//Create constant buffer for lighting
//		CD3D11_BUFFER_DESC lightConstantBufferDesc(sizeof(LightingConstantBuffer), D3D11_BIND_CONSTANT_BUFFER);
//		DX::ThrowIfFailed(
//			m_deviceResources->GetD3DDevice()->CreateBuffer(
//				&lightConstantBufferDesc,
//				nullptr,
//				&m_lightConstantBuffer
//			)
//		);
//	});
//
//	// Once both shaders are loaded, create the mesh.
//	auto createModelTask = (createPSTask && createVSTask).then([this]() {
//
//		for (int i = 0; i < models.size(); ++i)
//		{
//			Microsoft::WRL::ComPtr<ID3D11Buffer> m_vertexBuffer;
//
//			D3D11_SUBRESOURCE_DATA vertexBufferData = { 0 };
//			vertexBufferData.pSysMem = models[i].GetVertices().data();
//			vertexBufferData.SysMemPitch = 0;
//			vertexBufferData.SysMemSlicePitch = 0;
//			CD3D11_BUFFER_DESC vertexBufferDesc(sizeof(Vertex) * models[i].GetVertices().size(), D3D11_BIND_VERTEX_BUFFER);
//			DX::ThrowIfFailed(
//				m_deviceResources->GetD3DDevice()->CreateBuffer(
//					&vertexBufferDesc,
//					&vertexBufferData,
//					&m_vertexBuffer
//				)
//			);
//
//			m_vertexBuffers.push_back(m_vertexBuffer);
//
//			// Load mesh indices.
//
//			Microsoft::WRL::ComPtr<ID3D11Buffer> m_indexBuffer;
//
//			m_indexCount = models[i].GetBufferIndex().size();
//
//			D3D11_SUBRESOURCE_DATA indexBufferData = { 0 };
//			indexBufferData.pSysMem = models[i].GetBufferIndex().data();
//			indexBufferData.SysMemPitch = 0;
//			indexBufferData.SysMemSlicePitch = 0;
//			CD3D11_BUFFER_DESC indexBufferDesc(sizeof(unsigned int) * models[i].GetBufferIndex().size(), D3D11_BIND_INDEX_BUFFER);
//			DX::ThrowIfFailed(
//				m_deviceResources->GetD3DDevice()->CreateBuffer(
//					&indexBufferDesc,
//					&indexBufferData,
//					&m_indexBuffer
//				)
//			);
//
//			m_indexBuffers.push_back(m_indexBuffer);
//
//			if (models[i].IsInstanced())
//			{
//				//Get offsets of each instance
//				vector<InstanceData> instances(models[i].GetNumOfInstances());
//
//				for (int i = 0; i < instances.size(); ++i)
//				{
//					instances[i].pos.x = (i % 10) + models[i].GetInstanceWidth();
//					instances[i].pos.z = (i / 10) + models[i].GetInstanceWidth();
//				}
//
//				models[i].GetConstantBuffer()->isInstance.x = 1;
//
//
//				//Create instance buffer
//				D3D11_SUBRESOURCE_DATA instanceBufferData = { 0 };
//				instanceBufferData.pSysMem = instances.data();
//				instanceBufferData.SysMemPitch = 0;
//				instanceBufferData.SysMemSlicePitch = 0;
//				CD3D11_BUFFER_DESC instanceBufferDesc(sizeof(InstanceData) * instances.size(), D3D11_BIND_VERTEX_BUFFER);
//				DX::ThrowIfFailed(
//					m_deviceResources->GetD3DDevice()->CreateBuffer(
//						&instanceBufferDesc,
//						&instanceBufferData,
//						&*models[i].GetInstanceBuffer()
//					)
//				);
//			}
//			else
//			{
//				models[i].GetConstantBuffer()->isInstance.x = 0;
//
//			}
//		}
//	});
//
//
//	//Handle texture
//	for (int i = 0; i < models.size(); ++i)
//	{
//		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_shaderResourceView;
//		string texturePath = models[i].GetTexturePath();
//
//		wstring wideTexturePath = wstring(texturePath.begin(), texturePath.end());
//
//		HRESULT hr = CreateDDSTextureFromFile(m_deviceResources->GetD3DDevice(), wideTexturePath.c_str(), nullptr, &m_shaderResourceView);
//
//		m_shaderResourceViews.push_back(m_shaderResourceView);
//	}
//
//	createModelTask.then([this]() {
//		for (int i = 0; i < models.size(); ++i)
//		{
//			models[i].SetDoneLoading(true);
//		}
//	});
//}
//
//void Scene::Render()
//{
//	// Loading is asynchronous. Only draw geometry after it's loaded.
//	for (int i = 0; i < models.size(); ++i)
//	{
//		if (!models[i].IsDoneLoading())
//		{
//			return;
//		}
//
//		auto context = m_deviceResources->GetD3DDeviceContext();
//
//		// Prepare the constant buffer to send it to the graphics device.
//		context->UpdateSubresource1(
//			m_constantBuffer.Get(),
//			0,
//			NULL,
//			&*models[i].GetConstantBuffer(),
//			0,
//			0,
//			0
//		);
//
//		//Prepare light constant buffer
//		context->UpdateSubresource1(
//			m_lightConstantBuffer.Get(),
//			0,
//			NULL,
//			&m_lightConstantBufferData,
//			0,
//			0,
//			0
//		);
//
//		// Each vertex is one instance of the Vertex struct.
//		UINT stride = sizeof(Vertex);
//		UINT offset = 0;
//
//
//
//		if (models[i].IsInstanced())
//		{
//			UINT strides[2] = { sizeof(Vertex), sizeof(InstanceData) };
//			UINT offsets[2] = { 0, 0 };
//			ID3D11Buffer* instBuffer[2] = { *m_vertexBuffers[i].GetAddressOf(), *models[i].GetInstanceBuffer()->GetAddressOf() }; //I should probably release this somehow
//
//			context->IASetVertexBuffers(
//				0,
//				2,
//				instBuffer,
//				strides,
//				offsets
//			);
//		}
//		else
//		{
//			context->IASetVertexBuffers(
//				0,
//				1,
//				m_vertexBuffers[i].GetAddressOf(),
//				&stride,
//				&offset
//			);
//		}
//
//		context->IASetIndexBuffer(
//			m_indexBuffers[i].Get(),
//			DXGI_FORMAT_R32_UINT, // Each index is one 32-bit unsigned integer (uint).
//			0
//		);
//
//		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
//
//		context->IASetInputLayout(m_inputLayout.Get());
//
//		// Attach our vertex shader.
//		context->VSSetShader(
//			m_vertexShader.Get(),
//			nullptr,
//			0
//		);
//
//		// Send the constant buffer to the graphics device.
//		context->VSSetConstantBuffers1(
//			0,
//			1,
//			m_constantBuffer.GetAddressOf(),
//			nullptr,
//			nullptr
//		);
//
//		//Send light constant buffer to pixel shader
//		context->PSSetConstantBuffers1(
//			0,
//			1,
//			m_lightConstantBuffer.GetAddressOf(),
//			nullptr,
//			nullptr
//		);
//
//		// Attach our pixel shader.
//		context->PSSetShader(
//			m_pixelShader.Get(),
//			nullptr,
//			0
//		);
//
//		context->PSSetShaderResources(0, 1, m_shaderResourceViews[i].GetAddressOf());
//		//context->PSSetSamplers(0, 1, m_samplerState.GetAddressOf());
//
//		// Draw the objects.
//		if (models[i].IsInstanced())
//		{
//			context->DrawIndexedInstanced(m_indexCount, models[i].GetNumOfInstances(), 0, 0, 0);
//		}
//		else
//		{
//			context->DrawIndexed(
//				m_indexCount,
//				0,
//				0
//			);
//		}
//	}
//}
//
//void Scene::SetDirectionalLight(XMFLOAT4 directionalLightDirection, XMFLOAT4 directionalLightColor, XMFLOAT4 ambientRatio) // {directional, point, spot, 0}
//{
//	m_lightConstantBufferData.ambientLight = ambientRatio;
//	m_lightConstantBufferData.dirLightNorm = directionalLightDirection;
//	m_lightConstantBufferData.dirLightColor = directionalLightColor;
//	m_lightConstantBufferData.typeOfLight.x = 1.0f;
//}
//
//void Scene::SetPointLight(XMFLOAT4 pointPosition, XMFLOAT4 pointLightColor, XMFLOAT4 lightRadius)
//{
//	m_lightConstantBufferData.pointLightPosition = pointPosition;
//	m_lightConstantBufferData.pointLightColor = pointLightColor;
//	m_lightConstantBufferData.typeOfLight.y = 1.0f;
//	m_lightConstantBufferData.lightRadius = lightRadius;
//}
//
//void Scene::UpdateLightRadius(DX::StepTimer const& timer)
//{
//	const float delta_time = (float)timer.GetElapsedSeconds();
//
//	m_lightConstantBufferData.lightRadius.x += deltaLight * delta_time;
//
//	if (m_lightConstantBufferData.lightRadius.x >= 5)
//	{
//		m_lightConstantBufferData.lightRadius.x = 5;
//		deltaLight *= -1;
//	}
//	else if (m_lightConstantBufferData.lightRadius.x <= 1)
//	{
//		m_lightConstantBufferData.lightRadius.x = 1;
//		deltaLight *= -1;
//	}
//}
//
//void Scene::SetSpotLight(XMFLOAT4 spotPosition, XMFLOAT4 spotLightColor, XMFLOAT4 coneRatio, XMFLOAT4 coneDirection)
//{
//	m_lightConstantBufferData.spotLightPosition = spotPosition;
//	m_lightConstantBufferData.spotLightColor = spotLightColor;
//	m_lightConstantBufferData.coneRatio = coneRatio;
//	m_lightConstantBufferData.typeOfLight.z = 1;
//	m_lightConstantBufferData.coneDirection = coneDirection;
//}
//
//void Scene::UpdateSpotLight(XMFLOAT4X4 camera, XMVECTOR camTarget)
//{
//	m_lightConstantBufferData.spotLightPosition.x = camera._41;
//	m_lightConstantBufferData.spotLightPosition.y = camera._42 + 0.699999928;
//	m_lightConstantBufferData.spotLightPosition.z = camera._43 - 1.5f;
//
//	m_lightConstantBufferData.coneDirection.x = XMVectorGetX(camTarget) - m_lightConstantBufferData.spotLightPosition.x;
//	m_lightConstantBufferData.coneDirection.y = XMVectorGetY(camTarget) - m_lightConstantBufferData.spotLightPosition.y;
//	m_lightConstantBufferData.coneDirection.z = XMVectorGetZ(camTarget) - m_lightConstantBufferData.spotLightPosition.z;
//}
//
//void Scene::AddModel(Model model)
//{
//	models.push_back(model);
//}