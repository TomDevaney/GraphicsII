#include "pch.h"
#include "Model.h"

#define LCOPY(s) L##s
#define TOWCHAR(s) LCOPY(s)

//Model::~Model()
//{
//	delete[] vertices;
//	delete[] uvs;
//	delete[] normals;
//}

void Model::ReadFile()
{
	string buffer;
	XMFLOAT3 tempPosition;
	XMFLOAT2 tempUV;
	XMFLOAT3 tempNormal;
	vector<XMFLOAT3> positions;
	vector<XMFLOAT2> uvs;
	vector<XMFLOAT3> normals;
	vector<unsigned int> vertexIndices, uvIndices, normalIndices;

	fstream fin;
	fin.open(filePath, ios_base::in);

	if (fin.is_open())
	{
		while (true)
		{
			fin >> buffer;

			if (fin.eof())
			{
				break;
			}
			else if (buffer[0] == 'v' && buffer[1] == 'n')
			{
				fin >> tempNormal.x;
				fin >> tempNormal.y;
				fin >> tempNormal.z;

				normals.push_back(tempNormal);
			}
			else if (buffer[0] == 'v'  && buffer[1] == 't')
			{
				fin >> tempUV.x;
				fin >> tempUV.y;

				tempUV.y = 1 - tempUV.y;

				uvs.push_back(tempUV);
			}
			else if (buffer[0] == 'f' && buffer.size() == 1)
			{
				fin >> buffer;

				do
				{
					if (buffer[0] != 'f' && buffer[0] != 's')
					{
						int index1 = 0;
						int index2 = buffer.find('/', 0);
						string tempVertexIndex = buffer.substr(index1, index2);
						index1 = buffer.find('/', index2 + 1);
						string tempUvIndex = buffer.substr(index2 + 1, index1 - (index2 + 1));
						index2 = buffer.find('/', index1);
						string tempNormalIndex = buffer.substr(index2 + 1, 100);

						vertexIndices.push_back(stoi(tempVertexIndex));
						uvIndices.push_back(stoi(tempUvIndex));
						normalIndices.push_back(stoi(tempNormalIndex));
					}
					fin >> buffer;

				} while (!fin.eof());
			}
			else if (buffer[0] == 'v')
			{
				fin >> tempPosition.x;
				fin >> tempPosition.y;
				fin >> tempPosition.z;

				positions.push_back(tempPosition);
			}
		}
	}

	bool unique = true;

	for (int i = 0; i < vertexIndices.size(); ++i)
	{
		Vertex tempRealVertex;
		unsigned int tempBufferIndex;

		tempRealVertex.position = positions[vertexIndices[i] - 1];
		tempRealVertex.uv = uvs[uvIndices[i] - 1];
		tempRealVertex.normal = normals[normalIndices[i] - 1];


		for (int j = 0; j < realVertices.size(); ++j) //check if the vertex is unique
		{
			if (tempRealVertex == realVertices[j])
			{
				unique = false;
				tempBufferIndex = j;
				break;
			}
			else
			{
				unique = true;
			}
		}

		if (unique)
		{
			tempBufferIndex = realVertices.size();
			realVertices.push_back(tempRealVertex);
		}

		bufferIndex.push_back(tempBufferIndex);
	}

	fin.close();
}

void Model::SetFilePath(string path)
{
	filePath = path;
}

//void Model::SetHeaderDetails(const OBJ_VERT *data, unsigned int numOfVerts)
//{
//	modelData = data;
//	numVerts = numOfVerts;
//
//	//vertices = new XMFLOAT3[numOfVerts];
//	//uvs = new XMFLOAT2[numOfVerts];
//	//normals = new XMFLOAT3[numOfVerts];
//}
//
//void Model::ReadFromHeader()
//{
//	//for (int i = 0; i < numVerts; ++i)
//	//{
//	//	//set xyz
//	//	vertices[i].x = modelData[i].pos[0] * 0.1f;
//	//	vertices[i].y = modelData[i].pos[1] * 0.1f;
//	//	vertices[i].z = modelData[i].pos[2] * 0.1f;
//
//	//	//set uvs & w
//	//	uvs[i].x = modelData[i].uvw[0];
//	//	uvs[i].y = modelData[i].uvw[1];
//
//	//	//set normals
//	//	normals[i].x = modelData[i].nrm[0];
//	//	normals[i].y = modelData[i].nrm[1];
//	//	normals[i].z = modelData[i].nrm[2];
//	//}
//}

void Model::CreateDeviceDependentResources(const std::shared_ptr<DX::DeviceResources>& deviceResources)
{
	m_deviceResources = deviceResources;

	// Load shaders asynchronously.
	auto loadVSTask = DX::ReadDataAsync(L"MyFirstVertexShader.cso");
	auto loadPSTask = DX::ReadDataAsync(L"MyFirstPixelShader.cso");

	// After the vertex shader file is loaded, create the shader and input layout.
	auto createVSTask = loadVSTask.then([this](const std::vector<byte>& fileData) {
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateVertexShader(
				&fileData[0],
				fileData.size(),
				nullptr,
				&m_vertexShader
			)
		);

		static const D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
		{	
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },

			{ "INSTANCEPOS", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},
		};

		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateInputLayout(
				vertexDesc,
				ARRAYSIZE(vertexDesc),
				&fileData[0],
				fileData.size(),
				&m_inputLayout
			)
		);
	});

	//Create pixel shader
	auto createPSTask = loadPSTask.then([this](const std::vector<byte>& fileData) {
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreatePixelShader(
				&fileData[0],
				fileData.size(),
				nullptr,
				&m_pixelShader
			)
		);
		 
		//Create constant buffer to matrices and isInstance
		CD3D11_BUFFER_DESC constantBufferDesc(sizeof(ModelViewProjectionConstantBuffer), D3D11_BIND_CONSTANT_BUFFER);
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateBuffer(
				&constantBufferDesc,
				nullptr,
				&m_constantBuffer
			)
		);

		//Create constant buffer for lighting
		CD3D11_BUFFER_DESC lightConstantBufferDesc(sizeof(LightingConstantBuffer), D3D11_BIND_CONSTANT_BUFFER);
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateBuffer(
				&lightConstantBufferDesc,
				nullptr,
				&m_lightConstantBuffer
			)
		);
	});

	// Once both shaders are loaded, create the mesh.
	auto createModelTask = (createPSTask && createVSTask).then([this]() {


		D3D11_SUBRESOURCE_DATA vertexBufferData = { 0 };
		vertexBufferData.pSysMem = realVertices.data();
		vertexBufferData.SysMemPitch = 0;
		vertexBufferData.SysMemSlicePitch = 0;
		CD3D11_BUFFER_DESC vertexBufferDesc(sizeof(Vertex) * realVertices.size(), D3D11_BIND_VERTEX_BUFFER);
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateBuffer(
				&vertexBufferDesc,
				&vertexBufferData,
				&m_vertexBuffer
			)
		);

		// Load mesh indices.

		m_indexCount = bufferIndex.size();

		D3D11_SUBRESOURCE_DATA indexBufferData = { 0 };
		indexBufferData.pSysMem = bufferIndex.data();
		indexBufferData.SysMemPitch = 0;
		indexBufferData.SysMemSlicePitch = 0;
		CD3D11_BUFFER_DESC indexBufferDesc(sizeof(unsigned int) * bufferIndex.size(), D3D11_BIND_INDEX_BUFFER);
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateBuffer(
				&indexBufferDesc,
				&indexBufferData,
				&m_indexBuffer
			)
		);

		if (isInstanced)
		{
			//Get offsets of each instance
			vector<InstanceData> instances(numOfInstances);

			for (int i = 0; i < instances.size(); ++i)
			{
				instances[i].pos.x = (i % 10) + instanceWidth;
				instances[i].pos.z = (i / 10) + instanceWidth;
			}

			m_constantBufferData.isInstance.x = 1;


			//Create instance buffer
			D3D11_SUBRESOURCE_DATA instanceBufferData = { 0 };
			instanceBufferData.pSysMem = instances.data();
			instanceBufferData.SysMemPitch = 0;
			instanceBufferData.SysMemSlicePitch = 0;
			CD3D11_BUFFER_DESC instanceBufferDesc(sizeof(InstanceData) * instances.size(), D3D11_BIND_VERTEX_BUFFER);
			DX::ThrowIfFailed(
				m_deviceResources->GetD3DDevice()->CreateBuffer(
					&instanceBufferDesc,
					&instanceBufferData,
					&m_instanceBuffer
				)
			);
		}
		else
		{
			m_constantBufferData.isInstance.x = 0;

		}
	});

	//Handle texture
	wstring wideTexturePath = wstring(texturePath.begin(), texturePath.end());
	HRESULT hr = CreateDDSTextureFromFile(m_deviceResources->GetD3DDevice(), wideTexturePath.c_str(), nullptr, &m_shaderResourceView);

	createModelTask.then([this]() {
		m_loadingComplete = true;
	});
}

void Model::Render()
{
	// Loading is asynchronous. Only draw geometry after it's loaded.
	if (!m_loadingComplete)
	{
		return;
	}

	auto context = m_deviceResources->GetD3DDeviceContext();

	// Prepare the constant buffer to send it to the graphics device.
	context->UpdateSubresource1(
		m_constantBuffer.Get(),
		0,
		NULL,
		&m_constantBufferData,
		0,
		0,
		0
	);

	//Prepare light constant buffer
	context->UpdateSubresource1(
		m_lightConstantBuffer.Get(),
		0,
		NULL,
		&m_lightConstantBufferData,
		0,
		0,
		0
	);

	// Each vertex is one instance of the Vertex struct.
	UINT stride = sizeof(Vertex);
	UINT offset = 0;



	if (isInstanced)
	{
		UINT strides[2] = { sizeof(Vertex), sizeof(InstanceData) };
		UINT offsets[2] = { 0, 0 };
		ID3D11Buffer* instBuffer[2] = { *m_vertexBuffer.GetAddressOf(), *m_instanceBuffer.GetAddressOf() }; //I should probably release this somehow

		context->IASetVertexBuffers(
			0,
			2,
			instBuffer,
			strides,
			offsets
		);
	}
	else
	{
		context->IASetVertexBuffers(
			0,
			1,
			m_vertexBuffer.GetAddressOf(),
			&stride,
			&offset
		);
	}

	context->IASetIndexBuffer(
		m_indexBuffer.Get(),
		DXGI_FORMAT_R32_UINT, // Each index is one 32-bit unsigned integer (uint).
		0
	);

	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	context->IASetInputLayout(m_inputLayout.Get());

	// Attach our vertex shader.
	context->VSSetShader(
		m_vertexShader.Get(),
		nullptr,
		0
	);

	// Send the constant buffer to the graphics device.
	context->VSSetConstantBuffers1(
		0,
		1,
		m_constantBuffer.GetAddressOf(),
		nullptr,
		nullptr
	);

	//Send light constant buffer to pixel shader
	context->PSSetConstantBuffers1(
		0,
		1,
		m_lightConstantBuffer.GetAddressOf(),
		nullptr,
		nullptr
	);

	// Attach our pixel shader.
	context->PSSetShader(
		m_pixelShader.Get(),
		nullptr,
		0
	);

	context->PSSetShaderResources(0, 1, m_shaderResourceView.GetAddressOf());
	//context->PSSetSamplers(0, 1, m_samplerState.GetAddressOf());

	// Draw the objects.
	if (isInstanced)
	{
		context->DrawIndexedInstanced(m_indexCount, numOfInstances, 0, 0, 0);
	}
	else
	{
		context->DrawIndexed(
			m_indexCount,
			0,
			0
		);
	}
}

void Model::Translate(XMFLOAT3 distance)
{
	XMStoreFloat4x4(&m_constantBufferData.model, XMMatrixTranspose(XMMatrixTranslation(distance.x, distance.y, distance.z)));
}

void Model::SetProjection(XMMATRIX projection)
{
	XMStoreFloat4x4(&m_constantBufferData.projection ,projection);
}

void Model::SetView(XMMATRIX view)
{
	XMStoreFloat4x4(&m_constantBufferData.view, view);
}

void Model::SetTexturePath(string path)
{
	texturePath = path;
}

void Model::SetInstanceData(unsigned int numInstances, unsigned int widthOfInstance)
{
	isInstanced = true;
	numOfInstances = numInstances;
	instanceWidth = widthOfInstance;
}

void Model::SetDirectionalLight(XMFLOAT4 directionalLightDirection, XMFLOAT4 directionalLightColor, XMFLOAT4 ambientRatio) // {directional, point, spot, 0}
{
	m_lightConstantBufferData.ambientLight = ambientRatio;
	m_lightConstantBufferData.dirLightNorm = directionalLightDirection;
	m_lightConstantBufferData.dirLightColor = directionalLightColor;
	m_lightConstantBufferData.typeOfLight.x = 1.0f;
}

void Model::SetPointLight(XMFLOAT4 pointPosition, XMFLOAT4 pointLightColor, XMFLOAT4 lightRadius)
{
	m_lightConstantBufferData.pointLightPosition = pointPosition;
	m_lightConstantBufferData.pointLightColor = pointLightColor;
	m_lightConstantBufferData.typeOfLight.y = 1.0f;
	m_lightConstantBufferData.lightRadius = lightRadius;
}

void Model::UpdateLightRadius(DX::StepTimer const& timer)
{
	const float delta_time = (float)timer.GetElapsedSeconds();

	m_lightConstantBufferData.lightRadius.x += deltaLight * delta_time;

	if (m_lightConstantBufferData.lightRadius.x >= 5)
	{
		m_lightConstantBufferData.lightRadius.x = 5;
		deltaLight *= -1;
	}
	else if (m_lightConstantBufferData.lightRadius.x <= 1)
	{
		m_lightConstantBufferData.lightRadius.x = 1;
		deltaLight *= -1;
	}
}

void Model::SetSpotLight(XMFLOAT4 spotPosition, XMFLOAT4 spotLightColor, XMFLOAT4 coneRatio, XMFLOAT4 coneDirection)
{
	m_lightConstantBufferData.spotLightPosition = spotPosition;
	m_lightConstantBufferData.spotLightColor = spotLightColor;
	m_lightConstantBufferData.coneRatio = coneRatio;
	m_lightConstantBufferData.typeOfLight.z = 1;
	m_lightConstantBufferData.coneDirection = coneDirection;
}

void Model::UpdateSpotLight(XMFLOAT4X4 camera, XMVECTOR camTarget)
{
	m_lightConstantBufferData.spotLightPosition.x = camera._41;
	m_lightConstantBufferData.spotLightPosition.y = camera._42 + 0.699999928;
	m_lightConstantBufferData.spotLightPosition.z = camera._43 - 1.5f;

	m_lightConstantBufferData.coneDirection.x = XMVectorGetX(camTarget) - m_lightConstantBufferData.spotLightPosition.x;
	m_lightConstantBufferData.coneDirection.y = XMVectorGetY(camTarget) - m_lightConstantBufferData.spotLightPosition.y;
	m_lightConstantBufferData.coneDirection.z = XMVectorGetZ(camTarget) - m_lightConstantBufferData.spotLightPosition.z;
}

void Model::Rotate(float radians)
{
	//XMFLOAT3 position = { m_constantBufferData.model._14, m_constantBufferData.model._24, m_constantBufferData.model._34 };

	//Translate({ 0, 0, 0 }); //translate to origin

	XMStoreFloat4x4(&m_constantBufferData.model, XMMatrixTranspose(XMMatrixRotationY(radians)));
	//XMStoreFloat4x4(&m_constantBufferData.model, XMMatrixTranspose(XMMatrixMultiply(XMMatrixTranslation(position.x, position.y, position.z), XMMatrixRotationY(radians))));
	//Translate(position); //translate back to original position

	//XMFLOAT4X4 final;

	//XMStoreFloat4x4(&final, XMMatrixTranspose(XMMatrixRotationY(radians) * XMMatrixTranslation(position.x, position.y, position.z)));

	//final._14 = position.x;
	//final._24 = position.y;
	//final._34 = position.z;

	//m_constantBufferData.model = final;
}