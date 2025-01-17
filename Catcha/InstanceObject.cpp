#include "InstanceObject.h"

void InstanceObject::Update(float elapsed_time) {
	// nothing?
}

void InstanceObject::Draw(ID3D12GraphicsCommandList* command_list) {
	if (m_instance_count == 0) {
		return;
	}

	for (auto& m : m_meshes) {
		Mesh_Info* mesh_info = m.mesh_info;

		command_list->IASetVertexBuffers(0, 1, &mesh_info->Get_VBV());
		command_list->IASetIndexBuffer(&mesh_info->Get_IBV());
		command_list->IASetPrimitiveTopology(mesh_info->primitive_topology);

		command_list->DrawIndexedInstanced(mesh_info->index_count, m_instance_count, 0, 0, 0);
	}
}

void InstanceObject::Get_Instance_Data(InstanceData* instance_data_pointer) {
	instance_data_pointer = new InstanceData[m_instance_count];

	//for (int i = 0; i < m_instance_count; ++i) {
	//	instance_data_pointer[i] = m_instance_data_array[i];
	//}

	std::memcpy(instance_data_pointer, m_instance_data_array.data(), m_instance_data_array.size() * sizeof(InstanceData));
}

void InstanceObject::Get_Instance_Data(std::vector<InstanceData>& instance_data_array) {
	instance_data_array.assign(m_instance_data_array.begin(), m_instance_data_array.end());
}

void InstanceObject::Add_Instance_Data(InstanceData& instance_data) {
	m_instance_data_array.emplace_back(instance_data);

	++m_instance_count;
	Rst_Instc_Dirty_Cnt();
}
