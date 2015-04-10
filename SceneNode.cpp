#include "SceneNode.h"

SceneNode::SceneNode()
{
	m_p_model = NULL;

	m_x, m_y, m_z = 0.0f;
	m_dx, m_dz, m_rotation = 0.0f;
	m_xangle, m_zangle, m_yangle = 0.0f;
	m_scale = 1.0f;


}

void SceneNode::setModel(Model* aModel)
{
	m_p_model = aModel;
}

void SceneNode::setPosition(float x, float y, float z)
{
	m_x = x;
	m_y = y;
	m_z = z;
}
void SceneNode::setRotation(float x, float y, float z)
{
	m_xangle = x;
	m_zangle = y;
	m_yangle = z;
}
void SceneNode::setScale(float value)
{
	m_scale = value;
}
float SceneNode::getPositionX()
{
	return m_x;
}
float SceneNode::getPositionY()
{
	return m_y;
}
float SceneNode::getPositionZ()
{
	return m_z;
}
float SceneNode::getScale()
{
	return m_scale;
}

void SceneNode::addChildNode(SceneNode *n)
{
	m_children.push_back(n);
}

bool SceneNode::detatchNode(SceneNode*n)
{
	// traverse tree to find node to detatch
	for (int i = 0; i < m_children.size(); i++)
	{
		if (n == m_children[i])
		{
			m_children.erase(m_children.begin() + i);
			return true;
		}
		if (m_children[i]->detatchNode(n) == true) return true;
	}
	return false; // node not in this tree
}

void SceneNode::execute(XMMATRIX *world, XMMATRIX* view, XMMATRIX* projection)
{
	// the local_world matrix will be used to calc the local transformations for this node
	XMMATRIX local_world = XMMatrixIdentity();

	local_world = XMMatrixRotationX(XMConvertToRadians(m_xangle));
	local_world *= XMMatrixRotationY(XMConvertToRadians(m_yangle));
	local_world *= XMMatrixRotationZ(XMConvertToRadians(m_zangle));

	local_world *= XMMatrixScaling(m_scale, m_scale, m_scale);

	local_world *= XMMatrixTranslation(m_x, m_y, m_z);

	// the local matrix is multiplied by the passed in world matrix that contains the concatenated
	// transformations of all parent nodes so that this nodes transformations are relative to those
	local_world *= *world;

	// only draw if there is a model attached
	if (m_p_model) m_p_model->draw(&local_world, view, projection);

	// traverse all child nodes, passing in the concatenated world matrix
	for (int i = 0; i< m_children.size(); i++)
	{
		m_children[i]->execute(&local_world, view, projection);
	}
}

