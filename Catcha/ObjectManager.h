#pragma once
#include "common.h"
#include "Object.h"

enum class Action;

class ObjectManager {
private:
	std::unordered_map<std::wstring, std::unique_ptr<Object>> m_object_map;
	std::vector<Object*> m_objects;

	std::vector<Object*> m_opaque_objects;
	std::vector<Object*> m_transparent_objects;

	UINT m_object_count = 0;

public:
	ObjectManager() {}
	~ObjectManager() {}

	void Add_Obj(std::wstring object_name, MeshInfo* mesh_info, std::wstring mesh_name, MaterialInfo* material_info,
		D3D12_PRIMITIVE_TOPOLOGY primitive_topology, bool opaque_object, bool physics);

	Object* Get_Obj(std::wstring object_name);
	Object* Get_Obj(UINT object_number);
	Object* Get_Opaque_Obj(UINT object_number);
	Object* Get_Transparent_Obj(UINT object_number);

	size_t Get_Opaque_Obj_Count() { return m_opaque_objects.size(); }
	size_t Get_Transparent_Obj_Count() { return m_transparent_objects.size(); }
	UINT Get_Obj_Count() { return m_object_count; }

	//
	void Move(std::wstring object_name, Action action);
	void Teleport(std::wstring object_name, Action action);

	void Update(float elapsed_time);
	
	void Solve_Collision();
};

