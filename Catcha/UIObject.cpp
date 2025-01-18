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
		DirectX::XMMATRIX scale_matrix = DirectX::XMMatrixScaling(m_scale.x, m_scale.y, 1.0f);

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
