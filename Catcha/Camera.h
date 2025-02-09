#pragma once
#include "common.h"
#include "Object.h"

class Camera : public Object {
private:
	float m_near_z = 0.0f;
	float m_far_z = 0.0f;

	float m_aspect_ratio = 0.0f;
	float m_FOV_y = 0.0f;

	float m_near_window_height = 0.0f;
	float m_far_window_height = 0.0f;

	DirectX::XMFLOAT4X4 m_view_matrix = MathHelper::Identity_4x4();
	DirectX::XMFLOAT4X4 m_projection_matrix = MathHelper::Identity_4x4();

	Object* m_object = nullptr;
	float m_distance = 0.0f;

	float m_bind_offset_look = 0.0f;
	float m_bind_offset_up = 0.0f;
	float m_bind_offset_right = 0.0f;

	DirectX::XMFLOAT3 m_bind_position = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);

	//
	bool m_limit_rotate_right = false;
	float m_limit_min_right = 0.0f;
	float m_limit_max_right = 0.0f;

	bool m_limit_rotate_look = false;
	float m_limit_min_look = 0.0f;
	float m_limit_max_look = 0.0f;

	//
	DirectX::XMFLOAT3 m_target_position = { 0.0f, 0.0f, 0.0f };
	float m_lagging_degree = 1.0f;

	// 
	float m_freezing_time = 0.0f;

public:
	Camera();
	virtual ~Camera() {}

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
	void Set_Distance(float distance) { m_distance = distance; }
	float Get_Distance() { return m_distance; }

	Object* Get_Bind_Obj() { return m_object; }
	DirectX::XMFLOAT3 Get_Bind_Offset_LUR() { return DirectX::XMFLOAT3(m_bind_offset_look, m_bind_offset_up, m_bind_offset_right); }

	void Look_At(DirectX::FXMVECTOR position, DirectX::FXMVECTOR target, DirectX::FXMVECTOR up_vector);
	void Look_At(const DirectX::XMFLOAT3 position, const DirectX::XMFLOAT3 target, const DirectX::XMFLOAT3 up_vector);

	virtual void Update(float elapsed_time);

	void Udt_VM();	// Update View Matrix

	//
	void Bind_Obj(Object* object, float offset_look, float offset_up, float offset_right, float distance);

	//
	void Set_Limit_Rotate_Right(bool limit_rotate, float degree_min = -RIGHT_ANGLE_RADIAN, float degree_max = RIGHT_ANGLE_RADIAN);
	void Set_Limit_Rotate_Look(bool limit_rotate, float degree_min = -RIGHT_ANGLE_RADIAN, float degree_max = RIGHT_ANGLE_RADIAN);

	//
	void Set_Target_Position(DirectX::XMFLOAT3 target_position);
	void Set_Target_Position(float target_position_x, float target_position_y, float target_position_z);

	void Set_Lagging_Degree(float lagging_degree) { m_lagging_degree = lagging_degree; }
	float Get_Lagging_Degree() { return m_lagging_degree; }

	void Set_Freezing_Time(float time) { m_freezing_time = time; }
};
