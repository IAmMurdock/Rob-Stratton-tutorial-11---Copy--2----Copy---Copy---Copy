#include "camera.h"


camera::camera(float x, float y, float z, float camera_rotation)
{
	m_x = x;
	m_y = y;
	m_z = z;
	m_camera_rotation = camera_rotation;
	

	m_dx = sin(m_camera_rotation*(XM_PI / 180.0));
	m_dz = cos(m_camera_rotation*(XM_PI / 180.0));
	
	m_position = XMVectorSet(m_x, m_y, m_z, 0.0);
	m_lookat = XMVectorSet(m_x + m_dx, m_y, m_z+m_dz, 0.0);
	m_up = XMVectorSet(0.0, 1.0, 0.0, 0.0);

	XMMATRIX view = XMMatrixLookAtLH(m_position, m_lookat, m_up);



}

void camera::rotate(float degrees)
{
	m_camera_rotation += degrees;
	m_dx = sin(m_camera_rotation*(XM_PI / 180.0));
	m_dz = cos(m_camera_rotation*(XM_PI / 180.0));
	//m_dx = sin((m_camera_rotation+ degrees)*(XM_PI / 180.0));
	//m_x += degrees*m_dx;

}

void camera::forward(float distance)
{

	m_x += distance *m_dx;
	m_z += distance *m_dz;

}

void camera::up(float distance)
{

	m_y += distance;

}

void camera::side(float distance)
{
	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	XMVECTOR forward = m_lookat - m_position;
	//XMVECTOR strafe = XMVectorSet(distance, 0.0, 0.0, 0.0);
	XMVECTOR strafe = XMVector3Cross(forward, up);

	//Todo - go left or right, depending on key press.
	m_x += distance *strafe.x;
	m_z += distance*strafe.z;

}

XMMATRIX camera::getviewmatrix()
{

	m_position = XMVectorSet(m_x, m_y, m_z, 0.0);
	m_lookat = XMVectorSet(m_x + m_dx, m_y, m_z + m_dz, 0.0);
	m_up = XMVectorSet(0.0, 1.0, 0.0, 0.0);

	XMMATRIX view = XMMatrixLookAtLH(m_position, m_lookat, m_up);

	return view;

}

float camera::GetX()
{
	return m_x;
}

float camera::GetY()
{
	return m_y;
}

float camera::GetZ()
{
	return m_z;
}
