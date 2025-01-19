#include "ParticleObject.h"

ParticleObject::ParticleObject() {
	m_instance_max_count = MAX_PARTICLE_COUNT;
	m_instance_count = 0;
}

void ParticleObject::Update(float total_time) {
	if (m_dirty) {
		auto target_iterator = std::remove_if(m_instance_data_array.begin(), m_instance_data_array.end(),
			[total_time](const InstanceData& data) {
				return total_time - data.additional_info.w > MAX_PARTICLE_LIFE_TIME;
			});

		m_instance_data_array.erase(target_iterator, m_instance_data_array.end());

		m_instance_count = (UINT)m_instance_data_array.size();

		Rst_Dirty_Count();
	}
}

void ParticleObject::Add_Particle(DirectX::XMFLOAT3 position, DirectX::XMFLOAT3 scale, DirectX::XMFLOAT4 color, UINT count, float now_time) {
	std::random_device rd;
	std::uniform_real_distribution<float> urd(-100.0f, 100.0f);

	DirectX::XMFLOAT4X4 world_matrix = XMMATRIX_2_XMFLOAT4X4(DirectX::XMMatrixTranspose(
		DirectX::XMMatrixScaling(scale.x, scale.y, scale.z) * DirectX::XMMatrixTranslation(position.x, position.y, position.z)));

	world_matrix._41 = color.x;
	world_matrix._42 = color.y;
	world_matrix._43 = color.z;
	world_matrix._44 = color.w;

	for (UINT i = 0; i < count; ++i) {
		DirectX::XMFLOAT3 random_direction = DirectX::XMFLOAT3(urd(rd), urd(rd), urd(rd));

		InstanceData instance_data;
		instance_data.world_matrix = world_matrix;
		instance_data.additional_info = DirectX::XMFLOAT4(
			random_direction.x, random_direction.y, random_direction.z, now_time);

		Add_Instance_Data(instance_data);
	}

	m_instance_count = (UINT)m_instance_data_array.size();

	m_dirty = true;
}
