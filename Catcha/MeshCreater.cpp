#include "MeshCreater.h"
#include "FBXManager.h"

MeshData MeshCreater::Crt_Box(float width, float height, float depth, std::uint32_t subdivisions_number) {
    MeshData mesh_data;

    VertexData vertices[24];

    float half_width = 0.5f * width;
    float half_height = 0.5f * height;
    float half_depth = 0.5f * depth;

    int count = 0;
    // front
    vertices[count++] = VertexData(-half_width, -half_height, -half_depth, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
    vertices[count++] = VertexData(-half_width, +half_height, -half_depth, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
    vertices[count++] = VertexData(+half_width, +half_height, -half_depth, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f);
    vertices[count++] = VertexData(+half_width, -half_height, -half_depth, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f);

    // back
    vertices[count++] = VertexData(-half_width, -half_height, +half_depth, 0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f);
    vertices[count++] = VertexData(+half_width, -half_height, +half_depth, 0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
    vertices[count++] = VertexData(+half_width, +half_height, +half_depth, 0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
    vertices[count++] = VertexData(-half_width, +half_height, +half_depth, 0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f);

    // top
    vertices[count++] = VertexData(-half_width, +half_height, -half_depth, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
    vertices[count++] = VertexData(-half_width, +half_height, +half_depth, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
    vertices[count++] = VertexData(+half_width, +half_height, +half_depth, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f);
    vertices[count++] = VertexData(+half_width, +half_height, -half_depth, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f);

    // bottom
    vertices[count++] = VertexData(-half_width, -half_height, -half_depth, 0.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f);
    vertices[count++] = VertexData(+half_width, -half_height, -half_depth, 0.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
    vertices[count++] = VertexData(+half_width, -half_height, +half_depth, 0.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
    vertices[count++] = VertexData(-half_width, -half_height, +half_depth, 0.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f);

    // left
    vertices[count++] = VertexData(-half_width, -half_height, +half_depth, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f);
    vertices[count++] = VertexData(-half_width, +half_height, +half_depth, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f);
    vertices[count++] = VertexData(-half_width, +half_height, -half_depth, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f);
    vertices[count++] = VertexData(-half_width, -half_height, -half_depth, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f);

    // right
    vertices[count++] = VertexData(+half_width, -half_height, -half_depth, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f);
    vertices[count++] = VertexData(+half_width, +half_height, -half_depth, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f);
    vertices[count++] = VertexData(+half_width, +half_height, +half_depth, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f);
    vertices[count++] = VertexData(+half_width, -half_height, +half_depth, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);

    mesh_data.vertices.assign(&vertices[0], &vertices[count]);

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

    mesh_data.minimum_x = -half_width;
    mesh_data.minimum_y = -half_height;
    mesh_data.minimum_z = -half_depth;

    mesh_data.maximum_x = half_width;
    mesh_data.maximum_y = half_height;
    mesh_data.maximum_z = half_depth;

    mesh_data.indices_32.assign(&indices[0], &indices[count]);

    subdivisions_number = std::min<std::uint32_t>(subdivisions_number, 6u);

    for (std::uint32_t i = 0; i < subdivisions_number; ++i) {
        Subdivide(mesh_data);
    }

    return mesh_data;
}

MeshData MeshCreater::Crt_Mesh_From_File(std::wstring file_name, std::uint32_t subdivisions_number) {
    FBXManager* fbx_manager = FBXManager::Get_Inst();
    MeshData mesh_data = fbx_manager->Ipt_Mesh_From_File(file_name);

    subdivisions_number = std::min<std::uint32_t>(subdivisions_number, 6u);

    for (std::uint32_t i = 0; i < subdivisions_number; ++i) {
        Subdivide(mesh_data);
    }

    return mesh_data;
}

void MeshCreater::Subdivide(MeshData& mesh) {
    MeshData copy = mesh;

    mesh.vertices.resize(0);
    mesh.indices_32.resize(0);

    std::uint32_t triangles_count = (std::uint32_t)copy.indices_32.size() / 3;

    for (std::uint32_t i = 0; i < triangles_count; ++i) {
        VertexData vertex_0 = copy.vertices[copy.indices_32[i * 3 + 0]];
        VertexData vertex_1 = copy.vertices[copy.indices_32[i * 3 + 1]];
        VertexData vertex_2 = copy.vertices[copy.indices_32[i * 3 + 2]];

        VertexData mid_vertex_0 = Calc_Mid_Point(vertex_0, vertex_1);
        VertexData mid_vertex_1 = Calc_Mid_Point(vertex_1, vertex_2);
        VertexData mid_vertex_2 = Calc_Mid_Point(vertex_0, vertex_2);

        mesh.vertices.emplace_back(vertex_0);
        mesh.vertices.emplace_back(vertex_1);
        mesh.vertices.emplace_back(vertex_2);
        mesh.vertices.emplace_back(mid_vertex_0);
        mesh.vertices.emplace_back(mid_vertex_1);
        mesh.vertices.emplace_back(mid_vertex_2);

        mesh.indices_32.emplace_back(i * 6 + 0);
        mesh.indices_32.emplace_back(i * 6 + 3);
        mesh.indices_32.emplace_back(i * 6 + 5);

        mesh.indices_32.emplace_back(i * 6 + 3);
        mesh.indices_32.emplace_back(i * 6 + 4);
        mesh.indices_32.emplace_back(i * 6 + 5);

        mesh.indices_32.emplace_back(i * 6 + 5);
        mesh.indices_32.emplace_back(i * 6 + 4);
        mesh.indices_32.emplace_back(i * 6 + 2);

        mesh.indices_32.emplace_back(i * 6 + 3);
        mesh.indices_32.emplace_back(i * 6 + 1);
        mesh.indices_32.emplace_back(i * 6 + 4);
    }
}

VertexData MeshCreater::Calc_Mid_Point(VertexData& a, VertexData& b) {
    DirectX::XMVECTOR position_a = DirectX::XMLoadFloat3(&a.position);
    DirectX::XMVECTOR position_b = DirectX::XMLoadFloat3(&b.position);

    DirectX::XMVECTOR normal_a = DirectX::XMLoadFloat3(&a.normal);
    DirectX::XMVECTOR normal_b = DirectX::XMLoadFloat3(&b.normal);

    DirectX::XMVECTOR tangent_a = DirectX::XMLoadFloat3(&a.tangent);
    DirectX::XMVECTOR tangent_b = DirectX::XMLoadFloat3(&b.tangent);

    DirectX::XMVECTOR uv_a = DirectX::XMLoadFloat2(&a.uv);
    DirectX::XMVECTOR uv_b = DirectX::XMLoadFloat2(&b.uv);

    //
    DirectX::XMVECTOR position = DirectX::XMVectorScale(DirectX::XMVectorAdd(position_a, position_b), 0.5f);
    DirectX::XMVECTOR normal = DirectX::XMVector3Normalize(DirectX::XMVectorScale(DirectX::XMVectorAdd(normal_a, normal_b), 0.5f));
    DirectX::XMVECTOR tangent = DirectX::XMVector3Normalize(DirectX::XMVectorScale(DirectX::XMVectorAdd(tangent_a, tangent_b), 0.5f));
    DirectX::XMVECTOR uv = DirectX::XMVectorScale(DirectX::XMVectorAdd(uv_a, uv_b), 0.5f);

    VertexData vertex;
    DirectX::XMStoreFloat3(&vertex.position, position);
    DirectX::XMStoreFloat3(&vertex.normal, normal);
    DirectX::XMStoreFloat3(&vertex.tangent, tangent);
    DirectX::XMStoreFloat2(&vertex.uv, uv);

    return vertex;
}
