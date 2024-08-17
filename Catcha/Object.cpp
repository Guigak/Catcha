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
