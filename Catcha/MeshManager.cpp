#include "MeshManager.h"

Mesh_Info* MeshManager::Add_Mesh(std::wstring mesh_name, std::unique_ptr<Mesh_Info> mesh_pointer) {
    m_mesh_map[mesh_name] = std::move(mesh_pointer);
    ++m_mesh_count;

    return m_mesh_map[mesh_name].get();
}

Mesh_Info* MeshManager::Add_Mesh(std::wstring mesh_name, std::vector<Vertex_Info> vertices, std::vector<std::uint32_t> indices) {
    std::unique_ptr<Mesh_Info> mesh_info = std::make_unique<Mesh_Info>(vertices, indices);

    m_mesh_map[mesh_name] = std::move(mesh_info);
    ++m_mesh_count;

    return m_mesh_map[mesh_name].get();
}

Mesh_Info* MeshManager::Get_Mesh(std::wstring mesh_name) {
    return m_mesh_map[mesh_name].get();
}

void MeshManager::Crt_BV(ID3D12Device* device, ID3D12GraphicsCommandList* command_list) {
    for (auto& m : m_mesh_map) {
        m.second.get()->Crt_BV(device, command_list);
    }
}
