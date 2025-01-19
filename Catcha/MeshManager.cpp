#include "MeshManager.h"

MeshManager::MeshManager() {
    Crt_Default_Box();
}

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

Mesh_Info* MeshManager::Crt_Box_Mesh(std::wstring mesh_name, float width, float height, float depth) {
    std::vector<Vertex_Info> vertices_vector;
    std::vector<std::uint32_t> indices_vector;

    Vertex_Info vertices[24];

    float half_width = 0.5f * width;
    float half_height = 0.5f * height;
    float half_depth = 0.5f * depth;

    int count = 0;
    // front
    vertices[count++] = Vertex_Info(-half_width, -half_height, -half_depth, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
    vertices[count++] = Vertex_Info(-half_width, +half_height, -half_depth, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
    vertices[count++] = Vertex_Info(+half_width, +half_height, -half_depth, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f);
    vertices[count++] = Vertex_Info(+half_width, -half_height, -half_depth, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f);

    // back
    vertices[count++] = Vertex_Info(-half_width, -half_height, +half_depth, 0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f);
    vertices[count++] = Vertex_Info(+half_width, -half_height, +half_depth, 0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
    vertices[count++] = Vertex_Info(+half_width, +half_height, +half_depth, 0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
    vertices[count++] = Vertex_Info(-half_width, +half_height, +half_depth, 0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f);

    // top
    vertices[count++] = Vertex_Info(-half_width, +half_height, -half_depth, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
    vertices[count++] = Vertex_Info(-half_width, +half_height, +half_depth, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
    vertices[count++] = Vertex_Info(+half_width, +half_height, +half_depth, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f);
    vertices[count++] = Vertex_Info(+half_width, +half_height, -half_depth, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f);

    // bottom
    vertices[count++] = Vertex_Info(-half_width, -half_height, -half_depth, 0.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f);
    vertices[count++] = Vertex_Info(+half_width, -half_height, -half_depth, 0.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
    vertices[count++] = Vertex_Info(+half_width, -half_height, +half_depth, 0.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
    vertices[count++] = Vertex_Info(-half_width, -half_height, +half_depth, 0.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f);

    // left
    vertices[count++] = Vertex_Info(-half_width, -half_height, +half_depth, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f);
    vertices[count++] = Vertex_Info(-half_width, +half_height, +half_depth, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f);
    vertices[count++] = Vertex_Info(-half_width, +half_height, -half_depth, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f);
    vertices[count++] = Vertex_Info(-half_width, -half_height, -half_depth, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f);

    // right
    vertices[count++] = Vertex_Info(+half_width, -half_height, -half_depth, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f);
    vertices[count++] = Vertex_Info(+half_width, +half_height, -half_depth, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f);
    vertices[count++] = Vertex_Info(+half_width, +half_height, +half_depth, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f);
    vertices[count++] = Vertex_Info(+half_width, -half_height, +half_depth, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);

    vertices_vector.assign(&vertices[0], &vertices[count]);

    std::uint32_t indices[36];

    count = 0;
    // front
    indices[count++] = 0;
    indices[count++] = 1;
    indices[count++] = 2;

    indices[count++] = 0;
    indices[count++] = 2;
    indices[count++] = 3;

    // back
    indices[count++] = 4;
    indices[count++] = 5;
    indices[count++] = 6;

    indices[count++] = 4;
    indices[count++] = 6;
    indices[count++] = 7;

    // top
    indices[count++] = 8;
    indices[count++] = 9;
    indices[count++] = 10;

    indices[count++] = 8;
    indices[count++] = 10;
    indices[count++] = 11;

    // bottom
    indices[count++] = 12;
    indices[count++] = 13;
    indices[count++] = 14;

    indices[count++] = 12;
    indices[count++] = 14;
    indices[count++] = 15;

    // left
    indices[count++] = 16;
    indices[count++] = 17;
    indices[count++] = 18;

    indices[count++] = 16;
    indices[count++] = 18;
    indices[count++] = 19;

    // right
    indices[count++] = 20;
    indices[count++] = 21;
    indices[count++] = 22;

    indices[count++] = 20;
    indices[count++] = 22;
    indices[count++] = 23;

    indices_vector.assign(&indices[0], &indices[count]);

    return Add_Mesh(mesh_name, vertices_vector, indices_vector);
}

Mesh_Info* MeshManager::Crt_Floor_Plane_Mesh(std::wstring mesh_name, float width, float depth) {
    std::vector<Vertex_Info> vertices_vector;
    std::vector<std::uint32_t> indices_vector;

    Vertex_Info vertices[4];

    float half_width = 0.5f * width;
    float half_depth = 0.5f * depth;

    int count = 0;

    // top
    vertices[count++] = Vertex_Info(-half_width, 0.0f, -half_depth, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
    vertices[count++] = Vertex_Info(-half_width, 0.0f, +half_depth, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
    vertices[count++] = Vertex_Info(+half_width, 0.0f, +half_depth, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f);
    vertices[count++] = Vertex_Info(+half_width, 0.0f, -half_depth, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f);

    vertices_vector.assign(&vertices[0], &vertices[count]);

    std::uint32_t indices[36];

    count = 0;

    // top
    indices[count++] = 0;
    indices[count++] = 1;
    indices[count++] = 2;

    indices[count++] = 0;
    indices[count++] = 2;
    indices[count++] = 3;

    indices_vector.assign(&indices[0], &indices[count]);

    return Add_Mesh(mesh_name, vertices_vector, indices_vector);
}

Mesh_Info* MeshManager::Crt_Wall_Plane_Mesh(std::wstring mesh_name, float width, float height) {
    std::vector<Vertex_Info> vertices_vector;
    std::vector<std::uint32_t> indices_vector;

    Vertex_Info vertices[4];

    float half_width = 0.5f * width;
    float half_height = 0.5f * height;

    int count = 0;
    // front
    vertices[count++] = Vertex_Info(-half_width, -half_height, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
    vertices[count++] = Vertex_Info(-half_width, +half_height, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
    vertices[count++] = Vertex_Info(+half_width, +half_height, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f);
    vertices[count++] = Vertex_Info(+half_width, -half_height, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f);

    vertices_vector.assign(&vertices[0], &vertices[count]);

    std::uint32_t indices[6];

    count = 0;
    // front
    indices[count++] = 0;
    indices[count++] = 1;
    indices[count++] = 2;

    indices[count++] = 0;
    indices[count++] = 2;
    indices[count++] = 3;

    indices_vector.assign(&indices[0], &indices[count]);

    return Add_Mesh(mesh_name, vertices_vector, indices_vector);
}

Mesh_Info* MeshManager::Crt_Point_Mesh(std::wstring mesh_name) {
    std::vector<Vertex_Info> vertices_vector;
    std::vector<std::uint32_t> indices_vector;

    vertices_vector.emplace_back(Vertex_Info());
    indices_vector.emplace_back(0);

    Mesh_Info* mesh_info_pointer = Add_Mesh(mesh_name, vertices_vector, indices_vector);
    mesh_info_pointer->primitive_topology = D3D_PRIMITIVE_TOPOLOGY_POINTLIST;

    return mesh_info_pointer;
}

Mesh_Info* MeshManager::Crt_Default_Box() {
    return Crt_Box_Mesh(L"default_box");
}

Mesh_Info* MeshManager::Crt_Default_Floor_Plane() {
    return Crt_Floor_Plane_Mesh(L"default_floor_plane");
}

Mesh_Info* MeshManager::Crt_Default_Wall_Plane() {
    return Crt_Wall_Plane_Mesh(L"default_wall_plane");
}

void MeshManager::Crt_BV(ID3D12Device* device, ID3D12GraphicsCommandList* command_list) {
    for (auto& m : m_mesh_map) {
        m.second.get()->Crt_BV(device, command_list);
    }
}
