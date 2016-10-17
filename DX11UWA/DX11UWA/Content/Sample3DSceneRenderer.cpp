#include "pch.h"
#include "Sample3DSceneRenderer.h"

#include "..\Common\DirectXHelper.h"

using namespace DX11UWA;

using namespace DirectX;
using namespace Windows::Foundation;

// Loads vertex and pixel shaders from files and instantiates the cube geometry.
Sample3DSceneRenderer::Sample3DSceneRenderer(const std::shared_ptr<DX::DeviceResources>& deviceResources) :
	m_loadingComplete(false),
	m_degreesPerSecond(45),
	m_indexCount(0),
	m_tracking(false),
	m_deviceResources(deviceResources)
{
	memset(m_kbuttons, 0, sizeof(m_kbuttons));
	m_currMousePos = nullptr;
	m_prevMousePos = nullptr;
	memset(&m_camera, 0, sizeof(XMFLOAT4X4));
	memset(&m_bottomCamera, 0, sizeof(XMFLOAT4X4));

	CreateDeviceDependentResources();
	CreateWindowSizeDependentResources();
}

// Initializes view parameters when the window size changes.
void Sample3DSceneRenderer::CreateWindowSizeDependentResources(void)
{
	Size outputSize = m_deviceResources->GetOutputSize();
	float aspectRatio = outputSize.Width / (outputSize.Height / 2); //I added divide by two to account for splitscreen
	float fovAngleY = 70.0f * XM_PI / 180.0f;

	// This is a simple example of change that can be made when the app is in
	// portrait or snapped view.
	if (aspectRatio < 1.0f)
	{
		fovAngleY *= 2.0f;
	}

	// Note that the OrientationTransform3D matrix is post-multiplied here
	// in order to correctly orient the scene to match the display orientation.
	// This post-multiplication step is required for any draw calls that are
	// made to the swap chain render target. For draw calls to other targets,
	// this transform should not be applied.

	// This sample makes use of a right-handed coordinate system using row-major matrices.
	XMMATRIX perspectiveMatrix = XMMatrixPerspectiveFovLH(fovAngleY, aspectRatio, 0.01f, 100.0f);

	XMFLOAT4X4 orientation = m_deviceResources->GetOrientationTransform3D();

	XMMATRIX orientationMatrix = XMLoadFloat4x4(&orientation);

	XMStoreFloat4x4(&m_constantBufferData.projection, XMMatrixTranspose(perspectiveMatrix * orientationMatrix));

	//Set models projection
	skyBox.SetProjection(XMMatrixTranspose(perspectiveMatrix * orientationMatrix));
	for (int i = 0; i < models.size(); ++i)
	{
		models[i]->SetProjection(XMMatrixTranspose(perspectiveMatrix * orientationMatrix));
	}

	for (int i = 0; i < transparentModels.size(); ++i)
	{
		transparentModels[i]->SetProjection(XMMatrixTranspose(perspectiveMatrix * orientationMatrix));
	}

	// Eye is at (0,0.7,1.5), looking at point (0,-0.1,0) with the up-vector along the y-axis.
	static const XMVECTORF32 eye = { 0.0f, 0.7f, -1.5f, 0.0f };
	static const XMVECTORF32 at = { 0.0f, -0.1f, 0.0f, 0.0f };
	static const XMVECTORF32 up = { 0.0f, 1.0f, 0.0f, 0.0f };

	XMStoreFloat4x4(&m_camera, XMMatrixInverse(nullptr, XMMatrixLookAtLH(eye, at, up)));
	XMStoreFloat4x4(&m_constantBufferData.view, XMMatrixTranspose(XMMatrixLookAtLH(eye, at, up)));

	static const XMVECTORF32 eye2 = { 0.0f, 2.0f, 10.0f, 0.0f };
	static const XMVECTORF32 at2 = { 0.0f, 0.7f, -1.5f, 0.0f };
	static const XMVECTORF32 up2 = { 0.0f, 1.0f, 0.0f, 0.0f };

	XMStoreFloat4x4(&m_bottomCamera, XMMatrixInverse(nullptr, XMMatrixLookAtLH(eye2, at2, up2)));

	camTarget = XMVectorSet(0, 0, 0, 0);
}

// Called once per frame, rotates the cube and calculates the model and view matrices.
void Sample3DSceneRenderer::Update(DX::StepTimer const& timer)
{
	if (!m_tracking)
	{
		// Convert degrees to radians, then convert seconds to rotation angle
		float radiansPerSecond = XMConvertToRadians(m_degreesPerSecond);
		double totalRotation = timer.GetTotalSeconds() * radiansPerSecond;
		float radians = static_cast<float>(fmod(totalRotation, XM_2PI));

		Rotate(radians);
		for (int i = 0; i < models.size(); ++i)
		{
			//models[i]->UpdateLightRadius(timer, 1, 5);
			models[i]->UpdatePointLightPosition(timer, 0, 10);
		}
	}

	// Update or move camera here
	UpdateCamera(timer, 1.0f, 0.75f);

	//Set spotlight to camera position
	skyBox.UpdateSpotLight(m_camera);
	//skyBox.Translate({ m_camera._41, m_camera._42, m_camera._43 }); //breaks spotlight on skyBox
	
	for (int i = 0; i < models.size(); ++i)
	{
		models[i]->UpdateSpotLight(m_camera);
	}

	//move scene texture to camera position, so I don't see anything else
	scene.Translate({ m_camera._41, m_camera._42, m_camera._43 + 0.1f });


	skyBox.SetSecondCamPosition({ m_bottomCamera._41, m_bottomCamera._42, m_bottomCamera._43 });

	for (int i = 0; i < models.size(); ++i)
	{
		models[i]->SetCamPosition({ m_camera._41, m_camera._42, m_camera._43 });
		models[i]->SetSecondCamPosition({ m_bottomCamera._41, m_bottomCamera._42, m_bottomCamera._43 });
	}

	//handle transparency stuff
	for (int i = 0; i < transparentModels.size(); ++i)
	{
		transparentModels[i]->SetCamPosition({ m_camera._41, m_camera._42, m_camera._43 });
		transparentModels[i]->SetSecondCamPosition({ m_bottomCamera._41, m_bottomCamera._42, m_bottomCamera._43 });
	}

	//transparentModels = transparentModels[0]->UpdateTransparentObjects(transparentModels);

}

// Rotate the 3D cube model a set amount of radians.
void Sample3DSceneRenderer::Rotate(float radians)
{
	// Prepare to pass the updated model matrix to the shader
	XMStoreFloat4x4(&m_constantBufferData.model, XMMatrixTranspose(XMMatrixRotationY(radians)));
	//goomba.Rotate(radians);
}

void Sample3DSceneRenderer::UpdateCamera(DX::StepTimer const& timer, float const moveSpd, float const rotSpd)
{
	const float delta_time = (float)timer.GetElapsedSeconds();

	if (m_kbuttons['W'])
	{
		XMMATRIX translation = XMMatrixTranslation(0.0f, 0.0f, moveSpd * delta_time);
		XMMATRIX temp_camera = XMLoadFloat4x4(&m_camera);
		XMMATRIX result = XMMatrixMultiply(translation, temp_camera);
		XMStoreFloat4x4(&m_camera, result);
	}
	if (m_kbuttons['S'])
	{
		XMMATRIX translation = XMMatrixTranslation(0.0f, 0.0f, -moveSpd * delta_time);
		XMMATRIX temp_camera = XMLoadFloat4x4(&m_camera);
		XMMATRIX result = XMMatrixMultiply(translation, temp_camera);
		XMStoreFloat4x4(&m_camera, result);
	}
	if (m_kbuttons['A'])
	{
		XMMATRIX translation = XMMatrixTranslation(-moveSpd * delta_time, 0.0f, 0.0f);
		XMMATRIX temp_camera = XMLoadFloat4x4(&m_camera);
		XMMATRIX result = XMMatrixMultiply(translation, temp_camera);
		XMStoreFloat4x4(&m_camera, result);
	}
	if (m_kbuttons['D'])
	{
		XMMATRIX translation = XMMatrixTranslation(moveSpd * delta_time, 0.0f, 0.0f);
		XMMATRIX temp_camera = XMLoadFloat4x4(&m_camera);
		XMMATRIX result = XMMatrixMultiply(translation, temp_camera);
		XMStoreFloat4x4(&m_camera, result);
	}
	if (m_kbuttons['X'])
	{
		XMMATRIX translation = XMMatrixTranslation( 0.0f, -moveSpd * delta_time, 0.0f);
		XMMATRIX temp_camera = XMLoadFloat4x4(&m_camera);
		XMMATRIX result = XMMatrixMultiply(translation, temp_camera);
		XMStoreFloat4x4(&m_camera, result);
	}
	if (m_kbuttons[VK_SPACE])
	{
		XMMATRIX translation = XMMatrixTranslation( 0.0f, moveSpd * delta_time, 0.0f);
		XMMATRIX temp_camera = XMLoadFloat4x4(&m_camera);
		XMMATRIX result = XMMatrixMultiply(translation, temp_camera);
		XMStoreFloat4x4(&m_camera, result);
	}
	if (m_kbuttons[VK_LEFT])
	{
		for (int i = 0; i < models.size(); ++i)
		{
			models[i]->UpdateDirectionalLight(timer, true);
		}
	}
	if (m_kbuttons[VK_RIGHT])
	{
		for (int i = 0; i < models.size(); ++i)
		{
			models[i]->UpdateDirectionalLight(timer, false);
		}
	}

	//handle bottom screen input
	if (m_kbuttons['I'])
	{
		XMMATRIX translation = XMMatrixTranslation(0.0f, 0.0f, moveSpd * delta_time);
		XMMATRIX temp_camera = XMLoadFloat4x4(&m_bottomCamera);
		XMMATRIX result = XMMatrixMultiply(translation, temp_camera);
		XMStoreFloat4x4(&m_bottomCamera, result);
	}
	if (m_kbuttons['K'])
	{
		XMMATRIX translation = XMMatrixTranslation(0.0f, 0.0f, -moveSpd * delta_time);
		XMMATRIX temp_camera = XMLoadFloat4x4(&m_bottomCamera);
		XMMATRIX result = XMMatrixMultiply(translation, temp_camera);
		XMStoreFloat4x4(&m_bottomCamera, result);
	}
	if (m_kbuttons['J'])
	{
		XMMATRIX translation = XMMatrixTranslation(-moveSpd * delta_time, 0.0f, 0.0f);
		XMMATRIX temp_camera = XMLoadFloat4x4(&m_bottomCamera);
		XMMATRIX result = XMMatrixMultiply(translation, temp_camera);
		XMStoreFloat4x4(&m_bottomCamera, result);
	}
	if (m_kbuttons['L'])
	{
		XMMATRIX translation = XMMatrixTranslation(moveSpd * delta_time, 0.0f, 0.0f);
		XMMATRIX temp_camera = XMLoadFloat4x4(&m_bottomCamera);
		XMMATRIX result = XMMatrixMultiply(translation, temp_camera);
		XMStoreFloat4x4(&m_bottomCamera, result);
	}


	if (m_currMousePos) 
	{
		if (m_currMousePos->Properties->IsRightButtonPressed && m_prevMousePos)
		{
			float dx = m_currMousePos->Position.X - m_prevMousePos->Position.X;
			float dy = m_currMousePos->Position.Y - m_prevMousePos->Position.Y;

			XMFLOAT4 pos = XMFLOAT4(m_camera._41, m_camera._42, m_camera._43, m_camera._44);

			m_camera._41 = 0;
			m_camera._42 = 0;
			m_camera._43 = 0;

			XMMATRIX rotX = XMMatrixRotationX(dy * rotSpd * delta_time);
			XMMATRIX rotY = XMMatrixRotationY(dx * rotSpd * delta_time);

			//my stuff
			camYaw += dx * delta_time;
			camPitch += dy * delta_time;

			XMMATRIX temp_camera = XMLoadFloat4x4(&m_camera);
			temp_camera = XMMatrixMultiply(rotX, temp_camera);
			temp_camera = XMMatrixMultiply(temp_camera, rotY);

			XMStoreFloat4x4(&m_camera, temp_camera);

			m_camera._41 = pos.x;
			m_camera._42 = pos.y;
			m_camera._43 = pos.z;
		}
		m_prevMousePos = m_currMousePos;
	}


}

void Sample3DSceneRenderer::SetKeyboardButtons(const char* list)
{
	memcpy_s(m_kbuttons, sizeof(m_kbuttons), list, sizeof(m_kbuttons));
}

void Sample3DSceneRenderer::SetMousePosition(const Windows::UI::Input::PointerPoint^ pos)
{
	m_currMousePos = const_cast<Windows::UI::Input::PointerPoint^>(pos);
}

void Sample3DSceneRenderer::SetInputDeviceData(const char* kb, const Windows::UI::Input::PointerPoint^ pos)
{
	SetKeyboardButtons(kb);
	SetMousePosition(pos);
}

void DX11UWA::Sample3DSceneRenderer::StartTracking(void)
{
	m_tracking = true;
}

// When tracking, the 3D cube can be rotated around its Y axis by tracking pointer position relative to the output screen width.
void Sample3DSceneRenderer::TrackingUpdate(float positionX)
{
	if (m_tracking)
	{
		float radians = XM_2PI * 2.0f * positionX / m_deviceResources->GetOutputSize().Width;
		Rotate(radians);
	}
}

void Sample3DSceneRenderer::StopTracking(void)
{
	m_tracking = false;
}

// Renders one frame using the vertex and pixel shaders.
void Sample3DSceneRenderer::Render(void)
{
	// Loading is asynchronous. Only draw geometry after it's loaded.
	if (!m_loadingComplete)
	{
		return;
	}

	auto context = m_deviceResources->GetD3DDeviceContext();

	XMStoreFloat4x4(&m_constantBufferData.view, XMMatrixTranspose(XMMatrixInverse(nullptr, XMLoadFloat4x4(&m_camera))));

	//Set Models View
	skyBox.SetView(XMMatrixTranspose(XMMatrixInverse(nullptr, XMLoadFloat4x4(&m_camera))));
	skyBox.SetSecondView(XMMatrixTranspose(XMMatrixInverse(nullptr, XMLoadFloat4x4(&m_bottomCamera))));

	for (int i = 0; i < models.size(); ++i)
	{
		models[i]->SetView(XMMatrixTranspose(XMMatrixInverse(nullptr, XMLoadFloat4x4(&m_camera))));
		models[i]->SetSecondView(XMMatrixTranspose(XMMatrixInverse(nullptr, XMLoadFloat4x4(&m_bottomCamera))));
	}

	for (int i = 0; i < transparentModels.size(); ++i)
	{
		transparentModels[i]->SetView(XMMatrixTranspose(XMMatrixInverse(nullptr, XMLoadFloat4x4(&m_camera))));
		transparentModels[i]->SetSecondView(XMMatrixTranspose(XMMatrixInverse(nullptr, XMLoadFloat4x4(&m_bottomCamera))));
	}

	// Prepare the constant buffer to send it to the graphics device.
	context->UpdateSubresource1(m_constantBuffer.Get(), 0, NULL, &m_constantBufferData, 0, 0, 0);
	// Each vertex is one instance of the VertexPositionColor struct.
	UINT stride = sizeof(VertexPositionColor);
	UINT offset = 0;
	context->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &stride, &offset);
	// Each index is one 16-bit unsigned integer (short).
	context->IASetIndexBuffer(m_indexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	context->IASetInputLayout(m_inputLayout.Get());
	// Attach our vertex shader.
	context->VSSetShader(m_vertexShader.Get(), nullptr, 0);
	// Send the constant buffer to the graphics device.
	context->VSSetConstantBuffers1(0, 1, m_constantBuffer.GetAddressOf(), nullptr, nullptr);
	// Attach our pixel shader.
	context->PSSetShader(m_pixelShader.Get(), nullptr, 0);
	// Draw the objects.
	context->DrawIndexed(m_indexCount, 0, 0);

	//handle skybox
	skyBox.Render();
	m_deviceResources->GetD3DDeviceContext()->ClearDepthStencilView(m_deviceResources->GetDepthStencilView(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	//Draw all of my models
	for (int i = 0; i < models.size(); ++i)
	{
		models[i]->Render();
	}

	for (int i = 0; i < transparentModels.size(); ++i)
	{
		transparentModels[i]->Render();
	}
}

void Sample3DSceneRenderer::CreateDeviceDependentResources(void)
{
	// Load shaders asynchronously.
	auto loadVSTask = DX::ReadDataAsync(L"SampleVertexShader.cso");
	auto loadPSTask = DX::ReadDataAsync(L"SamplePixelShader.cso");

	// After the vertex shader file is loaded, create the shader and input layout.
	auto createVSTask = loadVSTask.then([this](const std::vector<byte>& fileData)
	{
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateVertexShader(&fileData[0], fileData.size(), nullptr, &m_vertexShader));

		static const D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "UV", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};

		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateInputLayout(vertexDesc, ARRAYSIZE(vertexDesc), &fileData[0], fileData.size(), &m_inputLayout));
	});

	// After the pixel shader file is loaded, create the shader and constant buffer.
	auto createPSTask = loadPSTask.then([this](const std::vector<byte>& fileData)
	{
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreatePixelShader(&fileData[0], fileData.size(), nullptr, &m_pixelShader));

		CD3D11_BUFFER_DESC constantBufferDesc(sizeof(ModelViewProjectionConstantBuffer), D3D11_BIND_CONSTANT_BUFFER);
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&constantBufferDesc, nullptr, &m_constantBuffer));
	});

	// Once both shaders are loaded, create the mesh.
	auto createCubeTask = (createPSTask && createVSTask).then([this]()
	{
		// Load mesh vertices. Each vertex has a position and a color.
		static const VertexPositionColor cubeVertices[] =
		{
			{XMFLOAT3(-0.5f, -0.5f, -0.5f), XMFLOAT3(0.0f, 0.0f, 0.0f)},
			{XMFLOAT3(-0.5f, -0.5f,  0.5f), XMFLOAT3(0.0f, 0.0f, 1.0f)},
			{XMFLOAT3(-0.5f,  0.5f, -0.5f), XMFLOAT3(0.0f, 1.0f, 0.0f)},
			{XMFLOAT3(-0.5f,  0.5f,  0.5f), XMFLOAT3(0.0f, 1.0f, 1.0f)},
			{XMFLOAT3( 0.5f, -0.5f, -0.5f), XMFLOAT3(1.0f, 0.0f, 0.0f)},
			{XMFLOAT3( 0.5f, -0.5f,  0.5f), XMFLOAT3(1.0f, 0.0f, 1.0f)},
			{XMFLOAT3( 0.5f,  0.5f, -0.5f), XMFLOAT3(1.0f, 1.0f, 0.0f)},
			{XMFLOAT3( 0.5f,  0.5f,  0.5f), XMFLOAT3(1.0f, 1.0f, 1.0f)},
		};

		D3D11_SUBRESOURCE_DATA vertexBufferData = { 0 };
		vertexBufferData.pSysMem = cubeVertices;
		vertexBufferData.SysMemPitch = 0;
		vertexBufferData.SysMemSlicePitch = 0;
		CD3D11_BUFFER_DESC vertexBufferDesc(sizeof(cubeVertices), D3D11_BIND_VERTEX_BUFFER);
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&vertexBufferDesc, &vertexBufferData, &m_vertexBuffer));

		// Load mesh indices. Each trio of indices represents
		// a triangle to be rendered on the screen.
		// For example: 0,2,1 means that the vertices with indexes
		// 0, 2 and 1 from the vertex buffer compose the 
		// first triangle of this mesh.
		static const unsigned short cubeIndices[] =
		{
			0,1,2, // -x
			1,3,2,

			4,6,5, // +x
			5,6,7,

			0,5,1, // -y
			0,4,5,

			2,7,6, // +y
			2,3,7,

			0,6,4, // -z
			0,2,6,

			1,7,3, // +z
			1,5,7,
		};

		m_indexCount = ARRAYSIZE(cubeIndices);

		D3D11_SUBRESOURCE_DATA indexBufferData = { 0 };
		indexBufferData.pSysMem = cubeIndices;
		indexBufferData.SysMemPitch = 0;
		indexBufferData.SysMemSlicePitch = 0;
		CD3D11_BUFFER_DESC indexBufferDesc(sizeof(cubeIndices), D3D11_BIND_INDEX_BUFFER);
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&indexBufferDesc, &indexBufferData, &m_indexBuffer));
	});

	// Once the cube is loaded, the object is ready to be rendered.
	createCubeTask.then([this]()
	{
		m_loadingComplete = true;
	});

	//Load in models and Create device dependent resources

	//handle pyramid
	pyramid.SetFilePath("Assets/test pyramid.obj");
	pyramid.SetTexturePath("Assets/cartoonWood_seamless.dds");
	pyramid.SetDirectionalLight({ 0.577f, 0.577f, -0.577f, 0 }, { 0.75f, 0.75f, 0.94f, 1.0f }, { 0.3f, 0.3f, 0.3f, 0.3f });
	pyramid.SetPointLight({ 2, 3.0f, 2, 0 }, { 1, 0, 0, 0 }, { 5, 0, 0, 0 });
	pyramid.SetSpotLight({ 0, 0, 0, 0 }, { 1.0f, 1.0f, 1.0f, 0 }, { 0.9f, 0, 0, 0 }, { 0, 0, 1, 0 });
	pyramid.ReadFile();
	pyramid.CreateDeviceDependentResources(m_deviceResources);
	pyramid.Translate({ 1.0f, 1.0f, 1.0f });
	//pyramid.CalculateNewNormalsTangentsNormals();

	models.push_back(&pyramid);

	//handle goombas
	goomba.SetFilePath("Assets/Fuzzy Goomba.obj");
	goomba.SetTexturePath("Assets/Diffuse_Fuzzy_Corrupt.dds");
	goomba.SetNormalPath("Assets/Normal_Fuzzy.dds");
	goomba.SetInstanceData(500, 1);
	goomba.SetDirectionalLight({ 0.577f, 0.577f, -0.577f, 0 }, { 0.75f, 0.75f, 0.94f, 1.0f }, { 0.3f, 0.3f, 0.3f, 0.3f });
	goomba.SetPointLight({ 2, 3.0f, 2, 0 }, { 1, 0, 0, 0 }, { 5, 0, 0, 0 });
//	goomba.SetSpotLight({ 5, 6, 5, 0 }, { 1.0f, 1.0f, 1.0f, 0 }, { 0.9f, 0, 0, 0 }, { 0, -1, 0, 0 });
	goomba.SetSpotLight({ 0, 0, 0, 0 }, { 1.0f, 1.0f, 1.0f, 0 }, { 0.9f, 0, 0, 0 }, { 0, 0, 1, 0 });
	goomba.ReadFile();
	goomba.CalculateNewNormalsTangentsNormals(); //breaks my spotlight (and potentially other lights) (for spotlight, it flipped my normals)
	goomba.CreateDeviceDependentResources(m_deviceResources);
	goomba.Translate({ 1, 0, 1 });

	models.push_back(&goomba);

	//handle platform
	platform.SetFilePath("Assets/Plane.obj");
	platform.SetTexturePath("Assets/cartoonWood_seamless.dds");
	platform.SetNormalPath("Assets/normal_cartoonWood.dds");
	platform.SetDirectionalLight({ 0.577f, 0.577f, -0.577f, 0 }, { 0.75f, 0.75f, 0.94f, 1.0f }, { 0.3f, 0.3f, 0.3f, 0.3f });
	platform.SetPointLight({ 2, 3.0f, 2, 0 }, { 1, 0, 0, 0 }, { 5, 0, 0, 0 });
	//platform.SetSpotLight({ 5, 6, 5, 0 }, { 1.0f, 1.0f, 1.0f, 0 }, { 0.9f, 0, 0, 0 }, { 0, -1, 0, 0 });
	platform.SetSpotLight({ 0, 0, 0, 0 }, { 1.0f, 1.0f, 1.0f, 0 }, { 0.9f, 0, 0, 0 }, { 0, 0, 1, 0 });
	platform.ReadFile();
	platform.CreateDeviceDependentResources(m_deviceResources);
	platform.CalculateNewNormalsTangentsNormals(); //breaks my spotlight (and potentially other lights) (for spotlight, it flipped my normals)
	platform.Translate({ 1, 0, 1 });
	platform.SetScaleMatrix(10, 1, 15);

	models.push_back(&platform);

	willowTree.SetFilePath("Assets/WillowTree.obj");
	willowTree.SetTexturePath("Assets/treeWillow_Trunk_D.dds");
	//willowTree.SetNormalPath("Assets/normal_cartoonWood.dds");
	willowTree.SetDirectionalLight({ 0.577f, 0.577f, -0.577f, 0 }, { 0.75f, 0.75f, 0.94f, 1.0f }, { 0.3f, 0.3f, 0.3f, 0.3f });
	willowTree.SetPointLight({ 2, 3.0f, 2, 0 }, { 1, 0, 0, 0 }, { 5, 0, 0, 0 });
	willowTree.SetSpotLight({ 0, 0, 0, 0 }, { 1.0f, 1.0f, 1.0f, 0 }, { 0.9f, 0, 0, 0 }, { 0, 0, 1, 0 });
	willowTree.ReadFile();
	willowTree.CreateDeviceDependentResources(m_deviceResources);
	//willowTree.CalculateNewNormalsTangentsNormals(); //breaks my spotlight (and potentially other lights) (for spotlight, it flipped my normals)
	willowTree.Translate({ -2, 0, -2 });
	willowTree.SetScaleMatrix(0.15f, 0.15f, 0.15f);

	models.push_back(&willowTree);

	//handle grass
	vector<Vertex> points;

	Vertex point = { XMFLOAT3{-5.0f, 1.6f, 2.0f}, XMFLOAT2{}, XMFLOAT3{0, 0, -1 } };
	points.push_back(point);

	grass.SetTexturePath("Assets/grass_blade.dds");
	grass.SetGeometryShader(points);
	grass.SetDirectionalLight({ 0.577f, 0.577f, -0.577f, 0 }, { 0.75f, 0.75f, 0.94f, 1.0f }, { 1, 1, 1, 1 });
	grass.SetPointLight({ 1, 4.0f, 1, 0 }, { 1, 0, 0, 0 }, { 5, 0, 0, 0 });
	grass.SetSpotLight({ 0, 0, 0, 0 }, { 1.0f, 1.0f, 1.0f, 0 }, { 0.9f, 0, 0, 0 }, { 0, 0, 1, 0 });
	grass.CreateDeviceDependentResources(m_deviceResources);
	grass.SetIdentityMatrix();

	models.push_back(&grass);

	//handle ground
	ground.SetFilePath("Assets/Plane.obj");
	ground.SetTexturePath("Assets/brownishDirt_seamless.dds");
	ground.SetDirectionalLight({ 0.577f, 0.577f, -0.577f, 0 }, { 0.75f, 0.75f, 0.94f, 1.0f }, { 0.3f, 0.3f, 0.3f, 0.3f });
	ground.SetPointLight({ 2, 3.0f, 2, 0 }, { 1, 0, 0, 0 }, { 5, 0, 0, 0 });
	//ground.SetSpotLight({ 5, 6, 5, 0 }, { 1.0f, 1.0f, 1.0f, 0 }, { 0.9f, 0, 0, 0 }, { 0, -1, 0, 0 });
	ground.SetSpotLight({ 0, 0, 0, 0 }, { 1.0f, 1.0f, 1.0f, 0 }, { 0.9f, 0, 0, 0 }, { 0, 0, 1, 0 });
	ground.ReadFile();
	ground.CreateDeviceDependentResources(m_deviceResources);
	ground.Translate({ -5.0f, 1.5f, 2.0f });
	ground.SetScaleMatrix(1, 1, 1);

	models.push_back(&ground);

	skyBox.SetFilePath("Assets/SkyBox.obj");
	skyBox.SetTexturePath("Assets/hotelCubeMap1024.dds");
	skyBox.SetIsSkybox(true);
	skyBox.ReadFile();
	skyBox.CreateDeviceDependentResources(m_deviceResources);
	skyBox.SetIdentityMatrix();
	skyBox.SetScaleMatrix(50, 50, 50);


	tv.SetFilePath("Assets/TV.obj");
	tv.SetTexturePath("Assets/TV_D.dds");
	tv.SetDirectionalLight({ 0.577f, 0.577f, -0.577f, 0 }, { 0.75f, 0.75f, 0.94f, 1.0f }, { 0.3f, 0.3f, 0.3f, 0.3f });
	tv.SetPointLight({ 2, 3.0f, 2, 0 }, { 1, 0, 0, 0 }, { 5, 0, 0, 0 });
	tv.SetSpotLight({ 0, 0, 0, 0 }, { 1.0f, 1.0f, 1.0f, 0 }, { 0.9f, 0, 0, 0 }, { 0, 0, 1, 0 });
	tv.ReadFile();
	tv.CreateDeviceDependentResources(m_deviceResources);
	//tv.Rotate(90 * 3.14 / 180);
	tv.Translate({6, 0, -2 });
	tv.SetScaleMatrix(5, 5, 5);

	models.push_back(&tv);

	cube.SetFilePath("Assets/Plane.obj");
	cube.SetShaderResourceView(m_deviceResources->GetShaderResourceView(), true);
	//cube.SetTexturePath("Assets/Diffuse_Treehouse.dds");
	//handle render to texture


	//D3D11_TEXTURE2D_DESC textDesc = { 0 };
	//textDesc.Width = m_deviceResources->GetOutputSize().Width;
	//textDesc.Height = m_deviceResources->GetOutputSize().Height;
	//textDesc.MipLevels = 1;
	//textDesc.ArraySize = 1;
	//textDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE; //so I can write to text and I can read
	//textDesc.Usage = D3D11_USAGE_DEFAULT;
	//textDesc.SampleDesc.Count = 1;
	//textDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;

	//m_deviceResources->GetD3DDevice()->CreateTexture2D(&textDesc, NULL, &renderToTextureCube);

	//D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = { };
	//rtvDesc.Format = textDesc.Format;
	//rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	//rtvDesc.Texture2D.MipSlice = 0;

	//m_deviceResources->GetD3DDevice()->CreateRenderTargetView(renderToTextureCube.Get(), &rtvDesc, &rtvCube);

	//D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	//srvDesc.Format = textDesc.Format;
	//srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	//srvDesc.Texture2D.MostDetailedMip = 0;
	//srvDesc.Texture2D.MipLevels = 1;

	//m_deviceResources->GetD3DDevice()->CreateShaderResourceView(renderToTextureCube.Get(), &srvDesc, &srvCube);


	//{XMFLOAT3(-0.5f, -0.5f, -0.5f), XMFLOAT3(0.0f, 0.0f, 0.0f)},
	//{ XMFLOAT3(-0.5f, -0.5f,  0.5f), XMFLOAT3(0.0f, 0.0f, 1.0f) },
	//{ XMFLOAT3(-0.5f,  0.5f, -0.5f), XMFLOAT3(0.0f, 1.0f, 0.0f) },
	//{ XMFLOAT3(-0.5f,  0.5f,  0.5f), XMFLOAT3(0.0f, 1.0f, 1.0f) },
	//{ XMFLOAT3(0.5f, -0.5f, -0.5f), XMFLOAT3(1.0f, 0.0f, 0.0f) },
	//{ XMFLOAT3(0.5f, -0.5f,  0.5f), XMFLOAT3(1.0f, 0.0f, 1.0f) },
	//{ XMFLOAT3(0.5f,  0.5f, -0.5f), XMFLOAT3(1.0f, 1.0f, 0.0f) },
	//{ XMFLOAT3(0.5f,  0.5f,  0.5f), XMFLOAT3(1.0f, 1.0f, 1.0f) },

	cube.SetIsSceneTexture(true);
	cube.ReadFile();
	cube.CreateDeviceDependentResources(m_deviceResources);
	cube.SetIdentityMatrix();
	cube.Translate(XMFLOAT3{ -3, 1, 5 });

	models.push_back(&cube);

	Vertex screenVertices[6];
	unsigned int indices[] = { 0, 1, 2, 3, 4, 5 };
	float left, right, top, bottom;

	//left = (float)(-1 * (m_deviceResources->GetOutputSize().Width / 2.0f));
	//right = left + m_deviceResources->GetOutputSize().Width;
	//top = m_deviceResources->GetOutputSize().Height / 2.0f;
	//bottom = top - m_deviceResources->GetOutputSize().Height;

	left = -1.0f / 2;
	right = 1.0f / 2;
	top = 1.0f / 2;
	bottom = -1.0f / 2;

	//first triangle
	screenVertices[0].position = XMFLOAT3(left, top, 0); //Top Left
	screenVertices[0].uv = XMFLOAT2(0, 0);

	screenVertices[1].position = XMFLOAT3(right, top, 0); //Top Right
	screenVertices[1].uv = XMFLOAT2(1, 0);

	screenVertices[2].position = XMFLOAT3(left, bottom, 0); //Bottom Left
	screenVertices[2].uv = XMFLOAT2(0, 1);

	//second triangle
	screenVertices[3].position = XMFLOAT3(right, top, 0); //Top Right
	screenVertices[3].uv = XMFLOAT2(1, 0);

	screenVertices[4].position = XMFLOAT3(right, bottom, 0); //Bottom Right
	screenVertices[4].uv = XMFLOAT2(1, 1);

	screenVertices[5].position = XMFLOAT3(left, bottom, 0); //Bottom Left
	screenVertices[5].uv = XMFLOAT2(0, 1);

	scene.SetVerticesAndIndices(screenVertices, indices, 6, 6);
	scene.SetShaderResourceView(m_deviceResources->GetShaderResourceView(), true);
	//scene.SetTexturePath("Assets/Diffuse_Treehouse.dds");
	scene.SetIsSceneTexture(true);
	scene.SetIdentityMatrix();
	scene.CreateDeviceDependentResources(m_deviceResources);

	//models.push_back(&scene);

	//handle transparent objects

	//handle positions
	Vertex transCubeVertices[] =
	{
		{ XMFLOAT3(-0.5f, -0.5f, -0.5f),},
		{ XMFLOAT3(-0.5f, -0.5f,  0.5f),},
		{ XMFLOAT3(-0.5f,  0.5f, -0.5f),},
		{ XMFLOAT3(-0.5f,  0.5f,  0.5f),},
		{ XMFLOAT3(0.5f, -0.5f, -0.5f),},
		{ XMFLOAT3(0.5f, -0.5f,  0.5f),},
		{ XMFLOAT3(0.5f,  0.5f, -0.5f),},
		{ XMFLOAT3(0.5f,  0.5f,  0.5f),},
	};

	//Vertex transCubeVertices2[] =
	//{
	//	{ XMFLOAT3(-0.5f, -0.5f, -0.5f), XMFLOAT4(0.0f, 1.0f, 0.0f, 0.7f) },
	//	{ XMFLOAT3(-0.5f, -0.5f,  0.5f), XMFLOAT4(0.0f, 1.0f, 0.0f, 0.7f) },
	//	{ XMFLOAT3(-0.5f,  0.5f, -0.5f), XMFLOAT4(0.0f, 1.0f, 0.0f, 0.7f) },
	//	{ XMFLOAT3(-0.5f,  0.5f,  0.5f), XMFLOAT4(0.0f, 1.0f, 0.0f, 0.7f) },
	//	{ XMFLOAT3(0.5f, -0.5f, -0.5f), XMFLOAT4(0.0f, 1.0f, 0.0f, 0.7f) },
	//	{ XMFLOAT3(0.5f, -0.5f,  0.5f), XMFLOAT4(0.0f, 1.0f, 0.0f, 0.7f) },
	//	{ XMFLOAT3(0.5f,  0.5f, -0.5f), XMFLOAT4(0.0f, 1.0f, 0.0f, 0.7f) },
	//	{ XMFLOAT3(0.5f,  0.5f,  0.5f), XMFLOAT4(0.0f, 1.0f, 0.0f, 0.7f) },
	//};

	//Vertex transCubeVertices3[] =
	//{
	//	{ XMFLOAT3(-0.5f, -0.5f, -0.5f), XMFLOAT4(0.0f, 0.0f, 1.0f, 0.7f) },
	//	{ XMFLOAT3(-0.5f, -0.5f,  0.5f), XMFLOAT4(0.0f, 0.0f, 1.0f, 0.7f) },
	//	{ XMFLOAT3(-0.5f,  0.5f, -0.5f), XMFLOAT4(0.0f, 0.0f, 1.0f, 0.7f) },
	//	{ XMFLOAT3(-0.5f,  0.5f,  0.5f), XMFLOAT4(0.0f, 0.0f, 1.0f, 0.7f) },
	//	{ XMFLOAT3(0.5f, -0.5f, -0.5f), XMFLOAT4(0.0f, 0.0f, 1.0f, 0.7f) },
	//	{ XMFLOAT3(0.5f, -0.5f,  0.5f), XMFLOAT4(0.0f, 0.0f, 1.0f, 0.7f) },
	//	{ XMFLOAT3(0.5f,  0.5f, -0.5f), XMFLOAT4(0.0f, 0.0f, 1.0f, 0.7f) },
	//	{ XMFLOAT3(0.5f,  0.5f,  0.5f), XMFLOAT4(0.0f, 0.0f, 1.0f, 0.7f) },
	//};

	unsigned int transCubeIndices[] =
	{
		0,1,2, // -x
		1,3,2,

		4,6,5, // +x
		5,6,7,

		0,5,1, // -y
		0,4,5,

		2,7,6, // +y
		2,3,7,

		0,6,4, // -z
		0,2,6,

		1,7,3, // +z
		1,5,7,
	};

	transparentCube1.SetFilePath("Assets/Cube.obj");
	transparentCube1.SetVerticesAndIndices(transCubeVertices, transCubeIndices, ARRAYSIZE(transCubeVertices), ARRAYSIZE(transCubeIndices));
	transparentCube1.SetDirectionalLight({ 0.577f, 0.577f, -0.577f, 0 }, { 0.75f, 0.75f, 0.94f, 1.0f }, { 0.3f, 0.3f, 0.3f, 0.3f });
	transparentCube1.SetPointLight({ 2, 3.0f, 2, 0 }, { 1, 0, 0, 0 }, { 5, 0, 0, 0 });
	transparentCube1.SetSpotLight({ 0, 0, 0, 0 }, { 1.0f, 1.0f, 1.0f, 0 }, { 0.9f, 0, 0, 0 }, { 0, 0, 1, 0 });
	transparentCube1.ReadFile();
	transparentCube1.CreateDeviceDependentResources(m_deviceResources);
	transparentCube1.SetIdentityMatrix();
	transparentCube1.Translate(XMFLOAT3(2, 1, 1));
	transparentCube1.SetIsTransparent(true);

	transparentModels.push_back(&transparentCube1);

	transparentCube2.SetFilePath("Assets/Cube.obj");
	transparentCube2.SetVerticesAndIndices(transCubeVertices, transCubeIndices, ARRAYSIZE(transCubeVertices), ARRAYSIZE(transCubeIndices));
	transparentCube2.SetDirectionalLight({ 0.577f, 0.577f, -0.577f, 0 }, { 0.75f, 0.75f, 0.94f, 1.0f }, { 0.3f, 0.3f, 0.3f, 0.3f });
	transparentCube2.SetPointLight({ 2, 3.0f, 2, 0 }, { 1, 0, 0, 0 }, { 5, 0, 0, 0 });
	transparentCube2.SetSpotLight({ 0, 0, 0, 0 }, { 1.0f, 1.0f, 1.0f, 0 }, { 0.9f, 0, 0, 0 }, { 0, 0, 1, 0 });
	transparentCube2.ReadFile();
	transparentCube2.CreateDeviceDependentResources(m_deviceResources);
	transparentCube2.SetIdentityMatrix();
	transparentCube2.Translate(XMFLOAT3(2, 1, -0.5f));
	transparentCube2.SetIsTransparent(true);

	transparentModels.push_back(&transparentCube2);

	transparentCube3.SetFilePath("Assets/Cube.obj");
	transparentCube3.SetVerticesAndIndices(transCubeVertices, transCubeIndices, ARRAYSIZE(transCubeVertices), ARRAYSIZE(transCubeIndices));
	transparentCube3.SetDirectionalLight({ 0.577f, 0.577f, -0.577f, 0 }, { 0.75f, 0.75f, 0.94f, 1.0f }, { 0.3f, 0.3f, 0.3f, 0.3f });
	transparentCube3.SetPointLight({ 2, 3.0f, 2, 0 }, { 1, 0, 0, 0 }, { 5, 0, 0, 0 });
	transparentCube3.SetSpotLight({ 0, 0, 0, 0 }, { 1.0f, 1.0f, 1.0f, 0 }, { 0.9f, 0, 0, 0 }, { 0, 0, 1, 0 });
	transparentCube3.ReadFile();
	transparentCube3.CreateDeviceDependentResources(m_deviceResources);
	transparentCube3.SetIdentityMatrix();
	transparentCube3.Translate(XMFLOAT3(2, 1, -2.0f));
	transparentCube3.SetIsTransparent(true);

	transparentModels.push_back(&transparentCube3);


	//tree.SetFilePath("Assets/Tree.obj");
	//tree.SetTexturePath("Assets/Diffuse_Treehouse.dds");
	//tree.SetDirectionalLight({ 0.577f, 0.577f, -0.577f, 0 }, { 0.75f, 0.75f, 0.94f, 1.0f }, { 0.3f, 0.3f, 0.3f, 0.3f });
	//tree.SetPointLight({ 1, 4.0f, 1, 0 }, { 1, 1, 0, 0 }, { 5, 0, 0, 0 });
	//tree.ReadFile();
	//tree.CreateDeviceDependentResources(m_deviceResources);
	//tree.Translate({ -6.0f, 2.0f, 0 });

}

void Sample3DSceneRenderer::ReleaseDeviceDependentResources(void)
{
	m_loadingComplete = false;
	m_vertexShader.Reset();
	m_inputLayout.Reset();
	m_pixelShader.Reset();
	m_constantBuffer.Reset();
	m_vertexBuffer.Reset();
	m_indexBuffer.Reset();
}

