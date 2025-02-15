#pragma once
#include "common.h"
#include "NetworkManager.h"

class Camera;
class ObjectManager;

class Object {
protected:
	std::wstring m_name = L"";

	DirectX::XMFLOAT3 m_position = { 0.0f, 0.0f, 0.0f };
	DirectX::XMFLOAT3 m_scale = { 0.0f, 0.0f, 0.0f };

	//
	DirectX::XMFLOAT3 m_additional_scale = { 0.0f, 0.0f, 0.0f };

	DirectX::XMFLOAT3 m_look = { 0.0f, 0.0f, 1.0f };
	DirectX::XMFLOAT3 m_up = { 0.0f, 1.0f, 0.0f };
	DirectX::XMFLOAT3 m_right = { 1.0f, 0.0f, 0.0f };

	DirectX::XMFLOAT4X4 m_world_matrix = MathHelper::Identity_4x4();	// not transposed

	UINT m_constant_buffer_index = -1;

	std::wstring m_submesh_name = L"";

	D3D12_PRIMITIVE_TOPOLOGY m_primitive_topology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	UINT m_index_count = 0;
	UINT m_start_index_location = 0;
	int m_base_vertex_location = 0;

	int m_dirty_count = FRAME_RESOURCES_NUMBER;

	bool m_dirty = false;

	//
	bool m_physics = false;

	float m_gravity = 980.0f;
	float m_jump_power = 500.0f;

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
	bool m_grounded = true;

	Object_State m_state = Object_State::STATE_IDLE;
	Object_State m_next_state = Object_State::STATE_IDLE;

	//
	Camera* m_camera = nullptr;

	bool m_visible = false;

	//
	BYTE m_camera_rotate_synchronization_flag = ROTATE_SYNC_NONE;

	//
	bool m_obb_object = false;
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
	std::array<Transform_Info, MAX_BONE_COUNT> m_blending_source_transform_info_array;
	std::array<DirectX::XMFLOAT4X4, MAX_BONE_COUNT> m_animation_matrix_array;

	std::unordered_map<Object_State, Animation_Binding_Info> m_animation_binding_map;

	//
	ObjectManager* m_object_manager = nullptr;

	//
	bool m_processable_input = true;
	bool m_movable = true;
	bool m_rotatable = true;

	//
	DirectX::XMFLOAT4 m_color_multiplier = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

	//
	DirectX::XMFLOAT3 m_additional_info = DirectX::XMFLOAT3();

	//
	void (*m_custom_function_one)();
	void (*m_custom_function_two)();
	void (*m_custom_function_three)();

	bool m_selectable = false;

	//
	bool m_shade = true;

	//
	std::unordered_map<std::wstring, bool> m_bool_values;
	std::unordered_map<std::wstring, int> m_int_values;
	std::unordered_map<std::wstring, float> m_float_values;

	//////////////////////////////////////////////////////////////////
	// [SC] 위치 보간을 위한 변수
	DirectX::XMFLOAT3 m_target_position{ 0, -999.0f, 0 };				// 서버에서 받은 Position
	float m_lerp_degree = 4.0f;											// 보간 수준 (작을수록 빠름) - 플레이어 기준 4.0f, AI 기준 50.0f

	// [SC] 회전 변화각 보낼때 사용하는 변수
	float m_last_sent_pitch = 0.0f;										// 마지막으로 보낸 Pitch 값
	const float m_pitch_send_delay = 0.05f;								// 50 ms (0.05 seconds)
	std::chrono::high_resolution_clock::time_point m_last_sent_time;	// 마지막으로 보낸 시간

	// [SC] 회전 변화각 받아서 보간에 사용하는 변수
    DirectX::XMFLOAT4 m_start_quat = { 0.0f, 0.0f, 0.0f, 1.0f };		// 보간 시작 쿼터니언 값
	DirectX::XMFLOAT4 m_target_quat = { 0.0f, 0.0f, 0.0f, 1.0f };		// 목표 Pitch 값
    float m_lerp_pitch_progress = 1.0f;									// Rotation 선형 보간 진행도 (0.0~1.0)
	bool m_change_pitch = false;										// Rotation 바꿔야하는지 여부
	float total_pitch = 0.0f;											// pitch 각도 변화량의 합	
	float total_yaw = 0.0f;												// yaw 각도 변화량의 합	

	// [SC] 캐릭터 번호
	int m_character_number = -1;										// 캐릭터 번호 (0~3 쥐, 4~7 ai, 8 고양이) - 고정된 캐릭터의 번호임
	bool m_is_need_send = false;										// 회전각 서버로 보내는지 여부
	//////////////////////////////////////////////////////////////////

public:
	Object() {}
	Object(ObjectManager* object_manager, std::wstring object_name, Mesh_Info* mesh, DirectX::XMMATRIX world_matrix, UINT constant_buffer_index, D3D12_PRIMITIVE_TOPOLOGY primitive_topology, bool physics, bool visible);
	Object(ObjectManager* object_manager, std::wstring object_name, std::vector<Mesh>& mesh_array, UINT constant_buffer_index, D3D12_PRIMITIVE_TOPOLOGY primitive_topology, bool physics, bool visible);
	virtual ~Object() {}

	void Set_Object_Manager(ObjectManager* object_manager) { m_object_manager = object_manager; }
	void Set_Name(std::wstring object_name) { m_name = object_name; }
	void Set_CB_Index(UINT constant_buffer_index) { m_constant_buffer_index = constant_buffer_index; }
	void Set_PT(D3D12_PRIMITIVE_TOPOLOGY primitive_topology) { m_primitive_topology = primitive_topology; }
	void Set_Phys(bool physics) { m_physics = physics; }
	void Set_Visible(bool visible) { m_visible = visible; }

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

	DirectX::XMFLOAT4X4 Get_WM_4x4f() { return m_world_matrix; }
	DirectX::XMMATRIX Get_WM_M() { return DirectX::XMLoadFloat4x4(&m_world_matrix); }

	UINT Get_CB_Index() { return m_constant_buffer_index; }

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

	void Set_Grounded(bool grounded) { m_grounded = grounded; }

	bool Get_Visible() { return m_visible; }

	void Calc_Delta(float elapsed_time);
	void Calc_Delta_Characters(float elapsed_time);
	//void Move_N_Solve_Collision();
	virtual void Update(float elapsed_time);

	void Udt_WM();	// Update World Matrix
	void Udt_LUR();	// Update Look Up Right

	// [SC] 위치 보간을 위한 함수
	void LerpPosition(float deltaTime);
	void SetTargetPosition(const DirectX::XMFLOAT3& newPosition);

	DirectX::XMFLOAT3 GetCameraLook();

	//
	void Set_Position(float position_x, float position_y, float position_z);
	void Set_Rotate(float rotate_x, float rotate_y, float rotate_z, float rotate_w);
	void Set_Scale(float scale_x, float scale_y, float scale_z);

	//
	void Set_Additional_Scale(float scale_x, float scale_y, float scale_z);

	void Set_Position(DirectX::XMFLOAT3 position);
	void Set_Rotate(DirectX::XMFLOAT4 rotate);
	void Set_Scale(DirectX::XMFLOAT3 scale);

	//
	void Rst_Rotate();

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

	void Rotate_Character(float elapsed_time);

	// rotate
	void Rotate(float degree_roll, float degree_pitch, float degree_yaw);

	void Rotate_Roll(float degree);
	void Rotate_Pitch(float degree);
	void Rotate_Yaw(float degree);

	void Rotate_Right(float degree);
	void Rotate_Look(float degree);

	//
	virtual void Jump();
	virtual void Act_One();
	virtual void Act_Two();
	virtual void Act_Three();
	virtual void Act_Four();
	virtual void Act_Five();

	// [SC] 회전 보간을 위한 함수
	void SetLerpDegree(float degree) { m_lerp_degree = degree; }
	void SendRotate(float pitch_degree, float yaw_degree);
	void LerpRotate(float deltaTime);
	void SetTargetPitch(float newpitch);
	void SetTargetQuat(const DirectX::XMFLOAT4& newQuat);

	//
	void Bind_Camera(Camera* camera);

	//
	void Add_Mesh(Mesh_Info* mesh_info, DirectX::XMFLOAT4X4 local_transform_matrix = MathHelper::Identity_4x4());
	void Add_Mesh(std::vector<Mesh>& mesh_array);

	std::vector<Mesh>& Get_Mesh_Array() { return m_meshes; }

	void Set_WM(DirectX::XMMATRIX world_matrix);
	virtual void Draw(ID3D12GraphicsCommandList* command_list);

	//
	void Set_Skeleton(Skeleton_Info* skeleton_info) { m_skeleton_info = skeleton_info; }

	//
	void Set_Animated(bool animated) { m_animated = animated; }
	bool Get_Animated() { return m_animated; }

	std::array<DirectX::XMFLOAT4X4, MAX_BONE_COUNT>& Get_Animation_Matrix() { return m_animation_matrix_array; }

	void Set_State(Object_State object_state) { m_state = object_state; }
	void Set_Next_State(Object_State object_state) { m_next_state = object_state; }
	Object_State Get_State() { return m_state; }

	//
	void Bind_Anim_2_State(Object_State object_state, Animation_Binding_Info animation_binding_info);

	//
	void Calc_Rotate();

	//
	void Set_Cam_Rotate_Flag(BYTE flag) { m_camera_rotate_synchronization_flag = flag; }
	void Reset_bind_Camera() { if(m_camera)	m_camera = nullptr; }

	//
	bool Get_Processable_Input() { return m_processable_input; }
	bool Get_Movable() { return m_movable; }
	bool Get_Rotatable() { return m_rotatable; }

	//
	void Set_OBB(DirectX::BoundingOrientedBox obb);
	void Set_OBB_Position(DirectX::XMFLOAT3 pos, DirectX::XMFLOAT4 rot);
	DirectX::BoundingOrientedBox Object::Get_OBB() { return m_OBB; }

	void Set_Look(DirectX::XMFLOAT4 quat);

	// 캐릭터 번호 설정
	void Set_Character_Number(int number) { m_character_number = number; }
	int Get_Character_Number() { return m_character_number; }

	// 카메라 회전값 서버로 보내는지 설정
	void Set_Camera_Need_Send(bool is_need_send) { m_is_need_send = is_need_send; }
	bool Get_Camera_Need_Send() { return m_is_need_send; }


	void Set_Color_Mul(DirectX::XMFLOAT4 color_rgb);
	void Set_Color_Mul(float color_r, float color_g, float color_b, float color_a);
	void Set_Color_Alpha(float color_a);
	DirectX::XMFLOAT4 Get_Color_Mul() { return m_color_multiplier; }

	//
	DirectX::XMFLOAT3 Get_Additional_Info() { return m_additional_info; }

	//
	void Set_Custom_Fuction_One(void(*custom_function)()) { m_custom_function_one = custom_function; }
	void Set_Custom_Fuction_Two(void(*custom_function)()) { m_custom_function_two = custom_function; }
	void Set_Custom_Fuction_Three(void(*custom_function)()) { m_custom_function_three = custom_function; }

	void Call_Custom_Function_One() { if (m_custom_function_one) { m_custom_function_one(); } }
	void Call_Custom_Function_Two() { if (m_custom_function_two) { m_custom_function_two(); } }
	void Call_Custom_Function_Three() { if (m_custom_function_three) { m_custom_function_three(); } }

	bool Get_Selectable() { return m_selectable; }
	void Set_Selectable(bool selectable) { m_selectable = selectable; }

	//
	virtual bool Picking(DirectX::XMFLOAT4 origin_ray, DirectX::XMFLOAT4 ray_direction,
		DirectX::XMFLOAT4X4 inverse_view_matrix, float picking_distance) { return false; }

	//
	bool Get_Shade() { return m_shade; }
	void Set_Shade(bool shade) { m_shade = shade; }

	//
	DirectX::XMFLOAT3* Get_Position_Addr() { return &m_position; }	// Get Position Address
	DirectX::XMFLOAT3* Get_Velocity_Addr() { return &m_velocity; }	// Get Position Address
	DirectX::XMFLOAT3* Get_Look_Addr() { return &m_look; }	// Get Position Address
	DirectX::XMFLOAT3* Get_Up_Addr() { return &m_up; }	// Get Position Address

	//
	bool Get_Bool_Value(std::wstring value_name) { return m_bool_values[value_name]; }
	int Get_Int_Value(std::wstring value_name) { return m_int_values[value_name]; }
	float Get_Float_Value(std::wstring value_name) { return m_float_values[value_name]; };
};

