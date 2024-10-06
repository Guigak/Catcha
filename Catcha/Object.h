#pragma once
#include "common.h"
#include "NetworkManager.h"

enum class Object_State {
	IDLE_STATE, MOVE_STATE, JUMP_STATE
};

class Camera;

class Object {
protected:
	std::wstring m_name = L"";

	DirectX::XMFLOAT3 m_position = { 0.0f, 0.0f, 0.0f };
	DirectX::XMFLOAT3 m_rotate = { 0.0f, 0.0f, 0.0f };
	DirectX::XMFLOAT3 m_scale = { 0.0f, 0.0f, 0.0f };

	DirectX::XMFLOAT3 m_look = { 0.0f, 0.0f, 1.0f };
	DirectX::XMFLOAT3 m_up = { 0.0f, 1.0f, 0.0f };
	DirectX::XMFLOAT3 m_right = { 1.0f, 0.0f, 0.0f };

	DirectX::XMFLOAT4X4 m_world_matrix = MathHelper::Identity_4x4();

	UINT m_constant_buffer_index = -1;

	MeshInfo* m_mesh_info = nullptr;
	std::wstring m_submesh_name = L"";
	MaterialInfo* m_material_info = nullptr;

	D3D12_PRIMITIVE_TOPOLOGY m_primitive_topology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	UINT m_index_count = 0;
	UINT m_start_index_location = 0;
	int m_base_vertex_location = 0;

	int m_dirty_count = FRAME_RESOURCES_NUMBER;

	bool m_dirty = false;

	//
	bool m_physics = false;

	float m_gravity = 9.8f;

	DirectX::XMFLOAT3 m_velocity = DirectX::XMFLOAT3();

	float m_acceleration = 100.0f;
	float m_deceleration = 200.0f;

	float m_speed = 0.0f;
	float m_max_speed = 200.0f;

	DirectX::XMFLOAT3 m_force = DirectX::XMFLOAT3();

	float m_friction = 0.0f;

	DirectX::XMFLOAT3 m_delta_position = DirectX::XMFLOAT3();

	//
	bool m_moving = false;

	Object_State m_state = Object_State::IDLE_STATE;

	//
	Camera* m_camera = nullptr;

	bool m_visiable = false;

	//
	DirectX::BoundingOrientedBox m_OBB;

	// [SC] ��ġ ������ ���� ����
	DirectX::XMFLOAT3 m_target_position{ 0, 0, 0 };  // Position received from the server
	float m_lerp_progress = 0.0f;     // ���� ���� ����

	// [SC] ȸ�� ������ ���� ����
	float m_last_sent_pitch = 0.0f;    
	const float pitchSendThreshold = 0.1f;  // 100 ms (0.1 seconds)
	std::chrono::high_resolution_clock::time_point m_lastSendTime;

public:
	Object() {}
	Object(std::wstring object_name, MeshInfo* mesh_info, std::wstring mesh_name,
		MaterialInfo* material_info, UINT constant_buffer_info, D3D12_PRIMITIVE_TOPOLOGY primitive_topology, bool physics);
	~Object() {}

	void Set_Name(std::wstring object_name) { m_name = object_name; }
	void Set_Mesh_Info(MeshInfo* mesh_info, std::wstring mesh_name);
	void Set_Material_Info(MaterialInfo* material_info) { m_material_info = material_info; }
	void Set_CB_Index(UINT constant_buffer_index) { m_constant_buffer_index = constant_buffer_index; }
	void Set_PT(D3D12_PRIMITIVE_TOPOLOGY primitive_topology) { m_primitive_topology = primitive_topology; }
	void Set_Phys(bool physics) { m_physics = physics; }

	void Chg_Mesh(std::wstring mesh_name);

	//
	void Crt_Simple_OBB();
	DirectX::XMMATRIX Get_OBB_WM();
	DirectX::BoundingOrientedBox Get_Calcd_OBB();

	//
	std::wstring Get_Name() { return m_name; }

	DirectX::XMVECTOR Get_Position_V() { return DirectX::XMLoadFloat3(&m_position); }	// Get Position Vector
	DirectX::XMFLOAT3 Get_Position_3f() { return m_position; }	// Get Position float3

	DirectX::XMVECTOR Get_Rotate_V() { return DirectX::XMLoadFloat3(&m_rotate); }	// Get Rotate Vector
	DirectX::XMFLOAT3 Get_Rotate_3f() { return m_rotate; }	// Get Rotate float3

	DirectX::XMVECTOR Get_Scale_V() { return DirectX::XMLoadFloat3(&m_scale); }	// Get Scale Vector
	DirectX::XMFLOAT3 Get_Scale_3f() { return m_scale; }	// Get Scale float3

	DirectX::XMFLOAT3 Get_Look() { return m_look; }
	DirectX::XMFLOAT3 Get_Up() { return m_up; }
	DirectX::XMFLOAT3 Get_Right() { return m_right; }

	DirectX::XMFLOAT4X4 Get_WM() { return m_world_matrix; }

	UINT Get_CB_Index() { return m_constant_buffer_index; }

	MeshInfo* Get_Mesh_Info() { return m_mesh_info; }
	MaterialInfo* Get_Material_Info() { return m_material_info; }

	D3D12_PRIMITIVE_TOPOLOGY Get_PT() { return m_primitive_topology; }

	UINT Get_Index() { return m_index_count; }
	UINT Get_Start_Index() { return m_start_index_location; }
	int Get_Base_Vertex() { return m_base_vertex_location; }

	int Get_Dirty_Count() { return m_dirty_count; }
	void Rst_Dirty_Count() { m_dirty_count = FRAME_RESOURCES_NUMBER; }
	void Sub_Dirty_Count() { m_dirty_count--; }

	DirectX::XMFLOAT3 Get_Vel() { return m_velocity; }
	DirectX::XMFLOAT3 Get_Force() { return m_force; }

	float Get_Spd() { return m_speed; }

	void Calc_Delta(float elapsed_time);
	void Calc_Delta_Characters(float elapsed_time);
	//void Move_N_Solve_Collision();
	virtual void Update();

	void Udt_WM();	// Update World Matrix
	void Udt_LUR();	// Update Look Up Right

	// [SC] ��ġ ������ ���� �Լ�
	void LerpPosition(float deltaTime);
	void SetTargetPosition(const DirectX::XMFLOAT3& newPosition);


	//
	void Set_Position(float position_x, float position_y, float position_z);

	// move
	void Move(DirectX::XMFLOAT3 direction);

	void Move_Forward();
	void Move_Back();
	void Move_Left();
	void Move_Right();
	void Move_Up();
	void Move_Down();

	// teleport
	void Teleport(DirectX::XMFLOAT3 direction, float distance);

	void TP_Forward(float distance);
	void TP_Back(float distance);
	void TP_Left(float distance);
	void TP_Right(float distance);
	void TP_Up(float distance);
	void TP_Down(float distance);

	// rotate
	void Rotate(float degree_roll, float degree_pitch, float degree_yaw);

	void Rotate_Roll(float degree);
	void Rotate_Pitch(float degree);
	void Rotate_Yaw(float degree);

	//
	void Bind_Camera(Camera* camera);
};

