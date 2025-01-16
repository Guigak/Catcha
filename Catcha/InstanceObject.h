#pragma once
#include "common.h"
#include "Object.h"

class InstanceObject : public Object {
protected:
	std::vector<InstanceDatas> m_instance_data_array;

	UINT m_instance_index = -1;
	UINT m_instance_max_count = -1;
	UINT m_instance_count = -1;

public:
	InstanceObject() {}
	~InstanceObject() {}

	virtual void Update(float elapsed_time);
	virtual void Draw(ID3D12GraphicsCommandList* command_list);

	void Set_Instance_Index(UINT instance_index) { m_instance_index = instance_index; }

	UINT Get_Instance_Index() { return m_instance_index; }
	UINT Get_Instance_Max_Count() { return m_instance_max_count; }
	UINT Get_Instance_Count() { return m_instance_count; }

	virtual void Get_Instance_Data(InstanceDatas* instance_data_pointer);
	virtual void Get_Instance_Data(std::vector<InstanceDatas>& instance_data_array);

	void Add_Instance_Data(InstanceDatas& instance_data);
};

