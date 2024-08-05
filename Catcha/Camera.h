#pragma once
#include "common.h"

class Camera {
private:
	DirectX::XMFLOAT3 m_position = { 0.0f, 0.0f, 0.0f };

	DirectX::XMFLOAT3 m_look = { 0.0f, 0.0f, 1.0f };
	DirectX::XMFLOAT3 m_up = { 0.0f, 1.0f, 0.0f };
	DirectX::XMFLOAT3 m_right = { 1.0f, 0.0f, 0.0f };

	float m_near_z = 0.0f;
	float m_far_z = 0.0f;

	float m_aspect_ratio = 0.0f;
	float m_FOV_y = 0.0f;

	float m_near_window_height = 0.0f;
	float m_far_window_height = 0.0f;

	float m_dirty = true;

	DirectX::XMFLOAT4X4 m_view_matrix = MathHelper::Identity_4x4();
	DirectX::XMFLOAT4X4 m_projection_matrix = MathHelper::Identity_4x4();

public:
	Camera();
	~Camera() {}

	void Set_Position(float position_x, float position_y, float position_z);
	void Set_Position(DirectX::XMFLOAT3 position_vector);

	DirectX::XMVECTOR Get_Position_V() { return DirectX::XMLoadFloat3(&m_position); }	// Get Position Vector
	DirectX::XMFLOAT3 Get_Position_3f() { return m_position; }	// Get Position float3

	DirectX::XMVECTOR Get_Look_V() { return DirectX::XMLoadFloat3(&m_look); }
	DirectX::XMFLOAT3 Get_Look_3f() { return m_look; }

	DirectX::XMVECTOR Get_Up_V() { return DirectX::XMLoadFloat3(&m_up); }
	DirectX::XMFLOAT3 Get_Up_3f() { return m_up; }

	DirectX::XMVECTOR Get_Right_V() { return DirectX::XMLoadFloat3(&m_right); }
	DirectX::XMFLOAT3 Get_Right_3f() { return m_right; }

	float Get_Near_Z() { return m_near_z; }
	float Get_Far_Z() { return m_far_z; }

	float Get_Aspect_Ratio() { return m_aspect_ratio; }
	float Get_FOV_Y() { return m_FOV_y; }
	float Get_FOV_X() { return 2.0f * atan((0.5f * m_near_window_height) / m_near_z); }

	float Get_Near_Window_Width() { return m_aspect_ratio * m_near_window_height; }
	float Get_Near_Window_Height() { return m_near_window_height; }
	float Get_Far_Window_Width() { return m_aspect_ratio * m_far_window_height; }
	float Get_Far_Window_Height() { return m_far_window_height; }

	DirectX::XMMATRIX Get_VM_M() { return DirectX::XMLoadFloat4x4(&m_view_matrix); }	// Get View Matrix MATRIX
	DirectX::XMFLOAT4X4 Get_VM_4x4f() { return m_view_matrix; }	// Get View Matrix float4x4

	DirectX::XMMATRIX Get_PM_M() { return DirectX::XMLoadFloat4x4(&m_projection_matrix); }	// Get Projection Matrix MATRIX
	DirectX::XMFLOAT4X4 Get_PM_4x4f() { return m_projection_matrix; }	// Get Projection Matrix float4x4

	void Set_Frustum(float FOV_y, float aspect_ratio, float near_z, float far_z);

	void Look_At(DirectX::FXMVECTOR position, DirectX::FXMVECTOR target, DirectX::FXMVECTOR up_vector);
	void Look_At(const DirectX::XMFLOAT3 position, const DirectX::XMFLOAT3 target, const DirectX::XMFLOAT3 up_vector);

	void Udt_VM();	// Update View Matrix
};
