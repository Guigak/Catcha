#include "VoxelCheese.h"

VoxelCheese::VoxelCheese(float position_x, float position_y, float position_z, float scale) {
	m_instance_count = CHEESE_VOXEL_COUNT;

	Rst_Voxel(position_x, position_y, position_z, scale);
	//Remove_Random_Voxel();
}

void VoxelCheese::Update(float elapsed_time) {
	// nothing?
}

void VoxelCheese::Draw(ID3D12GraphicsCommandList* command_list) {
	for (auto& m : m_meshes) {
		Mesh_Info* mesh_info = m.mesh_info;

		command_list->IASetVertexBuffers(0, 1, &mesh_info->Get_VBV());
		command_list->IASetIndexBuffer(&mesh_info->Get_IBV());
		command_list->IASetPrimitiveTopology(mesh_info->primitive_topology);

		command_list->DrawIndexedInstanced(mesh_info->index_count, m_instance_count, 0, 0, 0);
	}
}

//std::array<InstanceDatas, CHEESE_VOXEL_COUNT>& VoxelCheese::Get_Instance_Data() {
//	std::array<InstanceDatas, CHEESE_VOXEL_COUNT> instance_data;
//
//	int count = 0;
//
//	for (int i = 0; i < CHEESE_VOXEL_COUNT; ++i) {
//		if (m_voxel_info_array[i].draw) {
//			instance_data[count++].world_matrix = m_voxel_info_array[i].world_matrix;
//		}
//	}
//
//	return instance_data;
//}

void VoxelCheese::Get_Instance_Data(InstanceDatas* instance_data_pointer) {
	instance_data_pointer = new InstanceDatas[CHEESE_VOXEL_COUNT];

	int count = 0;

	for (int i = 0; i < CHEESE_VOXEL_COUNT; ++i) {
		if (m_voxel_info_array[i].draw) {
			instance_data_pointer[count++].world_matrix = m_voxel_info_array[i].world_matrix;
		}
	}
}

void VoxelCheese::Get_Instance_Data(std::vector<InstanceDatas>& instance_data_array) {
	int count = 0;

	for (int i = 0; i < CHEESE_VOXEL_COUNT; ++i) {
		if (m_voxel_info_array[i].draw) {
			instance_data_array.emplace_back(InstanceDatas(m_voxel_info_array[i].world_matrix));
		}
	}
}

void VoxelCheese::Rst_Voxel(float position_x, float position_y, float position_z, float scale) {
	int count = 0;

	DirectX::XMFLOAT3 position = DirectX::XMFLOAT3(position_x, position_y, position_z);
	DirectX::XMFLOAT3 pivot_position = position;

	position.y += scale / 2.0f;

	for (int i = 0; i < VOXEL_CHEESE_HEIGHT; ++i) {
		position.z = pivot_position.z - scale * (float)(VOXEL_CHEESE_DEPTH / 2);

		for (int j = 1; j <= VOXEL_CHEESE_DEPTH; ++j) {
			position.x = pivot_position.x - scale * (float)(VOXEL_CHEESE_WIDTH / 2);

			for (int k = 0; k <= j / 2; ++k) {
				m_voxel_info_array[count].world_matrix = XMMATRIX_2_XMFLOAT4X4(
					DirectX::XMMatrixTranspose(
					DirectX::XMMatrixScaling(scale, scale, scale) * DirectX::XMMatrixTranslation(position.x, position.y, position.z)));
				position.x += scale;

				++count;
			}

			position.z += scale;
		}

		position.y += scale;
	}

	m_index_count = count;	// == CHEESE_VOXEL_COUNT

	Rst_Dirty_Count();
}

void VoxelCheese::Remove_Voxel(int voxel_index) {
	m_voxel_info_array[voxel_index].draw = false;

	--m_instance_count;

	Rst_Dirty_Count();
}

void VoxelCheese::Remove_Random_Voxel() {
	int m_random_value = 10;

	int count = 0;

	std::random_device rd;
	std::uniform_int_distribution<int> uid(1, m_random_value);

	for (int i = 0; i < VOXEL_CHEESE_HEIGHT; ++i) {
		for (int j = 1; j <= VOXEL_CHEESE_DEPTH; ++j) {
			for (int k = 0; k <= j / 2; ++k) {
				if ((i == VOXEL_CHEESE_HEIGHT - 1 || j == VOXEL_CHEESE_DEPTH || k == 0 || k == j / 2) &&
					!(uid(rd) % m_random_value)) {
					Remove_Voxel(count);
				}

				++count;
			}
		}
	}
}

void VoxelCheese::Remove_Sphere_Voxel(const DirectX::XMFLOAT3& center, float radius)
{
	DirectX::BoundingSphere sphere(center, radius);
	for(auto& voxel : m_voxel_info_array)
	{
		if (true == voxel.draw)
		{
			if(sphere.Contains(
				DirectX::XMLoadFloat3(
					&DirectX::XMFLOAT3(
						voxel.world_matrix._14, 
						voxel.world_matrix._24, 
						voxel.world_matrix._34)
				)))
			{
				voxel.draw = false;
				--m_instance_count;
				Rst_Dirty_Count();
			}
		}
	}
}

