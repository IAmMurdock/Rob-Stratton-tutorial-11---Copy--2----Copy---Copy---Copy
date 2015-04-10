#include <windows.h>
#include <d3d11.h>
#include <d3dx11.h>
#include <dxerr.h>
#define _XM_NO_INTRINSICS_
#define XM_NO_ALIGNMENT
#include <xnamath.h>

class camera
{
private:
	float m_x, m_y, m_z, m_dx, m_dz, m_camera_rotation;


	XMVECTOR m_position, m_lookat, m_up;

public:
	camera(float x, float y, float z, float camera_rotation);

	void rotate(float degrees);
	void forward(float distance);
	void up(float distance);
	void side(float distance);
	XMMATRIX getviewmatrix();
	float GetX();
	float GetY();
	float GetZ();

	
	
};