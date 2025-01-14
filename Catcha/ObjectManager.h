#pragma once
#include "common.h"
#include "Object.h"
#include "MeshManager.h"
#include "FBXManager.h"
#include "SkeletonManager.h"
#include "AnimationManager.h"
#include "MaterialManager.h"

enum class Action;

enum class ObjectType {
	OPAQUE_OBJECT,
	TRANSPARENT_OBJECT,
	CAMERA_OBJECT,
	CHARACTER_OBJECT,
	GHOST_OBJECT
};

class ObjectManager {
private:
	std::unordered_map<std::wstring, std::unique_ptr<Object>> m_object_map;
	std::vector<Object*> m_objects;

	// 캐릭터를 위한 변수
	std::vector<Object*> m_characters;
	UINT m_character_count = 0;

	std::vector<Object*> m_opaque_objects;
	std::vector<Object*> m_transparent_objects;

	std::vector<Object*> m_camera_objects;

	std::vector<Object*> m_instance_objects;

	UINT m_object_count = 0;
	UINT m_instance_object_count = 0;

	//
	std::vector<Object*> m_optimization_obb_objects;
	std::vector<Object*> m_collision_obb_objects;

	//
	std::unordered_map<std::wstring, std::vector<Object*>> m_object_set_map;
	std::unordered_map<std::wstring, std::wstring> m_collision_pair_map;

	//
	MaterialManager m_material_manager;
	MeshManager m_mesh_manager;
	SkeletonManager m_skeleton_manager;
	AnimationManager m_animation_manager;

public:
	ObjectManager() {}
	~ObjectManager() {}

	Object* Get_Obj(std::wstring object_name);
	Object* Get_Obj(UINT object_number);
	Object* Get_Obj_Character(UINT object_number);
	Object* Get_Opaque_Obj(UINT object_number);
	Object* Get_Transparent_Obj(UINT object_number);

	std::vector<Object*>& Get_Obj_Arr() { return m_objects; }
	std::vector<Object*>& Get_Opaque_Obj_Arr() { return m_opaque_objects; }
	std::vector<Object*>& Get_Transparent_Obj_Arr() { return m_transparent_objects; }
	std::vector<Object*>& Get_Cam_Obj_Arr() { return m_camera_objects; }
	std::vector<Object*>& Get_Instc_Obj_Arr() { return m_instance_objects; }	// Get Instance Object Array
	std::vector<Object*>& Get_Optmz_OBB_Obj_Arr() { return m_optimization_obb_objects; }	// Get Optimization OBB Object Array
	std::vector<Object*>& Get_Col_OBB_Obj_Arr() { return m_collision_obb_objects; }	// Get Collision OBB Object Array

	size_t Get_Opaque_Obj_Count() { return m_opaque_objects.size(); }
	size_t Get_Transparent_Obj_Count() { return m_transparent_objects.size(); }
	UINT Get_Obj_Count() { return m_object_count; }
	
	// 캐릭터 수 Getter
	UINT Get_Character_Count() { return m_character_count; }

	//
	void Move(std::wstring object_name, Action action, BYTE flag);
	void Teleport(std::wstring object_name, Action action, float distance);
	void Rotate(std::wstring object_name, Action action, float degree);

	//
	void Actions(std::wstring object_name, Action action);

	void Update(float elapsed_time);
	
	void Solve_Collision();

	void Add_Collision_Pair(std::wstring set_name_a, std::wstring set_name_b);

	//
	bool Chk_Collision(const DirectX::BoundingOrientedBox& OBB_a, const DirectX::BoundingOrientedBox& OBB_b,
		DirectX::XMFLOAT3& normal, float& depth);

	bool Overlaped(DirectX::XMVECTOR corners_a[], DirectX::XMVECTOR corners_b[], const DirectX::XMVECTOR& axis, float& overlap);

	//
	void Bind_Cam_2_Obj(std::wstring camera_name, std::wstring object_name,
		float offset_look, float offset_up, float offset_right, float distance, BYTE flag = ROTATE_SYNC_NONE);	// Bind Camera to Object

	//
	MeshManager& Get_Mesh_Manager() { return m_mesh_manager; }
	SkeletonManager& Get_Skeleton_Manager() { return m_skeleton_manager; }
	AnimationManager& Get_Animation_Manager() { return m_animation_manager; }
	MaterialManager& Get_Material_Manager() { return m_material_manager; }

	void Ipt_From_FBX(std::wstring file_name, bool merge_mesh, bool add_object, bool merge_object, BYTE info_flag, std::wstring skeleton_name = L"");

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

	//
	Object* Add_Cam(std::wstring camera_name, std::wstring set_name = L"camera", std::wstring bind_object_name = L"",
		float offset_look = 0.0f, float offset_up = 0.0f, float offset_right = 0.0f, float distance = 0.0f, BYTE flag = ROTATE_SYNC_NONE);

	//
	Object* Add_Col_OBB_Obj(std::wstring obb_object_name, DirectX::BoundingOrientedBox obb, std::wstring object_name = L"");

	//
	Object* Add_Voxel_Cheese(std::wstring object_name, DirectX::XMFLOAT3 object_position, float scale);

	// player 전환을 위한 object swap
	void Swap_Object(const std::wstring& key1, const std::wstring& key2);
	// player 선정 전까지의 패킷 보내기 제한을 위한 카메라 설정
	void Set_Camera_4_Server(std::wstring camera_name, bool NeedSend);
	void Set_Camera_Init_4_Server(std::wstring camera_name);
};

