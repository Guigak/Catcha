#pragma once
#include "common.h"
#include "InstanceObject.h"

class VoxelCheese : public InstanceObject {
private:
	UINT m_detail_level = 0;

public:
	VoxelCheese(float position_x, float position_y, float position_z, float scale, UINT detail_level);
	virtual ~VoxelCheese() {}

	virtual void Get_Instance_Data(std::vector<InstanceData>& instance_data_array);

	void Rst_Voxel(float position_x, float position_y, float position_z, float scale, UINT detail_level);

	void Add_Voxel(float position_x, float position_y, float position_z, float scale, UINT detail_level);

	void Remove_Voxel(int voxel_index);
	void Remove_Random_Voxel(int random_seed);
	void Remove_Sphere_Voxel(const DirectX::XMFLOAT3& center, float radius);
};

