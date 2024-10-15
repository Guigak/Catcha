#include "Camera.h"

Camera::Camera() {
	Set_Frustum(0.25f * MathHelper::Pi(), 1.0f, 1.0f, 1000.0f);
}

//void Camera::Set_Position(float position_x, float position_y, float position_z) {
//	m_position = DirectX::XMFLOAT3(position_x, position_y, position_z);
//	m_dirty = true;
//}
//
//void Camera::Set_Position(DirectX::XMFLOAT3 position_vector) {
//	m_position = position_vector;
//	m_dirty = true;
//}

void Camera::Set_Frustum(float FOV_y, float aspect_ratio, float near_z, float far_z) {
	m_FOV_y = FOV_y;
	m_aspect_ratio = aspect_ratio;
	m_near_z = near_z;
	m_far_z = far_z;

	m_near_window_height = 2.0f * m_near_z * tanf(0.5f * m_FOV_y);
	m_far_window_height = 2.0f * m_far_z * tanf(0.5f * m_FOV_y);

	DirectX::XMMATRIX projection_matrix = DirectX::XMMatrixPerspectiveFovLH(m_FOV_y, m_aspect_ratio, m_near_z, m_far_z);
	DirectX::XMStoreFloat4x4(&m_projection_matrix, projection_matrix);
}

void Camera::Look_At(DirectX::FXMVECTOR position, DirectX::FXMVECTOR target, DirectX::FXMVECTOR up_vector) {
	DirectX::XMVECTOR look = DirectX::XMVector3Normalize(DirectX::XMVectorSubtract(target, position));
	DirectX::XMVECTOR up = DirectX::XMVector3Normalize(up_vector);
	DirectX::XMVECTOR right = DirectX::XMVector3Cross(up, look);

	DirectX::XMStoreFloat3(&m_position, position);
	DirectX::XMStoreFloat3(&m_look, look);
	DirectX::XMStoreFloat3(&m_up, up);
	DirectX::XMStoreFloat3(&m_right, right);

	m_dirty = true;
}

void Camera::Look_At(const DirectX::XMFLOAT3 position, const DirectX::XMFLOAT3 target, const DirectX::XMFLOAT3 up_vector) {
	DirectX::XMVECTOR position_ = DirectX::XMLoadFloat3(&position);
	DirectX::XMVECTOR target_ = DirectX::XMLoadFloat3(&target);
	DirectX::XMVECTOR up_ = DirectX::XMLoadFloat3(&up_vector);

	Look_At(position_, target_, up_);
}

void Camera::Update(float elapsed_time) {
	if (m_object) {
		Udt_LUR();

		m_position = MathHelper::Add(m_object->Get_Position_3f(), Get_Look(), -m_distance);

		m_dirty = true;
	}

	if (m_dirty) {
		Udt_WM();
		Udt_LUR();

		if (m_object) {
			Look_At(Get_Position_3f(), m_object->Get_Position_3f(), Get_Up());
		}
		else {
			Look_At(Get_Position_3f(), MathHelper::Add(Get_Position_3f(), Get_Look(), 300.0f), Get_Up());
		}

		Rst_Dirty_Count();

		Udt_VM();

		m_dirty = false;
	}
}

void Camera::Udt_VM() {
	DirectX::XMVECTOR position = DirectX::XMLoadFloat3(&m_position);
	DirectX::XMVECTOR look = DirectX::XMLoadFloat3(&m_look);
	DirectX::XMVECTOR up = DirectX::XMLoadFloat3(&m_up);
	DirectX::XMVECTOR right = DirectX::XMLoadFloat3(&m_right);

	look = DirectX::XMVector3Normalize(look);
	up = DirectX::XMVector3Normalize(DirectX::XMVector3Cross(look, right));
	right = DirectX::XMVector3Cross(up, look);

	float x = -DirectX::XMVectorGetX(DirectX::XMVector3Dot(position, right));
	float y = -DirectX::XMVectorGetX(DirectX::XMVector3Dot(position, up));
	float z = -DirectX::XMVectorGetX(DirectX::XMVector3Dot(position, look));

	DirectX::XMStoreFloat3(&m_look, look);
	DirectX::XMStoreFloat3(&m_up, up);
	DirectX::XMStoreFloat3(&m_right, right);

	m_view_matrix(0, 0) = m_right.x;
	m_view_matrix(1, 0) = m_right.y;
	m_view_matrix(2, 0) = m_right.z;
	m_view_matrix(3, 0) = x;

	m_view_matrix(0, 1) = m_up.x;
	m_view_matrix(1, 1) = m_up.y;
	m_view_matrix(2, 1) = m_up.z;
	m_view_matrix(3, 1) = y;

	m_view_matrix(0, 2) = m_look.x;
	m_view_matrix(1, 2) = m_look.y;
	m_view_matrix(2, 2) = m_look.z;
	m_view_matrix(3, 2) = z;

	m_view_matrix(0, 3) = 0.0f;
	m_view_matrix(1, 3) = 0.0f;
	m_view_matrix(2, 3) = 0.0f;
	m_view_matrix(3, 3) = 1.0f;
}

void Camera::Bind_Obj(Object* object, float distance) {
	m_object = object;
	m_distance = distance;
}
