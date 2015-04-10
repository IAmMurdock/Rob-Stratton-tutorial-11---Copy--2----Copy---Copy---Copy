#include <windows.h>
#include <d3d11.h>
#include <d3dx11.h>
#include <dxerr.h>
#define _XM_NO_INTRINSICS_
#define XM_NO_ALIGNMENT
#include <xnamath.h>
#include <dinput.h>
#include <vector>
#include "Model.h"

class SceneNode
{
private:
	Model* m_p_model;
	vector<SceneNode*> m_children;
	float m_x, m_y, m_z;
	float m_dx, m_dz, m_rotation;
	float m_xangle, m_zangle, m_yangle;
	float m_scale;

public:
	SceneNode();
	void setPosition(float x, float y, float z);
	void setRotation(float x, float y, float z);
	void setScale(float value);
	void setModel(Model* aModel);
	float getPositionX();
	float getPositionY();
	float getPositionZ();
	float getRotation();
	float getScale();
	void addChildNode(SceneNode *n);
	bool detatchNode(SceneNode *n);
	void execute(XMMATRIX *world, XMMATRIX* view, XMMATRIX* projection);
};