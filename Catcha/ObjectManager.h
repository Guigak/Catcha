#pragma once
#include "common.h"
#include "Object.h"
#include "MeshManager.h"
#include "FBXManager.h"
#include "SkeletonManager.h"
#include "AnimationManager.h"

enum class Action;

enum class ObjectType {
	OPAQUE_OBJECT,
	TRANSPARENT_OBJECT,
	CAMERA_OBJECT
};

class ObjectManager {
private:
	std::unordered_map<std::wstring, std::unique_ptr<Object>> m_object_map;
	std::vector<Object*> m_objects;

	std::vector<Object*> m_opaque_objects;
	std::vector<Object*> m_transparent_objects;

	std::vector<Object*> m_camera_objects;

	UINT m_object_count = 0;

	//
	std::unordered_map<std::wstring, std::vector<Object*>> m_object_set_map;
	std::unordered_map<std::wstring, std::wstring> m_collision_pair_map;

	//
	MeshManager m_mesh_manager;
	SkeletonManager m_skeleton_manager;
	AnimationManager m_animation_manager;

public:
	ObjectManager() {}
	~ObjectManager() {}

	Object* Add_Obj(std::wstring object_name, MeshInfo* mesh_info, std::wstring mesh_name, MaterialInfo* material_info,
		D3D12_PRIMITIVE_TOPOLOGY primitive_topology, ObjectType object_type, bool physics, bool visiable, std::wstring set_name);

	Object* Get_Obj(std::wstring object_name);
	Object* Get_Obj(UINT object_number);
	Object* Get_Opaque_Obj(UINT object_number);
	Object* Get_Transparent_Obj(UINT object_number);

	size_t Get_Opaque_Obj_Count() { return m_opaque_objects.size(); }
	size_t Get_Transparent_Obj_Count() { return m_transparent_objects.size(); }
	UINT Get_Obj_Count() { return m_object_count; }

	//
	void Move(std::wstring object_name, Action action);
	void Teleport(std::wstring object_name, Action action, float distance);
	void Rotate(std::wstring object_name, Action action, POINTF degree);

	void Update(float elapsed_time);
	
	void Solve_Collision();

	void Add_Collision_Pair(std::wstring set_name_a, std::wstring set_name_b);

	//
	bool Chk_Collision(const DirectX::BoundingOrientedBox& OBB_a, const DirectX::BoundingOrientedBox& OBB_b,
		DirectX::XMFLOAT3& normal, float& depth);

	bool Overlaped(DirectX::XMVECTOR corners_a[], DirectX::XMVECTOR corners_b[], const DirectX::XMVECTOR& axis, float& overlap);

	//
	void Bind_Cam_2_Obj(std::wstring camera_name, std::wstring object_name, float distance);	// Bind Camera to Object

	//
	MeshManager& Get_Mesh_Manager() { return m_mesh_manager; }
	SkeletonManager& Get_Skeleton_Manager() { return m_skeleton_manager; }
	AnimationManager& Get_Animation_Manager() { return m_animation_manager; }

	void Ipt_From_FBX(std::wstring file_name, bool merge_mesh, bool add_object, bool merge_object, BYTE info_flag);

	Object* Add_Obj(std::wstring object_name, std::wstring mesh_name, std::wstring set_name = L"Object",
		DirectX::XMMATRIX world_matrix = DirectX::XMMatrixIdentity(),
		D3D12_PRIMITIVE_TOPOLOGY primitive_topology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST, ObjectType object_type = ObjectType::OPAQUE_OBJECT,
		bool physics = false, bool visiable = true);
	Object* Add_Obj(std::wstring object_name, std::vector<Mesh>& mesh_array, std::wstring set_name = L"Object",
		D3D12_PRIMITIVE_TOPOLOGY primitive_topology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST, ObjectType object_type = ObjectType::OPAQUE_OBJECT,
		bool physics = false, bool visiable = true);

	//
	void Build_BV(ID3D12Device* device, ID3D12GraphicsCommandList* command_list);

	//
	void Set_Sklt_2_Obj(std::wstring object_name, std::wstring skeleton_name);
};

