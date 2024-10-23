#include "MaterialManager.h"

Material_Info* MaterialManager::Add_Material_Info(std::wstring material_info_name, Material_Factor& material_factor) {
	std::unique_ptr<Material_Info> material_info = std::make_unique<Material_Info>(material_info_name, material_factor);
	m_material_info_map[material_info_name] = std::move(material_info);

	return m_material_info_map[material_info_name].get();
}

Material* MaterialManager::Add_Material(std::wstring material_name, std::vector<Material_Info*>& material_info_array) {
	std::unique_ptr<Material> material = std::make_unique<Material>(m_material_count++, material_name, material_info_array);
	m_material_map[material_name] = std::move(material);

	return m_material_map[material_name].get();
}

bool MaterialManager::Contains_Material_Info(std::wstring material_info_name) {
	return m_material_info_map.contains(material_info_name);
}
