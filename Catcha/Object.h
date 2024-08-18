#pragma once
#include "common.h"

class Object {
private:
	std::wstring m_name = L"";

	DirectX::XMFLOAT3 m_position = { 0.0f, 0.0f, 0.0f };

	DirectX::XMFLOAT3 m_look = { 0.0f, 0.0f, 1.0f };
	DirectX::XMFLOAT3 m_up = { 0.0f, 1.0f, 0.0f };
	DirectX::XMFLOAT3 m_right = { 1.0f, 0.0f, 0.0f };

	DirectX::XMFLOAT4X4 m_world_matrix = MathHelper::Identity_4x4();

	UINT m_constant_buffer_index = -1;

	MeshInfo* m_mesh_info = nullptr;
	MaterialInfo* m_material_info = nullptr;

	D3D12_PRIMITIVE_TOPOLOGY m_primitive_topology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	UINT m_index_count = 0;
	UINT m_start_index_location = 0;
	int m_base_vertex_location = 0;

	int m_dirty_count = FRAME_RESOURCES_NUMBER;

public:
	Object() {}
	Object(std::wstring object_name, MeshInfo* mesh_info, std::wstring mesh_name,
		MaterialInfo* material_info, UINT constant_buffer_info, D3D12_PRIMITIVE_TOPOLOGY primitive_topology);
	~Object() {}

	void Set_Name(std::wstring object_name) { m_name = object_name; }
	void Set_Mesh_Info(MeshInfo* mesh_info, std::wstring mesh_name);
	void Set_Material_Info(MaterialInfo* material_info) { m_material_info = material_info; }
	void Set_CB_Index(UINT constant_buffer_index) { m_constant_buffer_index = constant_buffer_index; }
	void Set_PT(D3D12_PRIMITIVE_TOPOLOGY primitive_topology) { m_primitive_topology = primitive_topology; }

	void Chg_Mesh(std::wstring mesh_name);

	//
	std::wstring Get_Name() { return m_name; }

	DirectX::XMFLOAT3 Get_Position() { return m_position; }

	DirectX::XMFLOAT3 Get_Look() { return m_look; }
	DirectX::XMFLOAT3 Get_Up() { return m_up; }
	DirectX::XMFLOAT3 Get_Right() { return m_right; }

	DirectX::XMFLOAT4X4 Get_WM() { return m_world_matrix; }

	UINT Get_CB_Index() { return m_constant_buffer_index; }

	MeshInfo* Get_Mesh_Info() { return m_mesh_info; }
	MaterialInfo* Get_Material_Info() { return m_material_info; }

	D3D12_PRIMITIVE_TOPOLOGY Get_PT() { return m_primitive_topology; }

	UINT Get_Index() { return m_index_count; }
	UINT Get_Start_Index() { return m_start_index_location; }
	int Get_Base_Vertex() { return m_base_vertex_location; }

	int Get_Dirty_Count() { return m_dirty_count; }
	void Add_Dirty_Count() { m_dirty_count += FRAME_RESOURCES_NUMBER; }
	void Sub_Dirty_Count() { m_dirty_count--; }

	// move
	//void Move(float velocity_x, float velocity_y, float velocity_z);
	//void Move(DirectX::XMFLOAT3 velocity);
	//void Move(DirectX::XMFLOAT3 direction, float length);

	//void Move_Forward(float velocity);
	//void Move_Back(float velocity);
	//void Move_Left(float velocity);
	//void Move_Right(float velocity);

	// test
	void Teleport_Forward() {
		m_position.z += 5.0f;

		m_world_matrix._43 = m_position.z;

		Add_Dirty_Count();
	}

	//// rotate
	//void Rotate(float roll, float pitch, float yaw);
	//void Rotate(DirectX::XMFLOAT3 degree);

	//void Rotate_Roll(float degree);
	//void Rotate_Pitch(float degree);
	//void Rotate_Yaw(float degree);
};

