#include "DXUT.h"
#include "camera.h"

camera::camera(void)
{
}

camera::~camera(void)
{
}

camera::camera(D3DXVECTOR3* position, D3DXVECTOR3* lookat, D3DXVECTOR3* up)
{
	m_position = *position;
	m_lookat = *lookat;
	m_up = *up;
}

void camera::move(float dir)
{
	D3DXVECTOR3 direction;
	direction.x = m_lookat.x - m_position.x;
	direction.y = m_lookat.y - m_position.y;
	direction.z = m_lookat.z - m_position.z;
	D3DXVec3Normalize(&direction, &direction);
	m_position += direction*dir;
	m_lookat += direction*dir;
}

void camera::strafe(float dir)
{
	D3DXVECTOR3 direction;
	direction.x = m_lookat.x - m_position.x;
	direction.y = m_lookat.y - m_position.y;
	direction.z = m_lookat.z - m_position.z;
	D3DXVec3Normalize(&direction, &direction);
	D3DXVec3Cross(&direction, &direction, &m_up);
	m_position += direction*dir;
	m_lookat += direction*dir;
}

void camera::rotate(int x, int y)
{
	if (x == 0 && y == 0) return;
	float _x = (float)x;
	float _y = (float)y;
	_x /= 1000;
	_y /= 1000;

	D3DXVECTOR3 direction;
	direction.x = m_lookat.x - m_position.x;
	direction.y = m_lookat.y - m_position.y;
	direction.z = m_lookat.z - m_position.z;
	D3DXVec3Normalize(&direction, &direction);
	D3DXVec3Cross(&direction, &direction, &m_up);
	
	rotate(_y, direction);
	rotate(-_x, m_up);
}

void camera::rotate(float angle, D3DXVECTOR3 axis)
{
	D3DXVECTOR3 direction;
	direction.x = m_lookat.x - m_position.x;
	direction.y = m_lookat.y - m_position.y;
	direction.z = m_lookat.z - m_position.z;
	D3DXVec3Normalize(&direction, &direction);

	D3DXMATRIX mY;
	D3DXMatrixRotationAxis(&mY, &axis, angle);
	D3DXVECTOR3 vY;
	D3DXVec3TransformCoord(&vY, &direction, &mY);

	m_lookat = m_position + vY;
}