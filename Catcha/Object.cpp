#include "Object.h"

Object::Object(std::wstring object_name, MeshInfo* mesh_info, std::wstring mesh_name,
	MaterialInfo* material_info, UINT constant_buffer_index, D3D12_PRIMITIVE_TOPOLOGY primitive_topology)
{
	Set_Name(object_name);
	Set_CB_Index(constant_buffer_index);
	Set_Mesh_Info(mesh_info, mesh_name);
	Set_Material_Info(material_info);
	Set_PT(primitive_topology);
}

void Object::Set_Mesh_Info(MeshInfo* mesh_info, std::wstring mesh_name) {
	m_mesh_info = mesh_info;
	m_index_count = m_mesh_info->submesh_map[mesh_name].index_count;
	m_start_index_location = m_mesh_info->submesh_map[mesh_name].start_index_location;
	m_base_vertex_location = m_mesh_info->submesh_map[mesh_name].base_vertex_location;
}

void Object::Chg_Mesh(std::wstring mesh_name) {
	m_index_count = m_mesh_info->submesh_map[mesh_name].index_count;
	m_start_index_location = m_mesh_info->submesh_map[mesh_name].start_index_location;
	m_base_vertex_location = m_mesh_info->submesh_map[mesh_name].base_vertex_location;
}

void Object::Update() {
	if (m_dirty) {
		DirectX::XMMATRIX trans_matrix = DirectX::XMMatrixTranslation(m_position.x, m_position.y, m_position.z);
		DirectX::XMMATRIX rotate_matrix = DirectX::XMMatrixRotationRollPitchYaw(
			DirectX::XMConvertToRadians(m_rotate.x), DirectX::XMConvertToRadians(m_rotate.y), DirectX::XMConvertToRadians(m_rotate.z));
		DirectX::XMMATRIX scale_matrix = DirectX::XMMatrixTranslation(m_scale.x, m_scale.y, m_scale.z);

		DirectX::XMStoreFloat4x4(&m_world_matrix, scale_matrix * rotate_matrix * trans_matrix);

		Rst_Dirty_Count();
	}
}

void Object::TP_Forward(float distance) {
	m_position = MathHelper::Add(Get_Position(), Get_Look(), distance);

	m_dirty = true;
}

void Object::TP_Back(float distance) {
	m_position = MathHelper::Add(Get_Position(), Get_Look(), -distance);

	m_dirty = true;
}

void Object::TP_Left(float distance) {
	m_position = MathHelper::Add(Get_Position(), Get_Right(), -distance);

	m_dirty = true;
}

void Object::TP_Right(float distance) {
	m_position = MathHelper::Add(Get_Position(), Get_Right(), distance);

	m_dirty = true;
}

void Object::TP_Up(float distance) {
	m_position = MathHelper::Add(Get_Position(), Get_Up(), distance);

	m_dirty = true;
}

void Object::TP_Down(float distance) {
	m_position = MathHelper::Add(Get_Position(), Get_Up(), distance);

	m_dirty = true;
}
