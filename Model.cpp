#include "Model.h"



struct MODEL_CONSTANT_BUFFER

{

	XMMATRIX WorldViewProjection; // 64 bytes ( 4 x 4 = 16 floats x 4 bytes)

}; // TOTAL SIZE = 64 bytes

Model::Model(ID3D11Device* device, ID3D11DeviceContext* context)
{
	m_pTexture0 = NULL;
	m_pSampler0 = NULL;
	m_pD3DDevice = device;
	m_pImmediateContext = context;

	m_x, m_y, m_z = 10.0f;
	m_dx, m_dz, m_rotation = 0.0f;
	m_xangle, m_zangle, m_yangle = 0.0f;
	m_scale = 0.5f;


	//tutorial 10 section 10/b
	//m_pVShader = 

}

int Model::LoadObjModel(char* filename)
{
	HRESULT hr = S_OK;
	D3D11_BUFFER_DESC constant_buffer_desc;
	ZeroMemory(&constant_buffer_desc, sizeof(constant_buffer_desc));

	constant_buffer_desc.Usage = D3D11_USAGE_DEFAULT;
	constant_buffer_desc.ByteWidth = 64;
	constant_buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

	hr = m_pD3DDevice->CreateBuffer(&constant_buffer_desc, NULL, &m_pConstantBuffer);

	ID3DBlob *VS, *PS, *error;

	hr = D3DX11CompileFromFile("model_shaders.hlsl", 0, 0, "ModelVS", "vs_4_0", 0, 0, 0, &VS, &error,

		0);

	if (error != 0) // check for shader compilation error

	{

		OutputDebugStringA((char*)error->GetBufferPointer());

		error->Release();

		if (FAILED(hr)) // don't fail if error is just a warning

		{

			return hr;

		};

	}

	hr = D3DX11CompileFromFile("model_shaders.hlsl", 0, 0, "ModelPS", "ps_4_0", 0, 0, 0, &PS, &error,

		0);

	if (error != 0)// check for shader compilation error

	{

		OutputDebugStringA((char*)error->GetBufferPointer());

		error->Release();

		if (FAILED(hr))// don't fail if error is just a warning

		{

			return hr;

		};

	}


	// Create shader objects

	hr = m_pD3DDevice->CreateVertexShader(VS->GetBufferPointer(), VS->GetBufferSize(), NULL, &m_pVShader);

	if (FAILED(hr))

	{

		return hr;

	}



	hr = m_pD3DDevice->CreatePixelShader(PS->GetBufferPointer(), PS->GetBufferSize(), NULL,

		&m_pPShader);

	if (FAILED(hr))

	{

		return hr;

	}


	m_pObject = new ObjFileModel(filename, m_pD3DDevice, m_pImmediateContext);

	//m_pInputLayout = new CreateInputLayout

	D3D11_INPUT_ELEMENT_DESC iedesc[] =

	{

		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }

	};

	D3D11_SAMPLER_DESC sampler_desc;
	ZeroMemory(&sampler_desc, sizeof(sampler_desc));
	sampler_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampler_desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampler_desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampler_desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampler_desc.MaxLOD = D3D11_FLOAT32_MAX;

	m_pD3DDevice->CreateSamplerState(&sampler_desc, &m_pSampler0);

	hr = m_pD3DDevice->CreateInputLayout(iedesc, ARRAYSIZE(iedesc), VS->GetBufferPointer(), VS->GetBufferSize(),

		&m_pInputLayout);

	if (FAILED(hr))

	{

		return hr;

	}

	CalculateBoundingSphereRadius();

	m_pImmediateContext->IASetInputLayout(m_pInputLayout);
	return 1;
}


void Model::draw(XMMATRIX* world, XMMATRIX *view, XMMATRIX *projection)
{

	/*MODEL_CONSTANT_BUFFER model_cb_values;
	
	m_pImmediateContext->VSSetConstantBuffers(0, 1, &m_pConstantBuffer);
	m_pImmediateContext->IASetInputLayout(m_pInputLayout);
	m_pImmediateContext->VSSetShader(m_pVShader, 0, 0);

	m_pImmediateContext->PSSetShader(m_pPShader, 0, 0);
	m_pImmediateContext->UpdateSubresource(m_pConstantBuffer, 0, 0, &model_cb_values, 0, 0);


	m_pObject->Draw();*/

	MODEL_CONSTANT_BUFFER model_cb_values;
	model_cb_values.WorldViewProjection = (*world)*(*view)*(*projection);
	m_pImmediateContext->VSSetConstantBuffers(0, 1, &m_pConstantBuffer);
	m_pImmediateContext->UpdateSubresource(m_pConstantBuffer, 0, 0, &model_cb_values, 0, 0);

	m_pImmediateContext->VSSetShader(m_pVShader, 0, 0);
	m_pImmediateContext->PSSetShader(m_pPShader, 0, 0);
	m_pImmediateContext->IASetInputLayout(m_pInputLayout);
	m_pImmediateContext->PSSetShaderResources(0, 1, &m_pTexture0);
	m_pImmediateContext->PSSetSamplers(0, 1, &m_pSampler0);

	m_pObject->Draw();

}

Model::~Model()
{


	m_pConstantBuffer->Release();
	m_pInputLayout->Release();
	m_pVShader->Release();
	m_pPShader->Release();
	m_pObject->~ObjFileModel();
	m_pObject = NULL;
	m_pD3DDevice->Release();
}

void Model::setPosition(float x, float y, float z)
{
	m_x = x;
	m_y = y;
	m_z = z;
}
void Model::setRotation(float x, float y, float z)
{
	m_xangle = x;
	m_zangle = y;
	m_yangle = z;
}
void Model::setScale(float value)
{
	m_scale = value;
}
float Model::getPositionX()
{
	return m_x;
}
float Model::getPositionY()
{
	return m_y;
}
float Model::getPositionZ()
{
	return m_z;
}
float Model::getScale()
{
	return m_scale;
}
void Model::incPositionX(float value)
{
	m_x += value;
}
void Model::incPositionY(float value)
{
	m_y += value;
}
void Model::incPositionZ(float value)
{
	m_z += value;
}
void Model::LookAt_XZ(float x, float z)
{
	m_dx = sin(m_rotation*(XM_PI / 180.0));
	m_dz = cos(m_rotation*(XM_PI / 180.0));

	m_yangle = atan2(m_dx, m_dz) * (180.0 / XM_PI);
}
void Model::MoveForward(float distance)
{
	m_x += sin(m_yangle * (XM_PI / 180.0f))*distance;
	m_z += cos(m_yangle * (XM_PI / 180.0f))*distance;
}

void Model::CalculateModelCenterPoint()
{
	float max = 0.0f;
	float min = 0.0f;
	float difference = 0.0f;

	//x
	for (int i = 0; i < m_pObject->numverts; i++)
	{
		if (m_pObject->vertices[i].Pos.x<max)
		{
			max = m_pObject->vertices[i].Pos.x;
		}
		if (m_pObject->vertices[i].Pos.x>min)
		{
			min = m_pObject->vertices[i].Pos.x;
		}
	}

	difference = max - min;
	m_bounding_sphere_centre_x = max - (difference / 2);

	//y
	max, min, difference = 0.0f;

	for (int i = 0; i < m_pObject->numverts; i++)
	{
		if (m_pObject->vertices[i].Pos.y<max)
		{
			max = m_pObject->vertices[i].Pos.y;
		}
		if (m_pObject->vertices[i].Pos.y>min)
		{
			min = m_pObject->vertices[i].Pos.y;
		}
	}

	difference = max - min;
	m_bounding_sphere_centre_y = max - (difference / 2);

	difference = max - min;
	m_bounding_sphere_centre_x = max - (difference / 2);

	//z
	max, min, difference = 0.0f;

	for (int i = 0; i < m_pObject->numverts; i++)
	{
		if (m_pObject->vertices[i].Pos.z<max)
		{
			max = m_pObject->vertices[i].Pos.z;
		}
		if (m_pObject->vertices[i].Pos.z>min)
		{
			min = m_pObject->vertices[i].Pos.z;
		}
	}

	difference = max - min;
	m_bounding_sphere_centre_z = max - (difference / 2);
}

void Model::CalculateBoundingSphereRadius()
{
	float max = 0;
	float distance;

	for (int i = 0; i < m_pObject->numverts; i++)
	{
		if (m_pObject->vertices[i].Pos.x > m_bounding_sphere_centre_x)
		{
			distance = m_pObject->vertices[i].Pos.x - m_bounding_sphere_centre_x;

			if (m_pObject->vertices[i].Pos.x < max)
			{
				max = distance;
			}
		}

		if (m_pObject->vertices[i].Pos.x < m_bounding_sphere_centre_x)
		{
			distance = m_bounding_sphere_centre_x - m_pObject->vertices[i].Pos.x;

			if (m_pObject->vertices[i].Pos.x < max)
			{
				max = distance;
			}
		}

		    if (m_pObject->vertices[i].Pos.y > m_bounding_sphere_centre_y)
			{
				distance = m_pObject->vertices[i].Pos.y - m_bounding_sphere_centre_y;

				if (m_pObject->vertices[i].Pos.y < max)
				{
					max = distance;
				}
			}

			if (m_pObject->vertices[i].Pos.y < m_bounding_sphere_centre_y)
			{
				distance = m_bounding_sphere_centre_y - m_pObject->vertices[i].Pos.y;

				if (m_pObject->vertices[i].Pos.y < max)
				{
					max = distance;
				}
			}

		
				if (m_pObject->vertices[i].Pos.z > m_bounding_sphere_centre_x)
				{
					distance = m_pObject->vertices[i].Pos.z - m_bounding_sphere_centre_z;

					if (m_pObject->vertices[i].Pos.z < max)
					{
						max = distance;
					}
				}

				if (m_pObject->vertices[i].Pos.z < m_bounding_sphere_centre_z)
				{
					distance = m_bounding_sphere_centre_z - m_pObject->vertices[i].Pos.z;

					if (m_pObject->vertices[i].Pos.z < max)
					{
						max = distance;
					}
				}


				if (m_pObject->vertices[i].Pos.x < max)
				{
					max = m_pObject->vertices[i].Pos.z;
				}

			


		

		m_bounding_sphere_radius = max;

	}
}

XMVECTOR Model::GetBoundingSphereWorldSpacePosition()
{
	XMMATRIX world;
	XMVECTOR offset;

	world *= XMMatrixTranslation(1, 0, 10);
	world = XMMatrixRotationY(XMConvertToRadians(m_yangle));
	world = XMMatrixScaling(m_scale, m_scale, m_scale);

	offset = XMVectorSet(m_bounding_sphere_centre_x, m_bounding_sphere_centre_y, m_bounding_sphere_centre_z, 0.0);

	offset = XMVector3Transform(offset, world); 

	return offset;


}

float Model::GetBoundingSphereRadius()
{
	return (m_scale*m_bounding_sphere_radius);
}

bool Model::CheckCollision(Model* a_model)
{
	if (a_model == this)
	{
		return false;
	}

	XMVECTOR This_Model = this->GetBoundingSphereWorldSpacePosition();
	XMVECTOR That_Model = a_model->GetBoundingSphereWorldSpacePosition();

	float r1 = GetBoundingSphereRadius();
	float r2 = a_model->GetBoundingSphereRadius();

	float x1 = XMVectorGetX(This_Model);
	float y1 = XMVectorGetY(This_Model);
	float z1 = XMVectorGetZ(This_Model);

	float x2 = XMVectorGetX(That_Model);
	float y2 = XMVectorGetY(That_Model);
	float z2 = XMVectorGetZ(That_Model);

	float distance_squared = pow(x1 + x2, 2) + pow(y1 + y2, 2) + pow(z1 + z2, 2);
	if (distance_squared < pow(r1 + r2, 2))
	{
		return true;
	}

}