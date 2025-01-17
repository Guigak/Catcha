#pragma once
#include "common.h"

class MeshManager {
private:
	int m_mesh_count = 0;

	std::unordered_map<std::wstring, std::unique_ptr<Mesh_Info>> m_mesh_map;

public:
	MeshManager();
	~MeshManager() {}

	Mesh_Info* Add_Mesh(std::wstring mesh_name, std::unique_ptr<Mesh_Info> mesh_pointer);
	Mesh_Info* Add_Mesh(std::wstring mesh_name, std::vector<Vertex_Info> vertices, std::vector<std::uint32_t> indices);

	Mesh_Info* Get_Mesh(std::wstring mesh_name);

	//
	Mesh_Info* Crt_Box_Mesh(std::wstring mesh_name, float width = 1.0f, float height = 1.0f, float depth = 1.0f);
	Mesh_Info* Crt_Floor_Plane_Mesh(std::wstring mesh_name, float width = 1.0f, float depth = 1.0f);
	Mesh_Info* Crt_Wall_Plane_Mesh(std::wstring mesh_name, float width = 1.0f, float height = 1.0f);

	Mesh_Info* Crt_Default_Box();
	Mesh_Info* Crt_Default_Floor_Plane();
	Mesh_Info* Crt_Default_Wall_Plane();

	void Crt_BV(ID3D12Device* device, ID3D12GraphicsCommandList* command_list);
};

