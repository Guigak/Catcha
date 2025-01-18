#pragma once
#include "common.h"
#include "Object.h"

class InstanceObject : public Object {
protected:
	std::vector<InstanceData> m_instance_data_array;	// transposed

	UINT m_instance_index = -1;
	UINT m_instance_max_count = -1;
	UINT m_instance_count = -1;

	UINT m_instance_dirty_count = FRAME_RESOURCES_NUMBER;

public:
	InstanceObject() {}
	virtual ~InstanceObject() {}

	virtual void Update(float elapsed_time);
	virtual void Draw(ID3D12GraphicsCommandList* command_list);

	void Set_Instance_Index(UINT instance_index) { m_instance_index = instance_index; }

	UINT Get_Instance_Index() { return m_instance_index; }
	UINT Get_Instance_Max_Count() { return m_instance_max_count; }
	UINT Get_Instance_Count() { return m_instance_count; }

	UINT Get_Instc_Dirty_Cnt() { return m_instance_dirty_count; }
	void Rst_Instc_Dirty_Cnt() { m_instance_dirty_count = FRAME_RESOURCES_NUMBER; }
	void Sub_Instc_Dirty_Cnt() { --m_instance_dirty_count; }

	virtual void Get_Instance_Data(InstanceData* instance_data_pointer);
	virtual void Get_Instance_Data(std::vector<InstanceData>& instance_data_array);

	void Add_Instance_Data(InstanceData& instance_data);
};

