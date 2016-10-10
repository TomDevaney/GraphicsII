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
				fin >> tempNormal.x; //NEGATE 
				fin >> tempNormal.y;
				fin >> tempNormal.z;

				tempNormal.x = -tempNormal.x;

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
				fin >> tempPosition.x; //NEGATE
				fin >> tempPosition.y;
				fin >> tempPosition.z;

				tempPosition.x = -tempPosition.x;

				positions.push_back(tempPosition);
			}
		}
	}

	for (int i = 0; i < vertexIndices.size(); i += 3)
	{
		unsigned int temp = vertexIndices[i + 1];
		vertexIndices[i + 1] = vertexIndices[i + 2];
		vertexIndices[i + 2] = temp;

		temp = uvIndices[i + 1];
		uvIndices[i + 1] = uvIndices[i + 2];
		uvIndices[i + 2] = temp;

		temp = normalIndices[i + 1];
		normalIndices[i + 1] = normalIndices[i + 2];
		normalIndices[i + 2] = temp;
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
	auto loadGSTask = DX::ReadDataAsync(L"MyFirstGeometryShader.cso");
	//auto loadVSTask2 = DX::ReadDataAsync(L"SkyBoxVertexShader.cso");
	//auto loadPSTask2 = DX::ReadDataAsync(L"SkyBoxPixelShader.cso");

//	m_lightConstantBufferData.isSkyBox.x = 0;


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
			{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "BINORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },

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

	if (isGeometry)
	{
		auto createGSTask = loadGSTask.then([this](const std::vector<byte>& fileData) {
			DX::ThrowIfFailed(
				m_deviceResources->GetD3DDevice()->CreateGeometryShader(&fileData[0], fileData.size(), nullptr, &m_geometryShader)
			);
		});
	}

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

	//Handle normal texture
	wstring wideNormalTexturePath = wstring(normalPath.begin(), normalPath.end());
	HRESULT hr2 = CreateDDSTextureFromFile(m_deviceResources->GetD3DDevice(), wideNormalTexturePath.c_str(), nullptr, &m_normalShaderResourceView);

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
	//if (isSkybox)
	//{
	//	context->VSSetShader(
	//		m_skyBoxVertexShader.Get(),
	//		nullptr,
	//		0
	//	);
	//}
	//else
	{
		context->VSSetShader(
			m_vertexShader.Get(),
			nullptr,
			0
		);
	}

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
	//if (isSkybox)
	//{
	//	context->PSSetShader(
	//		m_skyBoxPixelShader.Get(),
	//		nullptr,
	//		0
	//	);
	//}
	//else
	{
		context->PSSetShader(
			m_pixelShader.Get(),
			nullptr,
			0
		);
	}

	if (isGeometry)
	{
		context->GSSetShader(m_geometryShader.Get(), nullptr, 0);
	}

	if (isSkybox)
	{
		context->PSSetShaderResources(2, 1, m_shaderResourceView.GetAddressOf());
	}
	else
	{
		context->PSSetShaderResources(0, 1, m_shaderResourceView.GetAddressOf());
	}

	context->PSSetShaderResources(1, 1, m_normalShaderResourceView.GetAddressOf());

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
	XMStoreFloat4x4(&m_constantBufferData.projection, projection);
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

void Model::UpdateLightRadius(DX::StepTimer const& timer, float min, float max)
{
	const float delta_time = (float)timer.GetElapsedSeconds();

	m_lightConstantBufferData.lightRadius.x += deltaLight * delta_time;

	if (m_lightConstantBufferData.lightRadius.x >= max)
	{
		m_lightConstantBufferData.lightRadius.x = max;
		deltaLight *= -1;
	}
	else if (m_lightConstantBufferData.lightRadius.x <= min)
	{
		m_lightConstantBufferData.lightRadius.x = min;
		deltaLight *= -1;
	}
}

void Model::UpdatePointLightPosition(DX::StepTimer const& timer, float min, float max)
{
	const float delta_time = (float)timer.GetElapsedSeconds();

	m_lightConstantBufferData.pointLightPosition.x += deltaPosition * delta_time;

	if (m_lightConstantBufferData.pointLightPosition.x >= max)
	{
		m_lightConstantBufferData.pointLightPosition.x = max;
		deltaPosition *= -1;
	}
	else if (m_lightConstantBufferData.pointLightPosition.x <= min)
	{
		m_lightConstantBufferData.pointLightPosition.x = min;
		deltaPosition *= -1;
	}
}

void Model::UpdateDirectionalLight(DX::StepTimer const& timer, bool left)
{
	const float delta_time = (float)timer.GetElapsedSeconds();

	if (left)
	{
		m_lightConstantBufferData.dirLightNorm.x += 1 * delta_time;
		m_lightConstantBufferData.dirLightNorm.z += 1 * delta_time;
	}
	else
	{
		m_lightConstantBufferData.dirLightNorm.x -= 1 * delta_time;
		m_lightConstantBufferData.dirLightNorm.z -= 1 * delta_time;

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

void Model::UpdateSpotLight(XMFLOAT4X4 camera)
{
	m_lightConstantBufferData.spotLightPosition.x = camera._41;
	m_lightConstantBufferData.spotLightPosition.y = camera._42;
	m_lightConstantBufferData.spotLightPosition.z = camera._43;

	m_lightConstantBufferData.coneDirection.x = camera._31;
	m_lightConstantBufferData.coneDirection.y = camera._32;
	m_lightConstantBufferData.coneDirection.z = camera._33;
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

void Model::SetNormalPath(string path)
{
	normalPath = path;
	m_lightConstantBufferData.isNormalMap.x = 1;
}

void Model::CalculateTangentBinormal(Vertex v1, Vertex v2, Vertex v3, XMFLOAT3 &tangent, XMFLOAT3 &binormal)
{
	XMFLOAT3 vector1, vector2;
	XMFLOAT2 uvVector[2];
	float den;
	float length;

	// Calculate the two vectors for this face.
	vector1.x = v2.position.x - v1.position.x;
	vector1.y = v2.position.y - v1.position.y;
	vector1.z = v2.position.z - v1.position.z;

	vector2.x = v3.position.x - v1.position.x;
	vector2.y = v3.position.y - v1.position.y;
	vector2.z = v3.position.z - v1.position.z;

	// Calculate the uv vectors.
	uvVector[0].x = v2.uv.x - v1.uv.x;
	uvVector[0].y = v2.uv.y - v1.uv.y;

	uvVector[1].x = v3.uv.x - v1.uv.x;
	uvVector[1].y = v3.uv.y - v1.uv.y;

	// Calculate the denominator of the tangent/binormal equation.
	den = 1.0f / (uvVector[0].x * uvVector[1].y - uvVector[1].x * uvVector[0].y);

	// Calculate the cross products and multiply by the coefficient to get the tangent and binormal.
	tangent.x = (uvVector[1].y * vector1.x - uvVector[0].y * vector2.x) * den;
	tangent.y = (uvVector[1].y * vector1.y - uvVector[0].y * vector2.y) * den;
	tangent.z = (uvVector[1].y * vector1.z - uvVector[0].y * vector2.z) * den;

	binormal.x = (uvVector[0].x * vector2.x - uvVector[1].x * vector1.x) * den;
	binormal.y = (uvVector[0].x * vector2.y - uvVector[1].x * vector1.y) * den;
	binormal.z = (uvVector[0].x * vector2.z - uvVector[1].x * vector1.z) * den;

	// Calculate the length of this normal.
	length = sqrt((tangent.x * tangent.x) + (tangent.y * tangent.y) + (tangent.z * tangent.z));

	// Normalize the normal and then store it
	tangent.x = tangent.x / length;
	tangent.y = tangent.y / length;
	tangent.z = tangent.z / length;

	// Calculate the length of this normal.
	length = sqrt((binormal.x * binormal.x) + (binormal.y * binormal.y) + (binormal.z * binormal.z));

	// Normalize the normal and then store it
	binormal.x = binormal.x / length;
	binormal.y = binormal.y / length;
	binormal.z = binormal.z / length;
}

void Model::CalculateNewNormal(XMFLOAT3 tangent, XMFLOAT3 binormal, XMFLOAT3 &normal)
{
	float length;

	// Calculate the normal vector.
	normal.x = (tangent.y * binormal.z) - (tangent.z * binormal.y);
	normal.y = (tangent.z * binormal.x) - (tangent.x * binormal.z);
	normal.z = (tangent.x * binormal.y) - (tangent.y * binormal.x);

	// Calculate the length of the normal.
	length = sqrt((normal.x * normal.x) + (normal.y * normal.y) + (normal.z * normal.z));

	// Normalize the normal.
	normal.x = normal.x / length;
	normal.y = normal.y / length;
	normal.z = normal.z / length;
}

void Model::CalculateNewNormalsTangentsNormals()
{
	XMFLOAT3 tangent;
	XMFLOAT3 binormal;
	XMFLOAT3 normal;

	for (int i = 0; i < bufferIndex.size(); i += 3)
	{
		Vertex v1 = realVertices[bufferIndex[i]];
		Vertex v2 = realVertices[bufferIndex[i + 1]];
		Vertex v3 = realVertices[bufferIndex[i + 2]];

		CalculateTangentBinormal(v1, v2, v3, tangent, binormal);
		CalculateNewNormal(tangent, binormal, normal);

		realVertices[bufferIndex[i]].normal = normal;
		realVertices[bufferIndex[i]].tangent = tangent;
		realVertices[bufferIndex[i]].binormal = binormal;

		realVertices[bufferIndex[i + 1]].normal = normal;
		realVertices[bufferIndex[i + 1]].tangent = tangent;
		realVertices[bufferIndex[i + 1]].binormal = binormal;

		realVertices[bufferIndex[i + 2]].normal = normal;
		realVertices[bufferIndex[i + 2]].tangent = tangent;
		realVertices[bufferIndex[i + 2]].binormal = binormal;
	}
}

void Model::SetIsSkybox(bool toggle)
{
	isSkybox = toggle;
	m_lightConstantBufferData.isSkyBox.x = 1;
	m_constantBufferData.isSkyBox.x = 1;
}

void Model::SetIdentityMatrix()
{
	XMStoreFloat4x4(&m_constantBufferData.model, XMMatrixIdentity());
}

void Model::SetScaleMatrix(float x, float y, float z)
{
	m_constantBufferData.model._11 = 1 * x;
	m_constantBufferData.model._22 = 1 * y;
	m_constantBufferData.model._33 = 1 * z;

}

void Model::SetGeometryShader(vector<Vertex> points)
{
	isGeometry = true;

	geometryPoints = points;
}

