#include "TextUIObject.h"

TextUIObject::TextUIObject(float position_x, float position_y, float scale_x, float scale_y) {
	m_instance_max_count = TEXT_MAX_COUNT;

	m_position = DirectX::XMFLOAT3(position_x, position_y, 0.0f);
	m_scale = DirectX::XMFLOAT3(scale_x, scale_y * CLIENT_ASPECT_RATIO, 0.0f);

	Set_Text(L"text");
}

void TextUIObject::Update(float elapsed_time) {
	if (m_dirty) {
		UINT count = 0;

		m_instance_data_array.resize(m_text.size());

		for (size_t i = 0; i < m_text.size(); ++i) {
			float position_x = m_position.x /*+ m_scale.x / 2.0f*/ + (m_scale.x + m_additional_scale.x) * TEXT_WIDTH_MULTIPLIER * (float)i;
			float position_y = m_position.y;

			m_instance_data_array[i].world_matrix = XMMATRIX_2_XMFLOAT4X4(DirectX::XMMatrixTranspose(
				DirectX::XMMatrixScaling(m_scale.x + m_additional_scale.x, m_scale.y + m_additional_scale.y, 1.0f) *
				DirectX::XMMatrixTranslation(position_x, position_y, 0.1f)));

			int index = (int)m_text[i];

			int index_x = index / 4096 * 16 + index % 16;
			int index_y = index % 4096 / 16;

			m_instance_data_array[i].additional_info = DirectX::XMFLOAT4(
				(float)index_x, (float)index_y, 0.0f, 0.0f);

			++count;
		}

		m_instance_count = count;

		Rst_Instc_Dirty_Cnt();
	}
}

void TextUIObject::Set_Text(std::wstring text) {
	m_text = text;

	m_dirty = true;
}

bool TextUIObject::Picking(DirectX::XMFLOAT4 origin_ray, DirectX::XMFLOAT4 ray_direction, DirectX::XMFLOAT4X4 inverse_view_matrix, float picking_distance) {
	DirectX::XMVECTOR left_top = DirectX::XMVectorSet(-0.5f, 0.5f, 0.0f, 0.0f);
	DirectX::XMVECTOR right_bottom = DirectX::XMVectorSet(0.5f, -0.5f, 0.0f, 0.0f);

	DirectX::XMFLOAT3 instance_left_top;
	DirectX::XMFLOAT3 instance_right_bottom;
	DirectX::XMMATRIX instance_world_matrix;

	for (auto& i : m_instance_data_array) {
		instance_world_matrix = MathHelper::Transpose(XMFLOAT4X4_2_XMMATRIX(i.world_matrix));
		DirectX::XMStoreFloat3(&instance_left_top, DirectX::XMVector3Transform(left_top, instance_world_matrix));
		DirectX::XMStoreFloat3(&instance_right_bottom, DirectX::XMVector3Transform(right_bottom, instance_world_matrix));

		if ((instance_left_top.x < ray_direction.x && ray_direction.x < instance_right_bottom.x) &&
			(instance_right_bottom.y < ray_direction.y && ray_direction.y < instance_left_top.y)) {

			return true;
		}
	}

	return false;
}
