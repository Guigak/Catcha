#pragma once
#include "Object.h"

class Player : public Object{
public:
	Player() {}
	Player(ObjectManager* object_manager, std::wstring object_name, Mesh_Info* mesh, DirectX::XMMATRIX world_matrix, UINT constant_buffer_index, D3D12_PRIMITIVE_TOPOLOGY primitive_topology, bool physics, bool visible);
	virtual ~Player() {}

	virtual void Update(float elapsed_time);

	virtual void Act_Four();
	virtual void Act_Five();
};

