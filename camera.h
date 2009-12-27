#pragma once

class camera
{
public:
	camera(void);
	~camera(void);

	camera(D3DXVECTOR3* position, D3DXVECTOR3* lookat, D3DXVECTOR3* up);

	void strafe(float dir);
	void move(float dir);
	void rotate(int x, int y);
	void rotate(float angle, D3DXVECTOR3 axis);

	D3DXVECTOR3 m_position;
	D3DXVECTOR3 m_lookat;
	D3DXVECTOR3 m_up;

	D3DXMATRIX m_rotation;
};
