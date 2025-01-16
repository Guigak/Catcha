#include "VoxelCheese.h"

VoxelCheese::VoxelCheese(float position_x, float position_y, float position_z, float scale, UINT detail_level) {
	m_detail_level = detail_level;
	m_instance_max_count = CHEESE_VOXEL_COUNT * (m_detail_level * 8 ? m_detail_level * 8 : 1);

	Rst_Voxel(position_x, position_y, position_z, scale, m_detail_level);
	//Remove_Random_Voxel();
}

void VoxelCheese::Get_Instance_Data(std::vector<InstanceDatas>& instance_data_array) {
	for (UINT i = 0; i < m_instance_max_count; ++i) {
		if (m_instance_data_array[i].additional_info.x) {
			instance_data_array.emplace_back(m_instance_data_array[i]);
		}
	}
}

void VoxelCheese::Rst_Voxel(float position_x, float position_y, float position_z, float scale, UINT detail_level) {
	m_instance_data_array.clear();
	m_instance_data_array.shrink_to_fit();

	DirectX::XMFLOAT3 position = DirectX::XMFLOAT3(position_x, position_y, position_z);
	DirectX::XMFLOAT3 pivot_position = position;

	position.y += scale / 2.0f;

	for (int i = 0; i < VOXEL_CHEESE_HEIGHT; ++i) {
		position.z = pivot_position.z - scale * (float)(VOXEL_CHEESE_DEPTH / 2);

		for (int j = 1; j <= VOXEL_CHEESE_DEPTH; ++j) {
			position.x = pivot_position.x - scale * (float)(VOXEL_CHEESE_WIDTH / 2);

			for (int k = 0; k <= j / 2; ++k) {
				Add_Voxel(position.x, position.y, position.z, scale, detail_level);

				position.x += scale;
			}

			position.z += scale;
		}

		position.y += scale;
	}

	m_instance_count = CHEESE_VOXEL_COUNT * (m_detail_level * 8 ? m_detail_level * 8 : 1);	// == CHEESE_VOXEL_COUNT

	Rst_Dirty_Count();
}

void VoxelCheese::Add_Voxel(float position_x, float position_y, float position_z, float scale, UINT detail_level) {
	if (detail_level == 0) {
		Add_Instance_Data(InstanceDatas(
			DirectX::XMFLOAT4X4(
				scale, 0.0f, 0.0f, position_x,
				0.0f, scale, 0.0f, position_y,
				0.0f, 0.0f, scale, position_z,
				0.0f, 0.0f, 0.0f, 1.0f),
			DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 0.0f)));
	}
	else {
		float half = scale / 2.0f;
		float quarter = scale / 4.0f;

		for (int i = -1; i < 2; ++++i) {
			for (int j = -1; j < 2; ++++j) {
				for (int k = -1; k < 2; ++++k) {
					Add_Voxel(
						position_x + quarter * k, position_y + quarter * i, position_z + quarter * j,
						half, detail_level - 1);
				}
			}
		}
	}
}

void VoxelCheese::Remove_Voxel(int voxel_index) {
	m_instance_data_array[voxel_index].additional_info.x = 0.0f;

	--m_instance_count;

	Rst_Dirty_Count();
}

void VoxelCheese::Remove_Random_Voxel(int random_seed) {
	int m_random_value = 10;

	int count = 0;

	std::mt19937 generator(random_seed);
	std::uniform_int_distribution<int> uid(1, m_random_value);

	for (int i = 0; i < VOXEL_CHEESE_HEIGHT; ++i) {
		for (int j = 1; j <= VOXEL_CHEESE_DEPTH; ++j) {
			for (int k = 0; k <= j / 2; ++k) {
				if ((i == VOXEL_CHEESE_HEIGHT - 1 || j == VOXEL_CHEESE_DEPTH || k == 0 || k == j / 2) &&
					!(uid(generator) % m_random_value)) {
					wchar_t debugMessage[256];
					swprintf_s(debugMessage, L"random value : %d\n", uid(generator));
					OutputDebugStringW(debugMessage);
					for (UINT l = 0; l <= (m_detail_level * 8 ? m_detail_level * 8 - 1 : 0); ++l) {
						Remove_Voxel(count * (m_detail_level * 8 ? m_detail_level * 8 : 1) + l);
					}
				}

				++count;
			}
		}
	}
}

void VoxelCheese::Remove_Sphere_Voxel(const DirectX::XMFLOAT3& center, float radius)
{
	DirectX::BoundingSphere sphere(center, radius);
	for(auto& voxel : m_instance_data_array)
	{
		if (0.0f != voxel.additional_info.x)
		{
			if(sphere.Contains(
				DirectX::XMLoadFloat3(
					&DirectX::XMFLOAT3(
						voxel.world_matrix._14, 
						voxel.world_matrix._24, 
						voxel.world_matrix._34)
				)))
			{
				voxel.additional_info.x = 0.0f;
				--m_instance_count;
				Rst_Dirty_Count();
			}
		}
	}
}

