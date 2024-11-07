#pragma once
#include "common.h"

class Camera;
class ObjectManager;

class Object {
protected:
	std::wstring m_name = L"";

	DirectX::XMFLOAT3 m_position = { 0.0f, 0.0f, 0.0f };
	DirectX::XMFLOAT3 m_scale = { 0.0f, 0.0f, 0.0f };

	DirectX::XMFLOAT3 m_look = { 0.0f, 0.0f, 1.0f };
	DirectX::XMFLOAT3 m_up = { 0.0f, 1.0f, 0.0f };
	DirectX::XMFLOAT3 m_right = { 1.0f, 0.0f, 0.0f };

	DirectX::XMFLOAT4X4 m_world_matrix = MathHelper::Identity_4x4();

	UINT m_constant_buffer_index = -1;

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
	BYTE m_camera_rotate_synchronization_flag = ROTATE_SYNC_NONE;

	//
	DirectX::BoundingOrientedBox m_OBB;

	//
	std::vector<Mesh> m_meshes;
	Skeleton_Info* m_skeleton_info = nullptr;

	float m_rotate_right = 0.0f;
	float m_rotate_look = 0.0f;
	DirectX::XMFLOAT4 m_rotate_roll_pitch_yaw = { 0.0f, 0.0f, 0.0f, 1.0f };
	DirectX::XMFLOAT4 m_rotate = { 0.0f, 0.0f, 0.0f, 1.0f };

	//
	bool m_animated = false;
	float m_animated_time = 0.0f;
	std::array<DirectX::XMFLOAT4X4, MAX_BONE_COUNT> m_animation_matrix_array;
	std::wstring m_playing_animation_name = L"";
	std::wstring m_next_animation_name = L"";

	std::unordered_map<Object_State, std::wstring> m_animation_map;

	//
	ObjectManager* m_object_manager = nullptr;

public:
	Object() {}
	Object(ObjectManager* object_manager, std::wstring object_name, Mesh_Info* mesh, DirectX::XMMATRIX world_matrix, UINT constant_buffer_index, D3D12_PRIMITIVE_TOPOLOGY primitive_topology, bool physics, bool visiable);
	Object(ObjectManager* object_manager, std::wstring object_name, std::vector<Mesh>& mesh_array, UINT constant_buffer_index, D3D12_PRIMITIVE_TOPOLOGY primitive_topology, bool physics, bool visiable);
	~Object() {}

	void Set_Object_Manager(ObjectManager* object_manager) { m_object_manager = object_manager; }
	void Set_Name(std::wstring object_name) { m_name = object_name; }
	void Set_Material_Info(MaterialInfo* material_info) { m_material_info = material_info; }
	void Set_CB_Index(UINT constant_buffer_index) { m_constant_buffer_index = constant_buffer_index; }
	void Set_PT(D3D12_PRIMITIVE_TOPOLOGY primitive_topology) { m_primitive_topology = primitive_topology; }
	void Set_Phys(bool physics) { m_physics = physics; }
	void Set_Visiable(bool visiable) { m_visiable = visiable; }

	//
	DirectX::XMMATRIX Get_OBB_WM();
	DirectX::BoundingOrientedBox Get_Calcd_OBB();

	//
	std::wstring Get_Name() { return m_name; }

	DirectX::XMVECTOR Get_Position_V() { return DirectX::XMLoadFloat3(&m_position); }	// Get Position Vector
	DirectX::XMFLOAT3 Get_Position_3f() { return m_position; }	// Get Position float3

	DirectX::XMVECTOR Get_Rotate_V() { return DirectX::XMLoadFloat4(&m_rotate); }	// Get Rotate Vector
	DirectX::XMFLOAT4 Get_Rotate_4f() { return m_rotate; }	// Get Rotate float3
	DirectX::XMVECTOR Get_Rotate_RPY_V() { return DirectX::XMLoadFloat4(&m_rotate_roll_pitch_yaw); }
	DirectX::XMFLOAT4 Get_Rotate_RPY_4f() { return m_rotate_roll_pitch_yaw; }

	float Get_Rotate_Right() { return m_rotate_right; }
	float Get_Rotate_Look() { return m_rotate_look; }

	DirectX::XMVECTOR Get_Scale_V() { return DirectX::XMLoadFloat3(&m_scale); }	// Get Scale Vector
	DirectX::XMFLOAT3 Get_Scale_3f() { return m_scale; }	// Get Scale float3

	DirectX::XMFLOAT3 Get_Look() { return m_look; }
	DirectX::XMFLOAT3 Get_Up() { return m_up; }
	DirectX::XMFLOAT3 Get_Right() { return m_right; }

	DirectX::XMFLOAT4X4 Get_WM() { return m_world_matrix; }

	UINT Get_CB_Index() { return m_constant_buffer_index; }

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

	bool Get_Visiable() { return m_visiable; }

	void Calc_Delta(float elapsed_time);
	//void Move_N_Solve_Collision();
	virtual void Update(float elapsed_time);

	void Udt_WM();	// Update World Matrix
	void Udt_LUR();	// Update Look Up Right

	//
	void Set_Position(float position_x, float position_y, float position_z);

	// move
	void Move(DirectX::XMFLOAT3 direction);

	void Move_Forward(BYTE flag);
	void Move_Back(BYTE flag);
	void Move_Left(BYTE flag);
	void Move_Right(BYTE flag);
	void Move_Up(BYTE flag);
	void Move_Down(BYTE flag);

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

	void Rotate_Right(float degree);
	void Rotate_Look(float degree);

	//
	void Bind_Camera(Camera* camera);

	//
	void Add_Mesh(Mesh_Info* mesh_info, DirectX::XMFLOAT4X4 local_transform_matrix);
	void Add_Mesh(std::vector<Mesh>& mesh_array);

	std::vector<Mesh>& Get_Mesh_Array() { return m_meshes; }

	void Set_WM(DirectX::XMMATRIX world_matrix);
	void Draw(ID3D12GraphicsCommandList* command_list);

	//
	void Set_Skeleton(Skeleton_Info* skeleton_info) { m_skeleton_info = skeleton_info; }

	//
	void Set_Animated(bool animated) { m_animated = animated; }
	bool Get_Animated() { return m_animated; }

	void Set_Animation(std::wstring animation_name) {
		m_playing_animation_name = animation_name;
		m_animated_time = 0.0f;
	}

	std::array<DirectX::XMFLOAT4X4, MAX_BONE_COUNT>& Get_Animation_Matrix() { return m_animation_matrix_array; }

	//
	void Bind_Anim_2_State(Object_State object_state, std::wstring animation_name);

	//
	void Calc_Rotate();

	//
	void Set_Cam_Rotate_Flag(BYTE flag) { m_camera_rotate_synchronization_flag = flag; }
};

