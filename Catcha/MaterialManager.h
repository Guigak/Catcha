#pragma once
#include "common.h"

class MaterialManager {
private:
	UINT m_material_count = 0;

	std::unordered_map<std::wstring, std::unique_ptr<Material_Info>> m_material_info_map;
	std::unordered_map<std::wstring, std::unique_ptr<Material>> m_material_map;

	int m_dirty_count = FRAME_RESOURCES_NUMBER;

public:
	MaterialManager() {}
	~MaterialManager() {}

	Material_Info* Add_Material_Info(std::wstring material_info_name, Material_Factor& material_factor);

	Material* Add_Material(std::wstring material_name, std::vector<Material_Info*>& material_info_array);

	bool Contains_Material_Info(std::wstring material_info_name);

	size_t Get_Material_Count() { return m_material_map.size(); }

	std::unordered_map<std::wstring, std::unique_ptr<Material>>& Get_Material_Map() { return m_material_map; }

	void Update();

	int Get_Dirty_Count() { return m_dirty_count; }
	void Rst_Dirty_Count() { m_dirty_count = FRAME_RESOURCES_NUMBER; }
	void Sub_Dirty_Count() { m_dirty_count--; }

	void Crt_Default_Material();
};

