#include "pch.h"
#include "Model.h"

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

				uvs.push_back(tempUV);
			}
			else if (buffer[0] == 'f')
			{
				fin >> buffer;

				do
				{
					if (buffer[0] != 'f')
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
		for (int j = 0; j < bufferIndex.size(); ++j) //check if the vertex is unique
		{
			if (vertexIndices[i] - 1 == bufferIndex[j])
			{
				unique = false;
				break;
			}
			else
			{
				unique = true;
			}
		}

		if (unique)
		{
			Vertex tempRealVertex;

			tempRealVertex.position = positions[vertexIndices[i] - 1];
			tempRealVertex.uv = uvs[uvIndices[i] - 1];
			tempRealVertex.normal = normals[normalIndices[i] - 1];

			realVertices.push_back(tempRealVertex);
		}

		bufferIndex.push_back(vertexIndices[i] - 1);
	}

	fin.close();
}

void Model::SetFilePath(string path)
{
	filePath = path;
}

void Model::SetHeaderDetails(const OBJ_VERT *data, unsigned int numOfVerts)
{
	modelData = data;
	numVerts = numOfVerts;

	//vertices = new XMFLOAT3[numOfVerts];
	//uvs = new XMFLOAT2[numOfVerts];
	//normals = new XMFLOAT3[numOfVerts];
}

void Model::ReadFromHeader()
{
	//for (int i = 0; i < numVerts; ++i)
	//{
	//	//set xyz
	//	vertices[i].x = modelData[i].pos[0] * 0.1f;
	//	vertices[i].y = modelData[i].pos[1] * 0.1f;
	//	vertices[i].z = modelData[i].pos[2] * 0.1f;

	//	//set uvs & w
	//	uvs[i].x = modelData[i].uvw[0];
	//	uvs[i].y = modelData[i].uvw[1];

	//	//set normals
	//	normals[i].x = modelData[i].nrm[0];
	//	normals[i].y = modelData[i].nrm[1];
	//	normals[i].z = modelData[i].nrm[2];
	//}
}

void Model::CreateDeviceDependentResources(const std::shared_ptr<DX::DeviceResources>& deviceResources)
{
	m_deviceResources = deviceResources;

	// Load shaders asynchronously.
	auto loadVSTask = DX::ReadDataAsync(L"SampleVertexShader.cso");
	auto loadPSTask = DX::ReadDataAsync(L"SamplePixelShader.cso");

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

	auto createPSTask = loadPSTask.then([this](const std::vector<byte>& fileData) {
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreatePixelShader(
				&fileData[0],
				fileData.size(),
				nullptr,
				&m_pixelShader
			)
		);

		CD3D11_BUFFER_DESC constantBufferDesc(sizeof(ModelViewProjectionConstantBuffer), D3D11_BIND_CONSTANT_BUFFER);
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateBuffer(
				&constantBufferDesc,
				nullptr,
				&m_constantBuffer
			)
		);
	});

	//auto loadVSTask = DX::ReadDataAsync(L"SampleVertexShader.cso");
	//auto loadPSTask = DX::ReadDataAsync(L"SamplePixelShader.cso");

	//loadVSTask.then(

	//DX::ThrowIfFailed(
	//	m_deviceResources->GetD3DDevice()->CreateVertexShader(
	//		&fileData[0],
	//		fileData.size(),
	//		nullptr,
	//		&m_vertexShader)
	//);


	//	static const D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
	//{
	//	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	//	{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	//	{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	//};



	//m_deviceResources->GetD3DDevice()->CreateInputLayout(vertexDesc, 3, 

}
