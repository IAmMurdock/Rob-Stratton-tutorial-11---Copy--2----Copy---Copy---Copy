#pragma once

#include "objfilemodel.h"
#include <xnamath.h>

class Model
{

private:
	ID3D11Device*           	m_pD3DDevice;
	ID3D11DeviceContext*    	m_pImmediateContext;

	ObjFileModel*		m_pObject;
	ID3D11VertexShader*	m_pVShader;
	ID3D11PixelShader*	m_pPShader;
	ID3D11InputLayout*	m_pInputLayout;
	ID3D11Buffer*		m_pConstantBuffer;
	ID3D11ShaderResourceView*	m_pTexture0;
	ID3D11SamplerState*	m_pSampler0;

	float m_x, m_y, m_z;
	float m_dx, m_dz, m_rotation;
	float m_xangle, m_zangle, m_yangle;
	float m_scale;

	float m_bounding_sphere_centre_x, m_bounding_sphere_centre_y, m_bounding_sphere_centre_z, m_bounding_sphere_radius;


public:
	Model(ID3D11Device* device, ID3D11DeviceContext* context);
	~Model();
	int LoadObjModel(char* filename);
	void draw(XMMATRIX *world, XMMATRIX *view, XMMATRIX *projection);
	void setPosition(float x, float y, float z);
	void setRotation(float x, float y, float z);
	void setScale(float value);
	float getPositionX();
	float getPositionY();
	float getPositionZ();
	float getRotation();
	float getScale();
	void incPositionX(float value);
	void incPositionY(float value);
	void incPositionZ(float value);
	void LookAt_XZ(float x, float z);
	void MoveForward(float distance);
	void CalculateModelCenterPoint();
	void CalculateBoundingSphereRadius();
	XMVECTOR GetBoundingSphereWorldSpacePosition();
	float GetBoundingSphereRadius();
	bool CheckCollision(Model* a_model);

	void SetTexture(ID3D11ShaderResourceView*	m_pTexture0);
	void SetSampler(ID3D11SamplerState*	m_pSampler0);
};