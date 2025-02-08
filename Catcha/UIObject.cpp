#include "UIObject.h"

UIObject::UIObject(float position_x, float position_y, float scale_x, float scale_y,
	UINT texture_width, UINT texture_height, float top, float left, float bottom, float right
) {
	m_position = DirectX::XMFLOAT3(position_x, position_y, 0.0f);
	m_scale = DirectX::XMFLOAT3(scale_x, scale_y, 1.0f);

	Set_Texture_WH(texture_width, texture_height);
	Set_TLBR(top, left, bottom, right);
}

void UIObject::Update(float elapsed_time) {
	if (m_dirty) {
		DirectX::XMMATRIX translate_matrix = DirectX::XMMatrixTranslation(m_position.x, m_position.y, 0.0f);
		DirectX::XMMATRIX scale_matrix = DirectX::XMMatrixScaling(
			m_scale.x + m_additional_scale.x, m_scale.y + m_additional_scale.y, 1.0f);

		DirectX::XMStoreFloat4x4(&m_world_matrix, scale_matrix * translate_matrix);

		m_world_matrix._14 = m_top_left_bottom_right.x;
		m_world_matrix._24 = m_top_left_bottom_right.y;
		m_world_matrix._34 = m_top_left_bottom_right.z;
		m_world_matrix._44 = m_top_left_bottom_right.w;

		Rst_Dirty_Count();
	}
}

void UIObject::Set_Texture_WH(UINT texture_width, UINT texture_height) {
	m_additional_info = DirectX::XMFLOAT3((float)texture_width, (float)texture_height, 0.0f);

	m_dirty = true;
}

void UIObject::Set_TLBR(float top, float left, float bottom, float right) {
	m_top_left_bottom_right = DirectX::XMFLOAT4(top, left, bottom, right);

	m_dirty = true;
}

//bool UIObject::Picking(DirectX::XMFLOAT4 origin_ray, DirectX::XMFLOAT4 ray_direction, DirectX::XMFLOAT4X4 inverse_view_matrix, float picking_distance) {
//	DirectX::XMMATRIX world_matrix = XMFLOAT4X4_2_XMMATRIX(m_world_matrix);
//	DirectX::XMMATRIX inverse_world_matrix = DirectX::XMMatrixInverse(&DirectX::XMMatrixDeterminant(world_matrix), world_matrix);
//
//	DirectX::XMMATRIX to_local_matrix = XMFLOAT4X4_2_XMMATRIX(MathHelper::Multiply(inverse_view_matrix, inverse_world_matrix));
//
//	DirectX::XMVECTOR origin_ray_vector = DirectX::XMVector3TransformCoord(DirectX::XMLoadFloat4(&origin_ray), to_local_matrix);
//	DirectX::XMVECTOR ray_direction_vector = DirectX::XMVector3TransformCoord(DirectX::XMLoadFloat4(&ray_direction), to_local_matrix);
//
//	ray_direction_vector = DirectX::XMVector3Normalize(ray_direction_vector);
//
//	for (auto& m : m_meshes) {
//		std::vector<Vertex_Info> vertices = m.mesh_info->vertices;
//		std::vector<uint32_t> indices = m.mesh_info->indices_32;
//		UINT triangle_count = (UINT)indices.size() / 3;
//
//		for (UINT i = 0; i < triangle_count; ++i) {
//			UINT index_0 = indices[i * 3 + 0];
//			UINT index_1 = indices[i * 3 + 1];
//			UINT index_2 = indices[i * 3 + 2];
//
//			DirectX::XMVECTOR vertex_0 = DirectX::XMLoadFloat3(&vertices[index_0].position);
//			DirectX::XMVECTOR vertex_1 = DirectX::XMLoadFloat3(&vertices[index_1].position);
//			DirectX::XMVECTOR vertex_2 = DirectX::XMLoadFloat3(&vertices[index_2].position);
//
//			float distance = 0.0f;
//			if (DirectX::TriangleTests::Intersects(origin_ray_vector, ray_direction_vector, vertex_0, vertex_1, vertex_2, distance)) {
//				if (distance < picking_distance) {
//					return true;
//				}
//			}
//		}
//	}
//
//	return false;
//}

bool UIObject::Picking(DirectX::XMFLOAT4 origin_ray, DirectX::XMFLOAT4 ray_direction, DirectX::XMFLOAT4X4 inverse_view_matrix, float picking_distance) {
	DirectX::XMFLOAT3 left_top = DirectX::XMFLOAT3(-0.5f, 0.5f, 0.0f);
	DirectX::XMFLOAT3 right_bottom = DirectX::XMFLOAT3(0.5f, -0.5f, 0.0f);

	left_top = MathHelper::Multiply(left_top, m_world_matrix);
	right_bottom = MathHelper::Multiply(right_bottom, m_world_matrix);

	if ((left_top.x < ray_direction.x && ray_direction.x < right_bottom.x) &&
		(right_bottom.y < ray_direction.y && ray_direction.y < left_top.y)) {

		return true;
	}

	return false;
}
