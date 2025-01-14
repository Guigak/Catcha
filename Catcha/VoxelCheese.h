#pragma once
#include "common.h"
#include "Object.h"

struct Voxel_Info {
	DirectX::XMFLOAT4X4 world_matrix = MathHelper::Identity_4x4();
	bool draw = true;
};

class VoxelCheese : public Object {
private:
	std::array<Voxel_Info, CHEESE_VOXEL_COUNT> m_voxel_info_array;

	UINT m_instance_index = -1;

	UINT m_instance_count = -1;

public:
	VoxelCheese(float position_x, float position_y, float position_z, float scale);
	~VoxelCheese() {}

	virtual void Update(float elapsed_time);
	virtual void Draw(ID3D12GraphicsCommandList* command_list);

	void Set_Instance_Index(UINT instance_index) { m_instance_index = instance_index; }

	UINT Get_Instance_Index() { return m_instance_index; }
	UINT Get_Instance_Count() { return m_instance_count; }

	//std::array<InstanceDatas, CHEESE_VOXEL_COUNT>& Get_Instance_Data();
	void Get_Instance_Data(InstanceDatas* instance_data_pointer);
	void Get_Instance_Data(std::vector<InstanceDatas>& instance_data_array);

	void Rst_Voxel(float position_x, float position_y, float position_z, float scale);

	void Remove_Voxel(int voxel_index);
	void Remove_Random_Voxel();
};

