#include "Camera.h"
#include "MapData.h"

std::vector<DirectX::BoundingOrientedBox> g_springArmObj;

Camera::Camera() {
	Set_Frustum(0.25f * MathHelper::Pi(), 1.0f, 1.0f, 1000.0f);
}

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
	DirectX::XMVectorSetY(position_, position.y/* - 39.3701f / 2.0f*/);
	DirectX::XMVECTOR target_ = DirectX::XMLoadFloat3(&target);
	DirectX::XMVECTOR up_ = DirectX::XMLoadFloat3(&up_vector);

	// 현재 타겟 위치에서 카메라 위치까지 거리 계산
	float length = DirectX::XMVectorGetX(DirectX::XMVector3Length(DirectX::XMVectorSubtract(position_, target_)));

	DirectX::XMVECTOR direction = DirectX::XMVector3Normalize(DirectX::XMVectorSubtract(position_, target_));

	// 카메라 스프링 암 구현
	for (const auto& obj : g_springArmObj) {
		float dist = 0.0f;
		if (obj.Intersects(target_, direction, dist)) {
			// 충돌 거리 확인
			if (dist <= length && dist >= 0.0f) {
				// 충돌 지점에서 약간 떨어진 지점으로 position 갱신위해 거리 추가 계산
				float offset = 1.0f;
				float safeDist = dist - offset;
				// 최소 거리 설정
				if (safeDist < 1.0f)
				{
					safeDist = 1.0f;
				}

				// 새로운 카메라 위치 갱신
				position_ = DirectX::XMVectorAdd(target_, DirectX::XMVectorScale(direction, safeDist));

				// 다른 물체 체크 위한 방향과 최소 거리 계산위해 재조정
				direction = DirectX::XMVector3Normalize(DirectX::XMVectorSubtract(position_, target_));
				length = DirectX::XMVectorGetX(DirectX::XMVector3Length(DirectX::XMVectorSubtract(position_, target_)));
			}
		}
	}

	Look_At(position_, target_, up_);
}

void Camera::Update(float elapsed_time) {
	if (m_dirty) {
		if (m_limit_rotate_right) {
			m_rotate_right = MathHelper::Min(m_limit_max_right, MathHelper::Max(m_rotate_right, m_limit_min_right));
		}

		if (m_limit_rotate_look) {
			m_rotate_look = MathHelper::Min(m_limit_max_look, MathHelper::Max(m_rotate_look, m_limit_min_look));
		}
	}

	if (m_object) {
		if (m_freezing_time > 0.001f)
		{
			m_freezing_time -= elapsed_time;
		}
		else
		{
			Calc_Rotate();
			Udt_LUR();

			m_bind_position = m_object->Get_Position_3f();
			m_bind_position = MathHelper::Add(m_bind_position, m_object->Get_Look(), m_bind_offset_look);
			m_bind_position = MathHelper::Add(m_bind_position, m_object->Get_Up(), m_bind_offset_up);
			m_bind_position = MathHelper::Add(m_bind_position, m_object->Get_Right(), m_bind_offset_right);

			m_target_position = MathHelper::Add(m_bind_position, Get_Look(), -m_distance);
			//m_position = MathHelper::Add(m_position, MathHelper::Multiply(MathHelper::Subtract(m_target_position, m_position), 1.0f / m_lagging_degree));
			DirectX::XMStoreFloat3(&m_position, DirectX::XMVectorLerp(DirectX::XMLoadFloat3(&m_position), DirectX::XMLoadFloat3(&m_target_position), 1.0f / m_lagging_degree));
		}
		m_dirty = true;
	}

	if (m_dirty) {
		Calc_Rotate();
		Udt_WM();
		Udt_LUR();

		if (m_object) {
			Look_At(Get_Position_3f(), m_bind_position, DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f));
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
	right = DirectX::XMVector3Normalize(DirectX::XMVector3Cross(up, look));

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

void Camera::Bind_Obj(Object* object, float offset_look, float offset_up, float offset_right, float distance) {
	m_object = object;
	m_bind_offset_look = offset_look;
	m_bind_offset_up = offset_up;
	m_bind_offset_right = offset_right;
	m_distance = distance;
}

void Camera::Set_Limit_Rotate_Right(bool limit_rotate, float degree_min, float degree_max) {
	m_limit_rotate_right = limit_rotate;
	m_limit_min_right = degree_min;
	m_limit_max_right = degree_max;
}

void Camera::Set_Limit_Rotate_Look(bool limit_rotate, float degree_min, float degree_max) {
	m_limit_rotate_look = limit_rotate;
	m_limit_min_look = degree_min;
	m_limit_max_look = degree_max;
}

void Camera::Set_Target_Position(DirectX::XMFLOAT3 target_position) {
	m_target_position = target_position;
}

void Camera::Set_Target_Position(float target_position_x, float target_position_y, float target_position_z) {
	m_target_position = DirectX::XMFLOAT3(target_position_x, target_position_y, target_position_z);
}
