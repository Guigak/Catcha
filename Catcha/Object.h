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
	float m_deceleration = 1000.0f;

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

	//
	std::vector<Mesh> m_meshes;

	DirectX::XMFLOAT4 m_rotate_quat = { 0.0f, 0.0f, 0.0f, 1.0f };

	//////////////////////////////////////////////////////////////////
	// [SC] 위치 보간을 위한 변수
	DirectX::XMFLOAT3 m_target_position{ 0, 0, 0 };						// 서버에서 받은 Position
	float m_lerp_position_progress = 0.0f;								// Position 선형 보간 진행도 (0.0~1.0)

	// [SC] 회전 보간을 위한 변수
	const float interp_duration = 0.05f;								// 보간 시간 상수 (20ms)

	// [SC] 회전 변화각 보낼때 사용하는 변수
	float m_last_sent_pitch = 0.0f;										// 마지막으로 보낸 Pitch 값
	const float m_pitch_send_delay = 0.05f;								// 100 ms (0.1 seconds)
	std::chrono::high_resolution_clock::time_point m_last_sent_time;	// 마지막으로 보낸 시간

	// [SC] 회전 변화각 받아서 보간에 사용하는 변수
    DirectX::XMFLOAT4 m_start_quat = { 0.0f, 0.0f, 0.0f, 1.0f };		// 보간 시작 쿼터니언 값
	DirectX::XMFLOAT4 m_target_quat = { 0.0f, 0.0f, 0.0f, 1.0f };		// 목표 Pitch 값
    float m_lerp_pitch_progress = 1.0f;									// Rotation 선형 보간 진행도 (0.0~1.0)
	bool m_change_pitch = false;										// Rotation 바꿔야하는지 여부
	float total_pitch = 0.0f;											// 각도 변화량의 합	

	//////////////////////////////////////////////////////////////////

public:
	Object() {}
	Object(std::wstring object_name, MeshInfo* mesh_info, std::wstring mesh_name,
		MaterialInfo* material_info, UINT constant_buffer_index, D3D12_PRIMITIVE_TOPOLOGY primitive_topology, bool physics, bool visiable);
	Object(std::wstring object_name, Mesh_Info* mesh, DirectX::XMMATRIX world_matrix, UINT constant_buffer_index, D3D12_PRIMITIVE_TOPOLOGY primitive_topology, bool physics, bool visiable);
	Object(std::wstring object_name, std::vector<Mesh>& mesh_array, UINT constant_buffer_index, D3D12_PRIMITIVE_TOPOLOGY primitive_topology, bool physics, bool visiable);
	~Object() {}

	void Set_Name(std::wstring object_name) { m_name = object_name; }
	void Set_Mesh_Info(MeshInfo* mesh_info, std::wstring mesh_name);
	void Set_Material_Info(MaterialInfo* material_info) { m_material_info = material_info; }
	void Set_CB_Index(UINT constant_buffer_index) { m_constant_buffer_index = constant_buffer_index; }
	void Set_PT(D3D12_PRIMITIVE_TOPOLOGY primitive_topology) { m_primitive_topology = primitive_topology; }
	void Set_Phys(bool physics) { m_physics = physics; }
	void Set_Visiable(bool visiable) { m_visiable = visiable; }

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

	std::wstring Get_Mesh_Name() { return m_submesh_name; }

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

	bool Get_Visiable() { return m_visiable; }

	void Calc_Delta(float elapsed_time);
	void Calc_Delta_Characters(float elapsed_time);
	//void Move_N_Solve_Collision();
	virtual void Update();

	void Udt_WM();	// Update World Matrix
	void Udt_LUR();	// Update Look Up Right

	// [SC] 위치 보간을 위한 함수
	void LerpPosition(float deltaTime);
	void SetTargetPosition(const DirectX::XMFLOAT3& newPosition);

	DirectX::XMFLOAT3 GetCameraLook();

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

	void Rotate_Network(float degree_roll, float degree_pitch, float degree_yaw);

	void Rotate_Character(float elapsed_time);

	// rotate
	void Rotate(float degree_roll, float degree_pitch, float degree_yaw);

	void Rotate_Roll(float degree);
	void Rotate_Pitch(float degree);
	void Rotate_Yaw(float degree);

	// [SC] 회전 보간을 위한 함수
	void LerpRotate(float deltaTime);
	void SetTargetPitch(float newpitch);

	//
	void Bind_Camera(Camera* camera);

	//
	void Add_Mesh(Mesh_Info* mesh_info, DirectX::XMFLOAT4X4 local_transform_matrix);
	void Add_Mesh(std::vector<Mesh>& mesh_array);

	void Set_WM(DirectX::XMMATRIX world_matrix);
	void Draw(ID3D12GraphicsCommandList* command_list);

	void Set_Look(DirectX::XMFLOAT3 look);

	// [SC] 캐릭터 변경을 위한 함수
	void Change_Character(uint8_t prev_num, uint8_t new_num);
};

