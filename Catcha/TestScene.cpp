#include "TestScene.h"
#include "SceneManager.h"
#include "D3DManager.h"
#include "VoxelCheese.h"
#include "MapData.h"
#include "TextUIObject.h"
#include "ParticleObject.h"

#include "SoundManager.h"

std::unordered_map<std::wstring, ObjectOBB> g_obbData;

#define PASS_NUMBER 2

UINT m_voxel_count = 0;
bool m_render_silhouette = false;

//
Scene_State m_scene_state = Scene_State::MAIN_STATE;
Scene_State m_next_scene_state = Scene_State::MAIN_STATE;

Camera* camera;

bool is_player_cat = false;		// 고양이 조명을 위한 변수 true - cat / false - mouse


bool m_dissolve = false;

constexpr float MAX_DISSOLVE_VALUE = 1.5f;
float m_dissolve_value = 0.0f;

bool m_door_open = false;
constexpr float DOOR_OPEN_TIME = 3.0f;
float m_door_open_time_value = 0.0f;

bool m_Need_Lagging = false;
float Camera_Lagging_Time = 0.0f;
float m_camera_lagging_value = 0.0f;
float m_camera_target_lagging = 1.0f;

int m_observing_num = 0;

bool m_is_cat_winner = false;		// true - cat win / false - mouse win

bool m_is_reborn = false;
constexpr float REBORN_RESPOWN_TIME = 5.0f;
float m_reborn_timer_value = 0.0f;

//
bool m_attacked = false;
constexpr float MAX_ATTACKED_VALUE = 0.5f;
float m_attacked_value = 0.0f;

//
constexpr float MAX_PICKING_DISTANCE = 1.0f;

//
SoundManager* m_sound_manager = nullptr;

// ui functions
void Game_Start_UI_Function();
void Game_End_UI_Function();
void To_Main_UI_Function();
void Select_Cat_UI_Function();
void Select_Mouse_UI_Function();

// test //
void Result_Test_UI_Function() {
	m_next_scene_state = Scene_State::END_STATE;
}

void TestScene::Enter(D3DManager* d3d_manager) {
	m_total_time = 0.0f;

	m_object_manager = std::make_unique<ObjectManager>(this);
	m_input_manager = std::make_unique<InputManager>(this, m_object_manager.get(), d3d_manager->Get_Client_Width(), d3d_manager->Get_Client_Height());                                   

	ID3D12Device* device = d3d_manager->Get_Device();
	ID3D12GraphicsCommandList* command_list = d3d_manager->Get_Cmd_List();

	Resize(d3d_manager);

	d3d_manager->Rst_Cmd_List();

	// OBB 시각화용 메쉬 생성
	MapData* mapData = new MapData();
	if (mapData->LoadMapData("Map.txt"))
	{
		std::cout << "Map data loaded." << std::endl;
		delete mapData;
	}
	else
	{
		std::cout << "Failed to load map data." << std::endl;
	}

	//
	m_shadow_map = std::make_unique<Shadowmap>(device, 8192, 8192);

	Load_Texture(device, command_list);
	Build_RS(device);
	Build_S_N_L();
	Build_Material();
	Build_Mesh(device, command_list);
	Build_O();
	Build_C(d3d_manager);
	Build_FR(device);
	Build_DH(device);
	Build_CBV(d3d_manager);
	Build_PSO(d3d_manager);

	Binding_Key();
	Pairing_Collision_Set();

	d3d_manager->Cls_Cmd_List();
	d3d_manager->Exct_Cmd_List();
	d3d_manager->Flush_Cmd_Q();

	//
	m_shadow_bounding_sphere.Center = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_shadow_bounding_sphere.Radius = std::sqrtf(650.0f * 650.0f + 650.0f * 650.0f);

	//
	m_client_width = d3d_manager->Get_Client_Width();
	m_client_height = d3d_manager->Get_Client_Height();

	//
	m_sound_manager = SoundManager::Get_Inst();
	m_sound_manager->Add_Sound(L"bgm.mp3", FMOD_2D | FMOD_LOOP_NORMAL | FMOD_CREATESTREAM);
	m_sound_manager->Add_Sound(L"victory_sound.mp3", FMOD_2D);
	m_sound_manager->Add_Sound(L"hit_sound.mp3", FMOD_3D);
	m_sound_manager->Add_Sound(L"bell_sound.wav", FMOD_3D);
	m_sound_manager->Add_Sound(L"eating_sound.wav", FMOD_3D);
	m_sound_manager->Add_Sound(L"swing_sound.wav", FMOD_3D);
	//m_sound_manager->Play_Sound(L"bgm", L"Work_of_a_cat.mp3");

	m_sound_manager->Set_Listener(m_main_camera->Get_Position_Addr(), m_main_camera->Get_Look_Addr(), m_main_camera->Get_Up_Addr(), nullptr);
}

void TestScene::Exit(D3DManager* d3d_manager) {
	Flush_Cmd_Q(d3d_manager);
}

void TestScene::Update(D3DManager* d3d_manager, float elapsed_time) {
	if (m_dissolve == false) {
		m_input_manager->Prcs_Input();
	}

	m_object_manager->Update(elapsed_time);

	//
	if (m_scene_state != m_next_scene_state) {
		m_dissolve = true;

		if (m_dissolve_value >= MAX_DISSOLVE_VALUE) {
			m_scene_state = m_next_scene_state;
			Chg_Scene_State(m_scene_state);
		}
	}
	else {
		m_dissolve = false;
	}

	if (m_dissolve) {
		if (m_dissolve_value < MAX_DISSOLVE_VALUE) {
			m_dissolve_value += elapsed_time;

			m_sound_manager->Set_Master_Volume(1.0f - m_dissolve_value / MAX_DISSOLVE_VALUE);
		}
	}
	else {
		if (m_dissolve_value > 0.0f) {
			m_dissolve_value -= elapsed_time;

			m_sound_manager->Set_Master_Volume(1.0f - m_dissolve_value / MAX_DISSOLVE_VALUE);
		}
	}

	// 문 여는 이벤트
	if (true == m_door_open)
	{
		if (m_door_open_time_value < DOOR_OPEN_TIME / 2.0f)
		{
			m_door_open_time_value += elapsed_time;
		}
		else
		{
			//m_object_manager->Get_Obj(L"Door_003")->Rotate_Pitch(180.0f);
			m_object_manager->Get_Obj(L"Door_003")->Set_Visible(false);
			((ParticleObject*)m_object_manager->Get_Obj(L"particle"))->Add_Particle(
				m_object_manager->Get_Obj(L"Door_003")->Get_Position_3f(),
				DirectX::XMFLOAT3(3.0f, 3.0f, 3.0f),
				DirectX::XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f),
				100,
				m_total_time
			);
			m_door_open = false;
			m_door_open_time_value = 0.0f;
		}
	}

	// 카메라 레깅 설정 예약
	if (true == m_Need_Lagging)
	{
		if (m_camera_lagging_value < Camera_Lagging_Time)
		{
			m_camera_lagging_value += elapsed_time;
		}
		else
		{
			((Camera*)m_object_manager->Get_Obj(L"maincamera"))->Set_Lagging_Degree(m_camera_target_lagging);
			
			m_Need_Lagging = false;
			Camera_Lagging_Time = 0.0f;
			m_camera_lagging_value = 0.0f;
			m_camera_target_lagging = 1.0f;
			m_object_manager->Get_Obj(L"player")->Set_Visible(false);
		}
	}

	// 환생 ui 이벤트 설정
	if (true == m_is_reborn)
	{
		if (m_reborn_timer_value < REBORN_RESPOWN_TIME)
		{
			m_object_manager->Get_Obj(L"reborn_comment")->Set_Visible(true);
			m_object_manager->Get_Obj(L"reborn_timer")->Set_Visible(true);
			((TextUIObject*)m_object_manager->Get_Obj(L"reborn_timer"))->
				Set_Text(
					std::to_wstring(
						static_cast<int>(REBORN_RESPOWN_TIME - m_reborn_timer_value)
					));

			m_reborn_timer_value += elapsed_time;
		}
		else
		{
			m_object_manager->Get_Obj(L"reborn_comment")->Set_Visible(false);
			m_object_manager->Get_Obj(L"reborn_timer")->Set_Visible(false);
			m_is_reborn = false;
			m_reborn_timer_value = 0.0f;
		}
	}


	float dissolve_value = 1.0f - (float)m_dissolve_value / (float)MAX_DISSOLVE_VALUE;
	m_object_manager->Get_Obj(L"dissolve")->Set_Color_Mul(dissolve_value, dissolve_value, dissolve_value, 1.0f - dissolve_value);
	
	//
	if (m_attacked) {
		if (m_attacked_value < MAX_ATTACKED_VALUE) {
			m_attacked_value += elapsed_time;
		}
		else {
			m_attacked = false;
			m_attacked_value -= elapsed_time;
		}
	}
	else {
		if (m_attacked_value > 0.0f) {
			m_attacked_value -= elapsed_time;
		}
	}

	float alpha = (float)m_attacked_value / (float)MAX_ATTACKED_VALUE;
	m_object_manager->Get_Obj(L"attacked_ui")->Set_Color_Mul(1.0f, 1.0f, 1.0f, alpha);

	//
	Object* cat_object = m_object_manager->Get_Obj(L"cat");
	cat_object->Set_Color_Alpha(
		MathHelper::Min(1.0f, 1.0f - MathHelper::Length(MathHelper::Subtract(cat_object->Get_Position_3f(), m_object_manager->Get_Obj(L"player")->Get_Position_3f())) / 500.0f));

	m_current_frameresource_index = (m_current_frameresource_index + 1) % FRAME_RESOURCES_NUMBER;
	m_current_frameresource = m_frameresources[m_current_frameresource_index].get();

	UINT64 value = d3d_manager->Get_Fence()->GetCompletedValue();
	if (m_current_frameresource->fence != 0 && d3d_manager->Get_Fence()->GetCompletedValue() < m_current_frameresource->fence) {
		HANDLE event_handle = CreateEventEx(nullptr, false, false, EVENT_ALL_ACCESS);

		Throw_If_Failed(d3d_manager->Get_Fence()->SetEventOnCompletion(m_current_frameresource->fence, event_handle));

		WaitForSingleObject(event_handle, INFINITE);
		CloseHandle(event_handle);
	}

	// object
	auto current_object_constant_buffer = m_current_frameresource->object_constant_buffer.get();
	auto current_animation_constant_buffer = m_current_frameresource->animation_constant_buffer.get();

	for (auto& object : m_object_manager->Get_Obj_Arr()) {
		if (object->Get_Dirty_Count()) {
			DirectX::XMMATRIX world_matrix = DirectX::XMLoadFloat4x4(&object->Get_WM_4x4f());

			ObjectConstants object_constants;
			DirectX::XMStoreFloat4x4(&object_constants.world_matrix, DirectX::XMMatrixTranspose(world_matrix));
			object_constants.color_multiplier = object->Get_Color_Mul();
			object_constants.additional_info = object->Get_Additional_Info();
			object_constants.animated = (UINT)object->Get_Animated();

			current_object_constant_buffer->Copy_Data(object->Get_CB_Index(), object_constants);

			if (object_constants.animated) {
				AnimationConstants animation_constants;
				animation_constants.animation_transform_matrix = object->Get_Animation_Matrix();

				current_animation_constant_buffer->Copy_Data(object->Get_CB_Index(), animation_constants);
			}

			object->Sub_Dirty_Count();
		}
	}

	// material
	if (m_object_manager->Get_Material_Manager().Get_Dirty_Count()) {
		auto current_material_constant_buffer = m_current_frameresource->material_constant_buffer.get();

		for (auto& m : m_object_manager->Get_Material_Manager().Get_Material_Map()) {
			Material* material = m.second.get();

			MaterialConstants material_constants;
			material_constants.material_array = material->Get_Material_Factors();

			current_material_constant_buffer->Copy_Data(material->constant_buffer_index, material_constants);
		}

		m_object_manager->Get_Material_Manager().Sub_Dirty_Count();
	}

	// instance data
	{
		for (auto& o : m_object_manager->Get_Instc_Obj_Arr()) {
			InstanceObject* instance_object_pointer = (InstanceObject*)o;

			if (instance_object_pointer->Get_Instc_Dirty_Cnt()) {

				auto instance_data_buffer = m_current_frameresource->instance_data_buffer_array[instance_object_pointer->Get_Instance_Index()].get();
				std::vector<InstanceData> instance_data_array;
				instance_object_pointer->Get_Instance_Data(instance_data_array);

				for (UINT i = 0; i < instance_object_pointer->Get_Instance_Count(); ++i) {
					instance_data_buffer->Copy_Data(i, instance_data_array[i]);
				}

				instance_object_pointer->Sub_Instc_Dirty_Cnt();
			}
		}
	}

	// shadow transform matrix
	{
		DirectX::XMVECTOR light_direction = DirectX::XMVectorSet(0.2f, -1.0f, 0.2f, 0.0f);
		DirectX::XMVECTOR light_position = DirectX::XMVectorScale(light_direction, -2.0f * m_shadow_bounding_sphere.Radius);
		DirectX::XMVECTOR target_position = DirectX::XMLoadFloat3(&m_shadow_bounding_sphere.Center);
		DirectX::XMVECTOR light_up = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
		DirectX::XMMATRIX light_view_matrix = DirectX::XMMatrixLookAtLH(light_position, target_position, light_up);

		DirectX::XMStoreFloat3(&m_light_position, light_position);

		DirectX::XMFLOAT3 sphere_center_lightspace;
		DirectX::XMStoreFloat3(&sphere_center_lightspace, DirectX::XMVector3TransformCoord(target_position, light_view_matrix));

		float left_x = sphere_center_lightspace.x - m_shadow_bounding_sphere.Radius;
		float bottom_y = sphere_center_lightspace.y - m_shadow_bounding_sphere.Radius;
		float near_z = sphere_center_lightspace.z - m_shadow_bounding_sphere.Radius;
		float right_x = sphere_center_lightspace.x + m_shadow_bounding_sphere.Radius;
		float top_y = sphere_center_lightspace.y + m_shadow_bounding_sphere.Radius;
		float far_z = sphere_center_lightspace.z + m_shadow_bounding_sphere.Radius;

		m_light_near_z = near_z;
		m_light_far_z = far_z;
		DirectX::XMMATRIX light_projection_matrix = DirectX::XMMatrixOrthographicOffCenterLH(left_x, right_x, bottom_y, top_y, near_z, far_z);

		DirectX::XMMATRIX texture_matrix(
			0.5f, 0.0f, 0.0f, 0.0f,
			0.0f, -0.5f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.5f, 0.5f, 0.0f, 1.0f);

		DirectX::XMMATRIX shadow_transform_matrix = light_view_matrix * light_projection_matrix * texture_matrix;
		DirectX::XMStoreFloat4x4(&m_light_view_matrix, light_view_matrix);
		DirectX::XMStoreFloat4x4(&m_light_projection_matrix, light_projection_matrix);
		DirectX::XMStoreFloat4x4(&m_shadow_transform_matrix, shadow_transform_matrix);
	}

	// main pass constants
	if (m_main_camera) {
		m_main_camera->Udt_VM();

		m_camera_position = m_main_camera->Get_Position_3f();

		DirectX::XMStoreFloat4x4(&m_view_matrix, m_main_camera->Get_VM_M());
		DirectX::XMStoreFloat4x4(&m_projection_matrix, m_main_camera->Get_PM_M());
	}
	else {
		m_camera_position = DirectX::XMFLOAT3(0.0f, 300.0f, -500.0f);
		DirectX::XMVECTOR pos = DirectX::XMVectorSet(m_camera_position.x, m_camera_position.y, m_camera_position.z, 1.0f);
		DirectX::XMVECTOR target = DirectX::XMVectorZero();
		DirectX::XMVECTOR up = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

		DirectX::XMMATRIX view = DirectX::XMMatrixLookAtLH(pos, target, up);
		DirectX::XMStoreFloat4x4(&m_view_matrix, view);
	}

	DirectX::XMMATRIX view_matrix = DirectX::XMLoadFloat4x4(&m_view_matrix);
	DirectX::XMMATRIX projection_matrix = DirectX::XMLoadFloat4x4(&m_projection_matrix);
	DirectX::XMMATRIX view_projection_matrix = DirectX::XMMatrixMultiply(view_matrix, projection_matrix);

	DirectX::XMMATRIX inverse_view_matrix = DirectX::XMMatrixInverse(&DirectX::XMMatrixDeterminant(view_matrix), view_matrix);
	DirectX::XMMATRIX inverse_projection_matrix = DirectX::XMMatrixInverse(&DirectX::XMMatrixDeterminant(projection_matrix), projection_matrix);
	DirectX::XMMATRIX inverse_view_projection_matrix = DirectX::XMMatrixInverse(&DirectX::XMMatrixDeterminant(view_projection_matrix), view_projection_matrix);

	DirectX::XMMATRIX shadow_transform_matrix = DirectX::XMLoadFloat4x4(&m_shadow_transform_matrix);

	DirectX::XMStoreFloat4x4(&m_main_pass_constant_buffer.view_matrix, DirectX::XMMatrixTranspose(view_matrix));
	DirectX::XMStoreFloat4x4(&m_main_pass_constant_buffer.inverse_view_matrix, DirectX::XMMatrixTranspose(inverse_view_matrix));
	DirectX::XMStoreFloat4x4(&m_main_pass_constant_buffer.projection_matrix, DirectX::XMMatrixTranspose(projection_matrix));
	DirectX::XMStoreFloat4x4(&m_main_pass_constant_buffer.inverse_projection_matrix, DirectX::XMMatrixTranspose(inverse_projection_matrix));
	DirectX::XMStoreFloat4x4(&m_main_pass_constant_buffer.view_projection_matrix, DirectX::XMMatrixTranspose(view_projection_matrix));
	DirectX::XMStoreFloat4x4(&m_main_pass_constant_buffer.inverse_view_projection_matrix, DirectX::XMMatrixTranspose(inverse_view_projection_matrix));
	DirectX::XMStoreFloat4x4(&m_main_pass_constant_buffer.shadow_transform_matrix, DirectX::XMMatrixTranspose(shadow_transform_matrix));
	m_main_pass_constant_buffer.camera_position = m_camera_position;
	m_main_pass_constant_buffer.buffer_padding = 0.0f;
	m_main_pass_constant_buffer.render_target_size = DirectX::XMFLOAT2((float)d3d_manager->Get_Client_Width(), (float)d3d_manager->Get_Client_Height());
	m_main_pass_constant_buffer.inverse_render_target_size = DirectX::XMFLOAT2(1.0f / (float)d3d_manager->Get_Client_Width(), 1.0f / (float)d3d_manager->Get_Client_Height());
	m_main_pass_constant_buffer.near_z = 1.0f;
	m_main_pass_constant_buffer.far_z = 1000.0f;
	m_main_pass_constant_buffer.total_time = m_total_time;
	m_main_pass_constant_buffer.delta_time = 0.0f;
	m_main_pass_constant_buffer.ambient_light = { 0.25f, 0.25f, 0.35f, 1.0f };
	//
	m_main_pass_constant_buffer.lights[0].direction = { 0.2f, -1.0f, 0.2f };
	m_main_pass_constant_buffer.lights[0].strength = { 0.6f, 0.6f, 0.6f };
	//
	m_main_pass_constant_buffer.lights[1].position = m_object_manager->Get_Obj(L"maincamera")->Get_Position_3f();
	m_main_pass_constant_buffer.lights[1].direction = m_object_manager->Get_Obj(L"maincamera")->Get_Look();

	m_main_pass_constant_buffer.lights[1].falloff_start = 50.0f;
	m_main_pass_constant_buffer.lights[1].falloff_end = 200.0f;
	m_main_pass_constant_buffer.lights[1].spot_power = 64;
	//
	Skeleton_Info* skeleton_info = m_object_manager->Get_Skeleton_Manager().Get_Skeleton(L"cat_mesh_edit.fbx");
	DirectX::XMFLOAT4 light_position;
	DirectX::XMFLOAT4 light_direction;

	if (true == is_player_cat)
	{
		cat_object = m_object_manager->Get_Obj(L"player");
		m_main_pass_constant_buffer.lights[1].strength = { 0.0f, 0.0f, 0.0f };
	}
	else
	{
		cat_object = m_object_manager->Get_Obj(L"cat");
		m_main_pass_constant_buffer.lights[1].strength = { 0.75f, 0.75f, 0.75f };
	}


	DirectX::XMFLOAT4X4 transform_matrix = MathHelper::Multiply(MathHelper::Multiply(
		MathHelper::Inverse(skeleton_info->bone_offset_matrix_array[23]), MathHelper::Transpose(cat_object->Get_Animation_Matrix()[23])), cat_object->Get_WM_4x4f());
	light_position = MathHelper::Multiply(DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f), transform_matrix);
	light_direction = MathHelper::Normalize(MathHelper::Multiply(DirectX::XMFLOAT4(0.0f, 1.0f, 0.0f, 0.0f), transform_matrix));

	m_main_pass_constant_buffer.lights[2].position = { light_position.x, light_position.y, light_position.z };
	m_main_pass_constant_buffer.lights[2].direction = { light_direction.x, light_direction.y, light_direction.z };
	m_main_pass_constant_buffer.lights[2].strength = { 1.0f, 0.0f, 0.0f };
	m_main_pass_constant_buffer.lights[2].falloff_start = 100.0f;
	m_main_pass_constant_buffer.lights[2].falloff_end = 500.0f;
	m_main_pass_constant_buffer.lights[2].spot_power = 16.0f;
	//
	transform_matrix = MathHelper::Multiply(MathHelper::Multiply(
		MathHelper::Inverse(skeleton_info->bone_offset_matrix_array[24]), MathHelper::Transpose(cat_object->Get_Animation_Matrix()[24])), cat_object->Get_WM_4x4f());
	light_position = MathHelper::Multiply(DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f), transform_matrix);
	light_direction = MathHelper::Normalize(MathHelper::Multiply(DirectX::XMFLOAT4(0.0f, 1.0f, 0.0f, 0.0f), transform_matrix));

	m_main_pass_constant_buffer.lights[3].position = { light_position.x, light_position.y, light_position.z };
	m_main_pass_constant_buffer.lights[3].direction = { light_direction.x, light_direction.y, light_direction.z };
	m_main_pass_constant_buffer.lights[3].strength = { 1.0f, 0.0f, 0.0f };
	m_main_pass_constant_buffer.lights[3].falloff_start = 100.0f;
	m_main_pass_constant_buffer.lights[3].falloff_end = 500.0f;
	m_main_pass_constant_buffer.lights[3].spot_power = 16.0f;

	auto current_pass_constant_buffer = m_current_frameresource->pass_constant_buffer.get();
	current_pass_constant_buffer->Copy_Data(0, m_main_pass_constant_buffer);

	// shadow pass constants
	{
		DirectX::XMMATRIX view_matrix = DirectX::XMLoadFloat4x4(&m_light_view_matrix);
		DirectX::XMMATRIX projection_matrix = DirectX::XMLoadFloat4x4(&m_light_projection_matrix);
		DirectX::XMMATRIX view_projection_matrix = DirectX::XMMatrixMultiply(view_matrix, projection_matrix);

		DirectX::XMMATRIX inverse_view_matrix = DirectX::XMMatrixInverse(&DirectX::XMMatrixDeterminant(view_matrix), view_matrix);
		DirectX::XMMATRIX inverse_projection_matrix = DirectX::XMMatrixInverse(&DirectX::XMMatrixDeterminant(projection_matrix), projection_matrix);
		DirectX::XMMATRIX inverse_view_projection_matrix = DirectX::XMMatrixInverse(&DirectX::XMMatrixDeterminant(view_projection_matrix), view_projection_matrix);

		DirectX::XMStoreFloat4x4(&m_shadow_pass_constant_buffer.view_matrix, DirectX::XMMatrixTranspose(view_matrix));
		DirectX::XMStoreFloat4x4(&m_shadow_pass_constant_buffer.inverse_view_matrix, DirectX::XMMatrixTranspose(inverse_view_matrix));
		DirectX::XMStoreFloat4x4(&m_shadow_pass_constant_buffer.projection_matrix, DirectX::XMMatrixTranspose(projection_matrix));
		DirectX::XMStoreFloat4x4(&m_shadow_pass_constant_buffer.inverse_projection_matrix, DirectX::XMMatrixTranspose(inverse_projection_matrix));
		DirectX::XMStoreFloat4x4(&m_shadow_pass_constant_buffer.view_projection_matrix, DirectX::XMMatrixTranspose(view_projection_matrix));
		DirectX::XMStoreFloat4x4(&m_shadow_pass_constant_buffer.inverse_view_projection_matrix, DirectX::XMMatrixTranspose(inverse_view_projection_matrix));
		m_shadow_pass_constant_buffer.camera_position = m_light_position;
		m_shadow_pass_constant_buffer.render_target_size = DirectX::XMFLOAT2((float)m_shadow_map->Get_Width(), (float)m_shadow_map->Get_Height());
		m_shadow_pass_constant_buffer.inverse_render_target_size = DirectX::XMFLOAT2(1.0f / (float)m_shadow_map->Get_Width(), 1.0f / (float)m_shadow_map->Get_Height());
		m_shadow_pass_constant_buffer.near_z = m_light_near_z;
		m_shadow_pass_constant_buffer.far_z = m_light_far_z;

		auto current_pass_constant_buffer = m_current_frameresource->pass_constant_buffer.get();
		current_pass_constant_buffer->Copy_Data(1, m_shadow_pass_constant_buffer);
	}

	//
	m_total_time += elapsed_time;
}

void TestScene::Resize(D3DManager* d3d_manager) {
	DirectX::XMMATRIX projection_view = DirectX::XMMatrixPerspectiveFovLH(0.25f * MathHelper::Pi(), d3d_manager->Get_Aspect_Ratio(), 1.0f, 1000.0f);
	DirectX::XMStoreFloat4x4(&m_projection_matrix, projection_view);
}

void TestScene::Draw(D3DManager* d3d_manager, ID3D12CommandList** command_lists) {
	//ID3D12Device* device = d3d_manager->Get_Device();
	ID3D12CommandAllocator* command_allocator = m_current_frameresource->command_allocator.Get();
	ID3D12GraphicsCommandList* command_list = m_current_frameresource->command_list.Get();

	Throw_If_Failed(command_allocator->Reset());
	Throw_If_Failed(command_list->Reset(command_allocator, m_pipeline_state_map[L"shadow"].Get()));

	ID3D12DescriptorHeap* descriptor_heaps[] = { m_CBV_heap.Get() };
	command_list->SetDescriptorHeaps(_countof(descriptor_heaps), descriptor_heaps);

	command_list->SetGraphicsRootSignature(m_root_signature.Get());
	
	// shadow map
	{
		command_list->RSSetViewports(1, &m_shadow_map->Get_Viewport());
		command_list->RSSetScissorRects(1, &m_shadow_map->Get_Scissor_Rect());

		command_list->ResourceBarrier(1, &D3D12_RESOURCE_BARRIER_EX::Transition(m_shadow_map->Get_Resource(),
			D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_DEPTH_WRITE));

		command_list->ClearDepthStencilView(m_shadow_map->Get_DSV_CPU_Handle(),
			D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

		command_list->OMSetRenderTargets(0, nullptr, false, &m_shadow_map->Get_DSV_CPU_Handle());

		UINT pass_CBV_index = m_pass_CBV_offset + m_current_frameresource_index * PASS_NUMBER + 1;	// main : 0 / shaadow : 1
		auto pass_CBV_gpu_descriptor_handle = D3D12_GPU_DESCRIPTOR_HANDLE_EX(m_CBV_heap->GetGPUDescriptorHandleForHeapStart());
		pass_CBV_gpu_descriptor_handle.Get_By_Offset(pass_CBV_index, d3d_manager->Get_CBV_SRV_UAV_Descritpor_Size());
		command_list->SetGraphicsRootDescriptorTable(2, pass_CBV_gpu_descriptor_handle);

		//command_list->SetPipelineState(m_pipeline_state_map[L"shadow"].Get());

		//
		DirectX::BoundingFrustum frustum_OBB;
		DirectX::BoundingFrustum::CreateFromMatrix(frustum_OBB, XMFLOAT4X4_2_XMMATRIX(m_light_projection_matrix));
		frustum_OBB.Transform(frustum_OBB, MathHelper::Inverse(XMFLOAT4X4_2_XMMATRIX(m_light_view_matrix)));

		for (auto& object : m_object_manager->Get_Opaque_Obj_Arr()) {
			//if (object->Get_Name() == L"Ceiling") {
			//	continue;
			//}

			if (!object->Get_Shade()) {
				continue;
			}

			UINT object_CBV_index = m_current_frameresource_index * (UINT)m_object_manager->Get_Obj_Count() + object->Get_CB_Index();
			auto object_CBV_gpu_descriptor_handle = D3D12_GPU_DESCRIPTOR_HANDLE_EX(m_CBV_heap->GetGPUDescriptorHandleForHeapStart());
			object_CBV_gpu_descriptor_handle.Get_By_Offset(object_CBV_index, d3d_manager->Get_CBV_SRV_UAV_Descritpor_Size());

			//
			UINT animation_CBV_index = m_animation_CBV_offset + m_current_frameresource_index * (UINT)m_object_manager->Get_Obj_Count() + object->Get_CB_Index();
			auto animation_CBV_gpu_descriptor_handle = D3D12_GPU_DESCRIPTOR_HANDLE_EX(m_CBV_heap->GetGPUDescriptorHandleForHeapStart());
			animation_CBV_gpu_descriptor_handle.Get_By_Offset(animation_CBV_index, d3d_manager->Get_CBV_SRV_UAV_Descritpor_Size());

			//
			command_list->SetGraphicsRootDescriptorTable(0, object_CBV_gpu_descriptor_handle);
			command_list->SetGraphicsRootDescriptorTable(3, animation_CBV_gpu_descriptor_handle);

			//
			DirectX::BoundingOrientedBox mesh_OBB;

			//UINT material_CBV_index;
			for (auto& m : object->Get_Mesh_Array()) {
				//
				m.mesh_info->Get_OBB().Transform(mesh_OBB, object->Get_WM_M());

				if (frustum_OBB.Intersects(mesh_OBB) == false) {
					continue;
				}

				m.mesh_info->Draw(command_list);
			}
		}

		command_list->SetPipelineState(m_pipeline_state_map[L"instance_shadow"].Get());

		for (auto& object : m_object_manager->Get_Voxel_Cheese_Obj_Arr()) {
			if (!object->Get_Visible()) {
				continue;
			}

			VoxelCheese* voxel_cheese_pointer = (VoxelCheese*)object;

			UINT object_CBV_index = m_current_frameresource_index * (UINT)m_object_manager->Get_Obj_Count() + object->Get_CB_Index();
			auto object_CBV_gpu_descriptor_handle = D3D12_GPU_DESCRIPTOR_HANDLE_EX(m_CBV_heap->GetGPUDescriptorHandleForHeapStart());
			object_CBV_gpu_descriptor_handle.Get_By_Offset(object_CBV_index, d3d_manager->Get_CBV_SRV_UAV_Descritpor_Size());

			command_list->SetGraphicsRootDescriptorTable(0, object_CBV_gpu_descriptor_handle);

			UINT instance_data_SRV_index = m_instance_SRV_offset + m_current_frameresource_index * (UINT)m_object_manager->Get_Instc_Obj_Arr().size() + voxel_cheese_pointer->Get_Instance_Index();
			auto instance_data_SRV_gpu_descriptor_handle = D3D12_GPU_DESCRIPTOR_HANDLE_EX(m_CBV_heap->GetGPUDescriptorHandleForHeapStart());
			instance_data_SRV_gpu_descriptor_handle.Get_By_Offset(instance_data_SRV_index, d3d_manager->Get_CBV_SRV_UAV_Descritpor_Size());

			command_list->SetGraphicsRootDescriptorTable(6, instance_data_SRV_gpu_descriptor_handle);

			object->Draw(command_list);
		}

		command_list->ResourceBarrier(1, &D3D12_RESOURCE_BARRIER_EX::Transition(m_shadow_map->Get_Resource(),
			D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_GENERIC_READ));
	}

	// 
	d3d_manager->Set_RTV_N_DSV(command_list);

	d3d_manager->Set_VP(command_list);
	d3d_manager->Set_SR(command_list);

	d3d_manager->Clr_RTV(command_list);
	d3d_manager->Clr_DSV(command_list);

	if (m_wireframe) {
		command_list->SetPipelineState(m_pipeline_state_map[L"opaque_wireframe"].Get());
	}
	else {
		command_list->SetPipelineState(m_pipeline_state_map[L"opaque"].Get());
	}

	// test?
	{
		D3D12_GPU_DESCRIPTOR_HANDLE_EX texture_descriptor_handle(m_CBV_heap->GetGPUDescriptorHandleForHeapStart());
		texture_descriptor_handle.Get_By_Offset(m_texture_SRV_offset + m_texture_map[L"unicode_texture"]->buffer_index, d3d_manager->Get_CBV_SRV_UAV_Descritpor_Size());
		command_list->SetGraphicsRootDescriptorTable(5, texture_descriptor_handle);
	}
	{
		D3D12_GPU_DESCRIPTOR_HANDLE_EX texture_descriptor_handle(m_CBV_heap->GetGPUDescriptorHandleForHeapStart());
		texture_descriptor_handle.Get_By_Offset(m_texture_SRV_offset + m_texture_map[L"test_ui"]->buffer_index, d3d_manager->Get_CBV_SRV_UAV_Descritpor_Size());
		command_list->SetGraphicsRootDescriptorTable(7, texture_descriptor_handle);
	}

	//
	D3D12_GPU_DESCRIPTOR_HANDLE_EX shadow_map_descriptor_handle(m_CBV_heap->GetGPUDescriptorHandleForHeapStart());
	shadow_map_descriptor_handle.Get_By_Offset(m_shadow_map_SRV_offset, d3d_manager->Get_CBV_SRV_UAV_Descritpor_Size());
	command_list->SetGraphicsRootDescriptorTable(4, shadow_map_descriptor_handle);

	//
	DirectX::BoundingFrustum frustum_OBB;
	DirectX::BoundingFrustum::CreateFromMatrix(frustum_OBB, m_main_camera->Get_PM_M());
	frustum_OBB.Transform(frustum_OBB, MathHelper::Inverse(m_main_camera->Get_VM_M()));

	//
	UINT pass_CBV_index = m_pass_CBV_offset + m_current_frameresource_index * PASS_NUMBER;
	auto pass_CBV_gpu_descriptor_handle = D3D12_GPU_DESCRIPTOR_HANDLE_EX(m_CBV_heap->GetGPUDescriptorHandleForHeapStart());
	pass_CBV_gpu_descriptor_handle.Get_By_Offset(pass_CBV_index, d3d_manager->Get_CBV_SRV_UAV_Descritpor_Size());
	command_list->SetGraphicsRootDescriptorTable(2, pass_CBV_gpu_descriptor_handle);

	//
	UINT object_constant_buffer_size = Calc_CB_Size(sizeof(ObjectConstants));

	auto object_constant_buffer = m_current_frameresource->object_constant_buffer->Get_Resource();

	{
		// draw cat mesh
		Object* cat_object = m_object_manager->Get_Obj(L"cat");
		command_list->OMSetStencilRef(1);

		if (cat_object->Get_Visible()) {
			UINT object_CBV_index = m_current_frameresource_index * (UINT)m_object_manager->Get_Obj_Count() + cat_object->Get_CB_Index();
			auto object_CBV_gpu_descriptor_handle = D3D12_GPU_DESCRIPTOR_HANDLE_EX(m_CBV_heap->GetGPUDescriptorHandleForHeapStart());
			object_CBV_gpu_descriptor_handle.Get_By_Offset(object_CBV_index, d3d_manager->Get_CBV_SRV_UAV_Descritpor_Size());

			//
			UINT animation_CBV_index = m_animation_CBV_offset + m_current_frameresource_index * (UINT)m_object_manager->Get_Obj_Count() + cat_object->Get_CB_Index();
			auto animation_CBV_gpu_descriptor_handle = D3D12_GPU_DESCRIPTOR_HANDLE_EX(m_CBV_heap->GetGPUDescriptorHandleForHeapStart());
			animation_CBV_gpu_descriptor_handle.Get_By_Offset(animation_CBV_index, d3d_manager->Get_CBV_SRV_UAV_Descritpor_Size());

			//
			command_list->SetGraphicsRootDescriptorTable(0, object_CBV_gpu_descriptor_handle);
			command_list->SetGraphicsRootDescriptorTable(3, animation_CBV_gpu_descriptor_handle);

			UINT material_CBV_index;
			for (auto& m : cat_object->Get_Mesh_Array()) {
				if (m.mesh_info->material == nullptr) {
					material_CBV_index = m_material_CBV_offset +
						m_current_frameresource_index * (UINT)m_object_manager->Get_Material_Manager().Get_Material_Count() + 0;
				}
				else {
					material_CBV_index = m_material_CBV_offset +
						m_current_frameresource_index * (UINT)m_object_manager->Get_Material_Manager().Get_Material_Count() +
						m.mesh_info->material->constant_buffer_index;
				}

				auto material_CBV_gpu_descriptor_handle = D3D12_GPU_DESCRIPTOR_HANDLE_EX(m_CBV_heap->GetGPUDescriptorHandleForHeapStart());
				material_CBV_gpu_descriptor_handle.Get_By_Offset(material_CBV_index, d3d_manager->Get_CBV_SRV_UAV_Descritpor_Size());

				command_list->SetGraphicsRootDescriptorTable(1, material_CBV_gpu_descriptor_handle);

				m.mesh_info->Draw(command_list);
			}
		}
	}

	// draw else
	command_list->OMSetStencilRef(2);

	for (auto& object : m_object_manager->Get_Opaque_Obj_Arr()) {
		if (object->Get_Name() == L"cat") {
			continue;
		}

		if (!object->Get_Visible()) {
			continue;
		}

		UINT object_CBV_index = m_current_frameresource_index * (UINT)m_object_manager->Get_Obj_Count() + object->Get_CB_Index();
		auto object_CBV_gpu_descriptor_handle = D3D12_GPU_DESCRIPTOR_HANDLE_EX(m_CBV_heap->GetGPUDescriptorHandleForHeapStart());
		object_CBV_gpu_descriptor_handle.Get_By_Offset(object_CBV_index, d3d_manager->Get_CBV_SRV_UAV_Descritpor_Size());

		//
		UINT animation_CBV_index = m_animation_CBV_offset + m_current_frameresource_index * (UINT)m_object_manager->Get_Obj_Count() + object->Get_CB_Index();
		auto animation_CBV_gpu_descriptor_handle = D3D12_GPU_DESCRIPTOR_HANDLE_EX(m_CBV_heap->GetGPUDescriptorHandleForHeapStart());
		animation_CBV_gpu_descriptor_handle.Get_By_Offset(animation_CBV_index, d3d_manager->Get_CBV_SRV_UAV_Descritpor_Size());

		//
		command_list->SetGraphicsRootDescriptorTable(0, object_CBV_gpu_descriptor_handle);
		command_list->SetGraphicsRootDescriptorTable(3, animation_CBV_gpu_descriptor_handle);

		//
		DirectX::BoundingOrientedBox mesh_OBB;

		UINT material_CBV_index;
		for (auto& m : object->Get_Mesh_Array()) {
			//
			m.mesh_info->Get_OBB().Transform(mesh_OBB, object->Get_WM_M());

			if (frustum_OBB.Intersects(mesh_OBB) == false) {
				continue;
			}

			if (m.mesh_info->material == nullptr) {
				material_CBV_index = m_material_CBV_offset +
					m_current_frameresource_index * (UINT)m_object_manager->Get_Material_Manager().Get_Material_Count() + 0;
			}
			else {
				material_CBV_index = m_material_CBV_offset +
					m_current_frameresource_index * (UINT)m_object_manager->Get_Material_Manager().Get_Material_Count() +
					m.mesh_info->material->constant_buffer_index;
			}

			auto material_CBV_gpu_descriptor_handle = D3D12_GPU_DESCRIPTOR_HANDLE_EX(m_CBV_heap->GetGPUDescriptorHandleForHeapStart());
			material_CBV_gpu_descriptor_handle.Get_By_Offset(material_CBV_index, d3d_manager->Get_CBV_SRV_UAV_Descritpor_Size());

			command_list->SetGraphicsRootDescriptorTable(1, material_CBV_gpu_descriptor_handle);

			m.mesh_info->Draw(command_list);
		}
	}

	// draw voxel cheese
	if (m_wireframe) {
		command_list->SetPipelineState(m_pipeline_state_map[L"instance_wireframe"].Get());
	}
	else {
		command_list->SetPipelineState(m_pipeline_state_map[L"instance"].Get());
	}

	for (auto& object : m_object_manager->Get_Voxel_Cheese_Obj_Arr()) {
		if (!object->Get_Visible()) {
			continue;
		}

		InstanceObject* instance_object_pointer = (InstanceObject*)object;

		UINT object_CBV_index = m_current_frameresource_index * (UINT)m_object_manager->Get_Obj_Count() + object->Get_CB_Index();
		auto object_CBV_gpu_descriptor_handle = D3D12_GPU_DESCRIPTOR_HANDLE_EX(m_CBV_heap->GetGPUDescriptorHandleForHeapStart());
		object_CBV_gpu_descriptor_handle.Get_By_Offset(object_CBV_index, d3d_manager->Get_CBV_SRV_UAV_Descritpor_Size());

		command_list->SetGraphicsRootDescriptorTable(0, object_CBV_gpu_descriptor_handle);

		UINT instance_data_SRV_index = m_instance_SRV_offset + m_current_frameresource_index * (UINT)m_object_manager->Get_Instc_Obj_Arr().size() + instance_object_pointer->Get_Instance_Index();
		auto instance_data_SRV_gpu_descriptor_handle = D3D12_GPU_DESCRIPTOR_HANDLE_EX(m_CBV_heap->GetGPUDescriptorHandleForHeapStart());
		instance_data_SRV_gpu_descriptor_handle.Get_By_Offset(instance_data_SRV_index, d3d_manager->Get_CBV_SRV_UAV_Descritpor_Size());

		command_list->SetGraphicsRootDescriptorTable(6, instance_data_SRV_gpu_descriptor_handle);

		//
		Mesh& mesh = object->Get_Mesh_Array()[0];
		UINT material_CBV_index;

		if (mesh.mesh_info->material == nullptr) {
			material_CBV_index = m_material_CBV_offset +
				m_current_frameresource_index * (UINT)m_object_manager->Get_Material_Manager().Get_Material_Count() + 0;
		}
		else {
			material_CBV_index = m_material_CBV_offset +
				m_current_frameresource_index * (UINT)m_object_manager->Get_Material_Manager().Get_Material_Count() +
				mesh.mesh_info->material->constant_buffer_index;
		}

		auto material_CBV_gpu_descriptor_handle = D3D12_GPU_DESCRIPTOR_HANDLE_EX(m_CBV_heap->GetGPUDescriptorHandleForHeapStart());
		material_CBV_gpu_descriptor_handle.Get_By_Offset(material_CBV_index, d3d_manager->Get_CBV_SRV_UAV_Descritpor_Size());

		command_list->SetGraphicsRootDescriptorTable(1, material_CBV_gpu_descriptor_handle);

		//
		object->Draw(command_list);
	}

	// draw transparent objects

	// draw siluet
	if (m_render_silhouette) {
		command_list->SetPipelineState(m_pipeline_state_map[L"silhouette"].Get());

		Object* cat_object = m_object_manager->Get_Obj(L"cat");

		UINT object_CBV_index = m_current_frameresource_index * (UINT)m_object_manager->Get_Obj_Count() + cat_object->Get_CB_Index();
		auto object_CBV_gpu_descriptor_handle = D3D12_GPU_DESCRIPTOR_HANDLE_EX(m_CBV_heap->GetGPUDescriptorHandleForHeapStart());
		object_CBV_gpu_descriptor_handle.Get_By_Offset(object_CBV_index, d3d_manager->Get_CBV_SRV_UAV_Descritpor_Size());

		//
		UINT animation_CBV_index = m_animation_CBV_offset + m_current_frameresource_index * (UINT)m_object_manager->Get_Obj_Count() + cat_object->Get_CB_Index();
		auto animation_CBV_gpu_descriptor_handle = D3D12_GPU_DESCRIPTOR_HANDLE_EX(m_CBV_heap->GetGPUDescriptorHandleForHeapStart());
		animation_CBV_gpu_descriptor_handle.Get_By_Offset(animation_CBV_index, d3d_manager->Get_CBV_SRV_UAV_Descritpor_Size());

		//
		command_list->SetGraphicsRootDescriptorTable(0, object_CBV_gpu_descriptor_handle);
		command_list->SetGraphicsRootDescriptorTable(3, animation_CBV_gpu_descriptor_handle);

		UINT material_CBV_index;
		for (auto& m : cat_object->Get_Mesh_Array()) {
			material_CBV_index = m_material_CBV_offset +
				m_current_frameresource_index * (UINT)m_object_manager->Get_Material_Manager().Get_Material_Count() + 0;

			auto material_CBV_gpu_descriptor_handle = D3D12_GPU_DESCRIPTOR_HANDLE_EX(m_CBV_heap->GetGPUDescriptorHandleForHeapStart());
			material_CBV_gpu_descriptor_handle.Get_By_Offset(material_CBV_index, d3d_manager->Get_CBV_SRV_UAV_Descritpor_Size());

			command_list->SetGraphicsRootDescriptorTable(1, material_CBV_gpu_descriptor_handle);

			m.mesh_info->Draw(command_list);
		}
		//
	}
	
	// draw particle
	command_list->SetPipelineState(m_pipeline_state_map[L"particle"].Get());

	for (auto& object : m_object_manager->Get_Particle_Obj_Arr()) {
		if (!object->Get_Visible()) {
			continue;
		}

		InstanceObject* instance_object_pointer = (InstanceObject*)object;

		UINT object_CBV_index = m_current_frameresource_index * (UINT)m_object_manager->Get_Obj_Count() + object->Get_CB_Index();
		auto object_CBV_gpu_descriptor_handle = D3D12_GPU_DESCRIPTOR_HANDLE_EX(m_CBV_heap->GetGPUDescriptorHandleForHeapStart());
		object_CBV_gpu_descriptor_handle.Get_By_Offset(object_CBV_index, d3d_manager->Get_CBV_SRV_UAV_Descritpor_Size());

		command_list->SetGraphicsRootDescriptorTable(0, object_CBV_gpu_descriptor_handle);

		UINT instance_data_SRV_index = m_instance_SRV_offset + m_current_frameresource_index * (UINT)m_object_manager->Get_Instc_Obj_Arr().size() + instance_object_pointer->Get_Instance_Index();
		auto instance_data_SRV_gpu_descriptor_handle = D3D12_GPU_DESCRIPTOR_HANDLE_EX(m_CBV_heap->GetGPUDescriptorHandleForHeapStart());
		instance_data_SRV_gpu_descriptor_handle.Get_By_Offset(instance_data_SRV_index, d3d_manager->Get_CBV_SRV_UAV_Descritpor_Size());

		command_list->SetGraphicsRootDescriptorTable(6, instance_data_SRV_gpu_descriptor_handle);

		//
		Mesh& mesh = object->Get_Mesh_Array()[0];
		UINT material_CBV_index;

		if (mesh.mesh_info->material == nullptr) {
			material_CBV_index = m_material_CBV_offset +
				m_current_frameresource_index * (UINT)m_object_manager->Get_Material_Manager().Get_Material_Count() + 0;
		}
		else {
			material_CBV_index = m_material_CBV_offset +
				m_current_frameresource_index * (UINT)m_object_manager->Get_Material_Manager().Get_Material_Count() +
				mesh.mesh_info->material->constant_buffer_index;
		}

		auto material_CBV_gpu_descriptor_handle = D3D12_GPU_DESCRIPTOR_HANDLE_EX(m_CBV_heap->GetGPUDescriptorHandleForHeapStart());
		material_CBV_gpu_descriptor_handle.Get_By_Offset(material_CBV_index, d3d_manager->Get_CBV_SRV_UAV_Descritpor_Size());

		command_list->SetGraphicsRootDescriptorTable(1, material_CBV_gpu_descriptor_handle);

		//
		object->Draw(command_list);
	}

	// draw wireframe bounding box
	if (m_render_boundingbox) {
		d3d_manager->Clr_DSV(command_list);
		command_list->SetPipelineState(m_pipeline_state_map[L"opaque_wireframe"].Get());

		for (auto& object : m_object_manager->Get_Col_OBB_Obj_Arr()) {
			UINT object_CBV_index = m_current_frameresource_index * (UINT)m_object_manager->Get_Obj_Count() + object->Get_CB_Index();
			auto object_CBV_gpu_descriptor_handle = D3D12_GPU_DESCRIPTOR_HANDLE_EX(m_CBV_heap->GetGPUDescriptorHandleForHeapStart());
			object_CBV_gpu_descriptor_handle.Get_By_Offset(object_CBV_index, d3d_manager->Get_CBV_SRV_UAV_Descritpor_Size());

			//
			command_list->SetGraphicsRootDescriptorTable(0, object_CBV_gpu_descriptor_handle);

			UINT material_CBV_index;
			for (auto& m : object->Get_Mesh_Array()) {
				if (m.mesh_info->material == nullptr) {
					material_CBV_index = m_material_CBV_offset +
						m_current_frameresource_index * (UINT)m_object_manager->Get_Material_Manager().Get_Material_Count() + 0;
				}
				else {
					material_CBV_index = m_material_CBV_offset +
						m_current_frameresource_index * (UINT)m_object_manager->Get_Material_Manager().Get_Material_Count() +
						m.mesh_info->material->constant_buffer_index;
				}

				auto material_CBV_gpu_descriptor_handle = D3D12_GPU_DESCRIPTOR_HANDLE_EX(m_CBV_heap->GetGPUDescriptorHandleForHeapStart());
				material_CBV_gpu_descriptor_handle.Get_By_Offset(material_CBV_index, d3d_manager->Get_CBV_SRV_UAV_Descritpor_Size());

				command_list->SetGraphicsRootDescriptorTable(1, material_CBV_gpu_descriptor_handle);

				m.mesh_info->Draw(command_list);
			}
		}
	}

	// draw ui
	d3d_manager->Clr_DSV(command_list);

	{
		command_list->SetPipelineState(m_pipeline_state_map[L"ui"].Get());

		for (auto& object : m_object_manager->Get_UI_Obj_Arr()) {
			if (!object->Get_Visible()) {
				continue;
			}

			UINT object_CBV_index = m_current_frameresource_index * (UINT)m_object_manager->Get_Obj_Count() + object->Get_CB_Index();
			auto object_CBV_gpu_descriptor_handle = D3D12_GPU_DESCRIPTOR_HANDLE_EX(m_CBV_heap->GetGPUDescriptorHandleForHeapStart());
			object_CBV_gpu_descriptor_handle.Get_By_Offset(object_CBV_index, d3d_manager->Get_CBV_SRV_UAV_Descritpor_Size());

			//
			command_list->SetGraphicsRootDescriptorTable(0, object_CBV_gpu_descriptor_handle);

			Mesh& mesh = object->Get_Mesh_Array()[0];
			UINT material_CBV_index;

			if (mesh.mesh_info->material == nullptr) {
				material_CBV_index = m_material_CBV_offset +
					m_current_frameresource_index * (UINT)m_object_manager->Get_Material_Manager().Get_Material_Count() + 0;
			}
			else {
				material_CBV_index = m_material_CBV_offset +
					m_current_frameresource_index * (UINT)m_object_manager->Get_Material_Manager().Get_Material_Count() +
					mesh.mesh_info->material->constant_buffer_index;
			}

			auto material_CBV_gpu_descriptor_handle = D3D12_GPU_DESCRIPTOR_HANDLE_EX(m_CBV_heap->GetGPUDescriptorHandleForHeapStart());
			material_CBV_gpu_descriptor_handle.Get_By_Offset(material_CBV_index, d3d_manager->Get_CBV_SRV_UAV_Descritpor_Size());

			command_list->SetGraphicsRootDescriptorTable(1, material_CBV_gpu_descriptor_handle);

			//
			object->Draw(command_list);
		}
	}

	// draw text ui
	command_list->SetPipelineState(m_pipeline_state_map[L"text_ui"].Get());

	for (auto& object : m_object_manager->Get_Text_UI_Obj_Arr()) {
		if (!object->Get_Visible()) {
			continue;
		}

		InstanceObject* instance_object_pointer = (InstanceObject*)object;

		UINT object_CBV_index = m_current_frameresource_index * (UINT)m_object_manager->Get_Obj_Count() + object->Get_CB_Index();
		auto object_CBV_gpu_descriptor_handle = D3D12_GPU_DESCRIPTOR_HANDLE_EX(m_CBV_heap->GetGPUDescriptorHandleForHeapStart());
		object_CBV_gpu_descriptor_handle.Get_By_Offset(object_CBV_index, d3d_manager->Get_CBV_SRV_UAV_Descritpor_Size());

		command_list->SetGraphicsRootDescriptorTable(0, object_CBV_gpu_descriptor_handle);

		UINT instance_data_SRV_index = m_instance_SRV_offset + m_current_frameresource_index * (UINT)m_object_manager->Get_Instc_Obj_Arr().size() + instance_object_pointer->Get_Instance_Index();
		auto instance_data_SRV_gpu_descriptor_handle = D3D12_GPU_DESCRIPTOR_HANDLE_EX(m_CBV_heap->GetGPUDescriptorHandleForHeapStart());
		instance_data_SRV_gpu_descriptor_handle.Get_By_Offset(instance_data_SRV_index, d3d_manager->Get_CBV_SRV_UAV_Descritpor_Size());

		command_list->SetGraphicsRootDescriptorTable(6, instance_data_SRV_gpu_descriptor_handle);

		//
		Mesh& mesh = object->Get_Mesh_Array()[0];
		UINT material_CBV_index;

		if (mesh.mesh_info->material == nullptr) {
			material_CBV_index = m_material_CBV_offset +
				m_current_frameresource_index * (UINT)m_object_manager->Get_Material_Manager().Get_Material_Count() + 0;
		}
		else {
			material_CBV_index = m_material_CBV_offset +
				m_current_frameresource_index * (UINT)m_object_manager->Get_Material_Manager().Get_Material_Count() +
				mesh.mesh_info->material->constant_buffer_index;
		}

		auto material_CBV_gpu_descriptor_handle = D3D12_GPU_DESCRIPTOR_HANDLE_EX(m_CBV_heap->GetGPUDescriptorHandleForHeapStart());
		material_CBV_gpu_descriptor_handle.Get_By_Offset(material_CBV_index, d3d_manager->Get_CBV_SRV_UAV_Descritpor_Size());

		command_list->SetGraphicsRootDescriptorTable(1, material_CBV_gpu_descriptor_handle);

		//
		object->Draw(command_list);
	}

	// close
	Throw_If_Failed(command_list->Close());

	command_lists[0] = command_list;

	m_current_frameresource->fence = d3d_manager->Get_Curr_Fence() + 1;
}

void TestScene::Prcs_Input_Msg(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam) {
	m_input_manager->Prcs_Input_Msg(hwnd, message, wparam, lparam);
}

void TestScene::Load_Texture(ID3D12Device* device, ID3D12GraphicsCommandList* command_list) {
	auto unicode_texture = std::make_unique<Texture_Info>();
	unicode_texture->name = L"unicode_texture";
	unicode_texture->file_name = L"unicode_texture.dds";
	unicode_texture->buffer_index = (UINT)m_texture_map.size();
	Throw_If_Failed(TextureLoader::Create_DDS_Texture_From_File(
		device, command_list,
		unicode_texture->file_name.c_str(), unicode_texture->resource, unicode_texture->upload_heap));

	m_texture_map[unicode_texture->name] = std::move(unicode_texture);
	
	//
	auto ui_texture = std::make_unique<Texture_Info>();
	ui_texture->name = L"test_ui";
	ui_texture->file_name = L"ui_texture_r.dds";
	//ui_texture->file_name = L"ui_texture.dds";
	//ui_texture->file_name = L"ui_sample.dds";
	ui_texture->buffer_index = (UINT)m_texture_map.size();
	Throw_If_Failed(TextureLoader::Create_DDS_Texture_From_File(
		device, command_list,
		ui_texture->file_name.c_str(), ui_texture->resource, ui_texture->upload_heap));

	m_texture_map[ui_texture->name] = std::move(ui_texture);
}

void TestScene::Build_RS(ID3D12Device* device) {
	D3D12_DESCRIPTOR_RANGE_EX desriptor_range_0;
	desriptor_range_0.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);

	D3D12_DESCRIPTOR_RANGE_EX desriptor_range_1;
	desriptor_range_1.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 1);

	D3D12_DESCRIPTOR_RANGE_EX desriptor_range_2;
	desriptor_range_2.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 2);

	D3D12_DESCRIPTOR_RANGE_EX desriptor_range_3;
	desriptor_range_3.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 3);

	D3D12_DESCRIPTOR_RANGE_EX desriptor_range_4;
	desriptor_range_4.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);	// shadow map

	D3D12_DESCRIPTOR_RANGE_EX desriptor_range_5;
	desriptor_range_5.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1);	//  unicode texture

	D3D12_DESCRIPTOR_RANGE_EX desriptor_range_6;
	desriptor_range_6.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 1);	// instance

	D3D12_DESCRIPTOR_RANGE_EX desriptor_range_7;
	desriptor_range_7.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 2);	// texture

	D3D12_ROOT_PARAMETER_EX root_parameters[8];

	root_parameters[0].Init_As_DT(1, &desriptor_range_0);
	root_parameters[1].Init_As_DT(1, &desriptor_range_1);
	root_parameters[2].Init_As_DT(1, &desriptor_range_2);
	root_parameters[3].Init_As_DT(1, &desriptor_range_3);
	root_parameters[4].Init_As_DT(1, &desriptor_range_4, D3D12_SHADER_VISIBILITY_PIXEL);
	root_parameters[5].Init_As_DT(1, &desriptor_range_5, D3D12_SHADER_VISIBILITY_PIXEL);
	root_parameters[6].Init_As_DT(1, &desriptor_range_6);
	root_parameters[7].Init_As_DT(1, &desriptor_range_7, D3D12_SHADER_VISIBILITY_PIXEL);

	std::array<D3D12_STATIC_SAMPLER_DESC_EX, 3> sampler_desc;
	sampler_desc[0] = D3D12_STATIC_SAMPLER_DESC_EX(
		0,
		D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT,
		D3D12_TEXTURE_ADDRESS_MODE_BORDER,
		D3D12_TEXTURE_ADDRESS_MODE_BORDER,
		D3D12_TEXTURE_ADDRESS_MODE_BORDER,
		0.0f,
		16,
		D3D12_COMPARISON_FUNC_LESS_EQUAL,
		D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK);
	sampler_desc[1] = D3D12_STATIC_SAMPLER_DESC_EX(
		1,
		D3D12_FILTER_MIN_MAG_MIP_POINT,
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,
		0.0f,
		1);
	sampler_desc[2] = D3D12_STATIC_SAMPLER_DESC_EX(
		2,
		D3D12_FILTER_ANISOTROPIC,
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,
		0.0f,
		8);

	D3D12_ROOT_SIGNATURE_DESC_EX root_signature_desc(8, root_parameters,
		(UINT)sampler_desc.size(), sampler_desc.data(), D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	Microsoft::WRL::ComPtr<ID3DBlob> serialized_root_signature = nullptr;
	Microsoft::WRL::ComPtr<ID3DBlob> error_blob = nullptr;

	HRESULT hr = S_OK;

	hr = D3D12SerializeRootSignature(&root_signature_desc, D3D_ROOT_SIGNATURE_VERSION_1,
		serialized_root_signature.GetAddressOf(), error_blob.GetAddressOf());

	if (error_blob != nullptr) {
		OutputDebugStringA((char*)error_blob->GetBufferPointer());
	}

	Throw_If_Failed(hr);

	Throw_If_Failed(device->CreateRootSignature(0, serialized_root_signature->GetBufferPointer(),
		serialized_root_signature->GetBufferSize(), IID_PPV_ARGS(m_root_signature.GetAddressOf())));
}

void TestScene::Build_S_N_L() {
	m_shader_map[L"standard_VS"] = Compile_Shader(L"shaders\\shaders.hlsl", nullptr, "VS", "vs_5_1");
	m_shader_map[L"opaque_PS"] = Compile_Shader(L"shaders\\shaders.hlsl", nullptr, "PS", "ps_5_1");

	m_shader_map[L"silhouette_PS"] = Compile_Shader(L"shaders\\shaders.hlsl", nullptr, "Silhouette_PS", "ps_5_1");

	m_shader_map[L"shadow_VS"] = Compile_Shader(L"shaders\\shaders.hlsl", nullptr, "Shadow_VS", "vs_5_1");
	m_shader_map[L"shadow_PS"] = Compile_Shader(L"shaders\\shaders.hlsl", nullptr, "Shadow_PS", "ps_5_1");

	m_shader_map[L"instance_VS"] = Compile_Shader(L"shaders\\shaders.hlsl", nullptr, "Instance_VS", "vs_5_1");

	m_shader_map[L"instance_shadow_VS"] = Compile_Shader(L"shaders\\shaders.hlsl", nullptr, "Instance_Shadow_VS", "vs_5_1");

	m_shader_map[L"text_ui_VS"] = Compile_Shader(L"shaders\\shaders.hlsl", nullptr, "Text_UI_VS", "vs_5_1");
	m_shader_map[L"text_ui_PS"] = Compile_Shader(L"shaders\\shaders.hlsl", nullptr, "Text_UI_PS", "ps_5_1");

	m_shader_map[L"ui_VS"] = Compile_Shader(L"shaders\\shaders.hlsl", nullptr, "UI_VS", "vs_5_1");
	m_shader_map[L"ui_PS"] = Compile_Shader(L"shaders\\shaders.hlsl", nullptr, "UI_PS", "ps_5_1");

	m_shader_map[L"particle_VS"] = Compile_Shader(L"shaders\\shaders.hlsl", nullptr, "Particle_VS", "vs_5_1");
	m_shader_map[L"particle_GS"] = Compile_Shader(L"shaders\\shaders.hlsl", nullptr, "Particle_GS", "gs_5_1");
	m_shader_map[L"particle_PS"] = Compile_Shader(L"shaders\\shaders.hlsl", nullptr, "Particle_PS", "ps_5_1");

	m_input_layouts = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "UV", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 36, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "BONECOUNT", 0, DXGI_FORMAT_R32_UINT, 0, 44, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "BONEINDICES", 0, DXGI_FORMAT_R32G32B32A32_UINT, 0, 48, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "BONEWEIGHTS", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 64, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "MATERIAL", 0, DXGI_FORMAT_R32_UINT, 0, 80, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};
}

void TestScene::Build_Mesh(ID3D12Device* device, ID3D12GraphicsCommandList* command_list) {
	//
	Mesh_Info* mesh_info;
	mesh_info = m_object_manager->Get_Mesh_Manager().Crt_Box_Mesh(L"boundingbox");
	mesh_info->material = m_object_manager->Get_Material_Manager().Get_Material(L"boundingbox");

	mesh_info = m_object_manager->Get_Mesh_Manager().Crt_Box_Mesh(L"cheese");
	mesh_info->material = m_object_manager->Get_Material_Manager().Get_Material(L"cheese");

	mesh_info = m_object_manager->Get_Mesh_Manager().Crt_Box_Mesh(L"zero_box", 0.0f, 0.0f, 0.0f);

	mesh_info = m_object_manager->Get_Mesh_Manager().Crt_Wall_Plane_Mesh(L"ui");
	mesh_info->material = m_object_manager->Get_Material_Manager().Get_Material(L"ui");

	mesh_info = m_object_manager->Get_Mesh_Manager().Crt_Point_Mesh(L"point");

	m_object_manager->Ipt_From_FBX(L"cat_mesh_edit.fbx", true, false, true, MESH_INFO | SKELETON_INFO | MATERIAL_INFO);
	m_object_manager->Ipt_From_FBX(L"cat_walk.fbx", true, false, true, ANIMATION_INFO, L"cat_mesh_edit.fbx");
	m_object_manager->Ipt_From_FBX(L"cat_run.fbx", true, false, true, ANIMATION_INFO, L"cat_mesh_edit.fbx");
	m_object_manager->Ipt_From_FBX(L"cat_idle.fbx", true, false, true, ANIMATION_INFO, L"cat_mesh_edit.fbx");
	m_object_manager->Ipt_From_FBX(L"cat_jump_test.fbx", true, false, true, ANIMATION_INFO, L"cat_mesh_edit.fbx");
	m_object_manager->Ipt_From_FBX(L"cat_bite.fbx", true, false, true, ANIMATION_INFO, L"cat_mesh_edit.fbx");
	m_object_manager->Ipt_From_FBX(L"cat_paw.fbx", true, false, true, ANIMATION_INFO, L"cat_mesh_edit.fbx");
	m_object_manager->Ipt_From_FBX(L"cat_jump_ready.fbx", true, false, true, ANIMATION_INFO, L"cat_mesh_edit.fbx");
	m_object_manager->Ipt_From_FBX(L"cat_jump_test_start.fbx", true, false, true, ANIMATION_INFO, L"cat_mesh_edit.fbx");
	m_object_manager->Ipt_From_FBX(L"cat_jump_test_idle.fbx", true, false, true, ANIMATION_INFO, L"cat_mesh_edit.fbx");
	m_object_manager->Ipt_From_FBX(L"cat_jump_test_end.fbx", true, false, true, ANIMATION_INFO, L"cat_mesh_edit.fbx");
	m_object_manager->Ipt_From_FBX(L"cat_damage.fbx", true, false, true, ANIMATION_INFO, L"cat_mesh_edit.fbx");
	m_object_manager->Ipt_From_FBX(L"cat_win_0.fbx", true, false, true, ANIMATION_INFO, L"cat_mesh_edit.fbx");
	m_object_manager->Ipt_From_FBX(L"cat_lose_0.fbx", true, false, true, ANIMATION_INFO, L"cat_mesh_edit.fbx");

	m_object_manager->Ipt_From_FBX(L"mouse_mesh_edit.fbx", true, false, true, MESH_INFO | SKELETON_INFO | MATERIAL_INFO);
	m_object_manager->Ipt_From_FBX(L"mouse_death.fbx", true, false, true, ANIMATION_INFO, L"mouse_mesh_edit.fbx");
	m_object_manager->Ipt_From_FBX(L"mouse_hit.fbx", true, false, true, ANIMATION_INFO, L"mouse_mesh_edit.fbx");
	//m_object_manager->Ipt_From_FBX(L"mouse_jump.fbx", true, false, true, ANIMATION_INFO, L"mouse_mesh_edit.fbx");
	m_object_manager->Ipt_From_FBX(L"mouse_run.fbx", true, false, true, ANIMATION_INFO, L"mouse_mesh_edit.fbx");
	m_object_manager->Ipt_From_FBX(L"mouse_walk.fbx", true, false, true, ANIMATION_INFO, L"mouse_mesh_edit.fbx");
	m_object_manager->Ipt_From_FBX(L"mouse_idle.fbx", true, false, true, ANIMATION_INFO, L"mouse_mesh_edit.fbx");
	m_object_manager->Ipt_From_FBX(L"mouse_jump_start.fbx", true, false, true, ANIMATION_INFO, L"mouse_mesh_edit.fbx");
	m_object_manager->Ipt_From_FBX(L"mouse_jump_idle.fbx", true, false, true, ANIMATION_INFO, L"mouse_mesh_edit.fbx");
	m_object_manager->Ipt_From_FBX(L"mouse_jump_end.fbx", true, false, true, ANIMATION_INFO, L"mouse_mesh_edit.fbx");
	m_object_manager->Ipt_From_FBX(L"mouse_win_0.fbx", true, false, true, ANIMATION_INFO, L"mouse_mesh_edit.fbx");
	m_object_manager->Ipt_From_FBX(L"mouse_lose_0.fbx", true, false, true, ANIMATION_INFO, L"mouse_mesh_edit.fbx");

	m_object_manager->Ipt_From_FBX(L"housee.fbx", false, true, false, MESH_INFO | MATERIAL_INFO);
	m_object_manager->Get_Obj(L"Ceiling")->Set_Shade(false);

	m_object_manager->Build_BV(device, command_list);
}

void TestScene::Build_Material() {
	//
	m_object_manager->Get_Mesh_Manager().Crt_Default_Box();
	m_object_manager->Get_Material_Manager().Add_Material(L"boundingbox", Material_Factor(DirectX::XMFLOAT4(DirectX::Colors::LightGreen)));
	m_object_manager->Get_Material_Manager().Add_Material(L"cheese", Material_Factor(DirectX::XMFLOAT4(DirectX::Colors::Yellow)));
	m_object_manager->Get_Material_Manager().Add_Material(L"ui", Material_Factor(DirectX::XMFLOAT4(DirectX::Colors::White)));
}

void TestScene::Build_O() {
	Object* object;

	object = m_object_manager->Add_Obj(L"main_scene_object", L"zero_box");
	object->Set_Position(50.0f, -61.592f, 140.0f);
	object->Set_Visible(false);

	object = m_object_manager->Add_Obj(L"end_scene_object", L"zero_box");
	object->Set_Position(-80.0f, 0.0f, -450.0f);
	object->Set_Visible(false);



	for (int i = 0; i < 2; ++i) {
		m_object_manager->Add_Obj(L"cat_model_" + std::to_wstring(i), L"cat_mesh_edit.fbx")->Set_Visible(true);
		m_object_manager->Set_Sklt_2_Obj(L"cat_model_" + std::to_wstring(i), L"cat_mesh_edit.fbx");
	}

	for (int i = 0; i < 9; ++i) {
		m_object_manager->Add_Obj(L"mouse_model_" + std::to_wstring(i), L"mouse_mesh_edit.fbx")->Set_Visible(true);
		m_object_manager->Set_Sklt_2_Obj(L"mouse_model_" + std::to_wstring(i), L"mouse_mesh_edit.fbx");
	}



	object = m_object_manager->Get_Obj(L"cat_model_0");
	object->Set_Position(100.0f, -61.592f, 200.0f);
	object->Rotate_Pitch(4.0f);
	object->Bind_Anim_2_State(Object_State::STATE_IDLE, Animation_Binding_Info(L"cat_idle.fbx", 1.0f, 0.2f, LOOP_ANIMATION));
	object->Set_Animated(true);

	object = m_object_manager->Get_Obj(L"cat_model_1");
	object->Set_Position(-30.0f, -61.592f, -350.0f);
	object->Rotate_Pitch(4.0f);
	object->Bind_Anim_2_State(Object_State::STATE_IDLE, Animation_Binding_Info(L"cat_win_0.fbx", 1.0f, 0.2f, LOOP_ANIMATION));
	object->Set_Animated(true);
	object->Set_Visible(false);
	object->Set_Shade(false);

	object = m_object_manager->Get_Obj(L"mouse_model_0");
	object->Set_Position(80.0f, -61.592f, 200.0f);
	object->Rotate_Pitch(4.0f);
	object->Bind_Anim_2_State(Object_State::STATE_IDLE, Animation_Binding_Info(L"mouse_idle.fbx", 1.0f, 0.2f, LOOP_ANIMATION));
	object->Set_Animated(true);
	for (int i = 1; i < 9; ++i) {
		object = m_object_manager->Get_Obj(L"mouse_model_" + std::to_wstring(i));
		object->Set_Position(-30.0f, -61.592f + (float)(i - 1) * 4.0f, -350.0f);
		object->Rotate_Pitch(4.0f);
		object->Bind_Anim_2_State(Object_State::STATE_IDLE, Animation_Binding_Info(L"mouse_idle.fbx", 1.0f, 0.2f, LOOP_ANIMATION));
		object->Set_Animated(true);
	}


	m_object_manager->Add_Obj(L"mouse0", L"mouse_mesh_edit.fbx", L"Object", DirectX::XMMatrixIdentity(), D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST, ObjectType::CHARACTER_OBJECT, true);
	m_object_manager->Add_Obj(L"mouse1", L"mouse_mesh_edit.fbx", L"Object", DirectX::XMMatrixIdentity(), D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST, ObjectType::CHARACTER_OBJECT, true);
	m_object_manager->Add_Obj(L"mouse2", L"mouse_mesh_edit.fbx", L"Object", DirectX::XMMatrixIdentity(), D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST, ObjectType::CHARACTER_OBJECT, true);
	m_object_manager->Add_Obj(L"mouse3", L"mouse_mesh_edit.fbx", L"Object", DirectX::XMMatrixIdentity(), D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST, ObjectType::CHARACTER_OBJECT, true);
	m_object_manager->Add_Obj(L"mouse4", L"mouse_mesh_edit.fbx", L"Object", DirectX::XMMatrixIdentity(), D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST, ObjectType::CHARACTER_OBJECT, true);
	m_object_manager->Add_Obj(L"mouse5", L"mouse_mesh_edit.fbx", L"Object", DirectX::XMMatrixIdentity(), D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST, ObjectType::CHARACTER_OBJECT, true);
	m_object_manager->Add_Obj(L"mouse6", L"mouse_mesh_edit.fbx", L"Object", DirectX::XMMatrixIdentity(), D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST, ObjectType::CHARACTER_OBJECT, true);
	m_object_manager->Add_Obj(L"mouse7", L"mouse_mesh_edit.fbx", L"Object", DirectX::XMMatrixIdentity(), D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST, ObjectType::CHARACTER_OBJECT, true);
	m_object_manager->Add_Obj(L"cat", L"cat_mesh_edit.fbx", L"Object", DirectX::XMMatrixIdentity(), D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST, ObjectType::CHARACTER_OBJECT, true);


	m_object_manager->Set_Sklt_2_Obj(L"mouse0", L"mouse_mesh_edit.fbx");
	m_object_manager->Set_Sklt_2_Obj(L"mouse1", L"mouse_mesh_edit.fbx");
	m_object_manager->Set_Sklt_2_Obj(L"mouse2", L"mouse_mesh_edit.fbx");
	m_object_manager->Set_Sklt_2_Obj(L"mouse3", L"mouse_mesh_edit.fbx");
	m_object_manager->Set_Sklt_2_Obj(L"mouse4", L"mouse_mesh_edit.fbx");
	m_object_manager->Set_Sklt_2_Obj(L"mouse5", L"mouse_mesh_edit.fbx");
	m_object_manager->Set_Sklt_2_Obj(L"mouse6", L"mouse_mesh_edit.fbx");
	m_object_manager->Set_Sklt_2_Obj(L"mouse7", L"mouse_mesh_edit.fbx");
	m_object_manager->Set_Sklt_2_Obj(L"cat", L"cat_mesh_edit.fbx");


	for (int i = 0; i < 8; i++)
	{
		object = m_object_manager->Get_Obj(L"mouse" + std::to_wstring(i));
		object->Bind_Anim_2_State(Object_State::STATE_IDLE, Animation_Binding_Info(L"mouse_idle.fbx", 1.0f, 0.2f, LOOP_ANIMATION));
		object->Bind_Anim_2_State(Object_State::STATE_MOVE, Animation_Binding_Info(L"mouse_walk.fbx", 1.0f, 0.2f, LOOP_ANIMATION));
		object->Bind_Anim_2_State(Object_State::STATE_JUMP_START, Animation_Binding_Info(L"mouse_jump_start.fbx", 0.5f, 0.2f, ONCE_ANIMATION, Object_State::STATE_JUMP_IDLE));
		object->Bind_Anim_2_State(Object_State::STATE_JUMP_IDLE, Animation_Binding_Info(L"mouse_jump_idle.fbx", 0.5f, 0.2f, LOOP_ANIMATION));
		object->Bind_Anim_2_State(Object_State::STATE_JUMP_END, Animation_Binding_Info(L"mouse_jump_end.fbx", 0.5f, 0.2f, ONCE_ANIMATION, Object_State::STATE_IDLE));
		object->Bind_Anim_2_State(Object_State::STATE_ACTION_ONE, Animation_Binding_Info(L"mouse_hit.fbx", 0.5f, 0.2f, ONCE_ANIMATION, Object_State::STATE_IDLE));
		object->Bind_Anim_2_State(Object_State::STATE_DEAD, Animation_Binding_Info(L"mouse_death.fbx", 0.5f, 0.2f, ONCE_ANIMATION, Object_State::STATE_NONE, Restriction_Option::Restrict_All));
		object->Set_Animated(true);
		object->Set_Phys(true);
		object->TP_Down(999.0f);
		if (i > NUM_MOUSE4)
		{
			object->TP_Up(999.0f + FLOOR_Y - 10.0f);
			object->SetTargetPosition(DirectX::XMFLOAT3(0.0f, FLOOR_Y - 10.0f, 0.0f));
			object->SetLerpDegree(50.0f);
		}
	}

	object = m_object_manager->Get_Obj(L"cat");
	object->Bind_Anim_2_State(Object_State::STATE_IDLE, Animation_Binding_Info(L"cat_idle.fbx", 1.0f, 0.2f, LOOP_ANIMATION));
	object->Bind_Anim_2_State(Object_State::STATE_MOVE, Animation_Binding_Info(L"cat_walk.fbx", 1.0f, 0.2f, LOOP_ANIMATION));
	object->Bind_Anim_2_State(Object_State::STATE_JUMP_START, Animation_Binding_Info(L"cat_jump_test_start.fbx", 0.5f, 0.2f, ONCE_ANIMATION, Object_State::STATE_JUMP_IDLE));
	object->Bind_Anim_2_State(Object_State::STATE_JUMP_IDLE, Animation_Binding_Info(L"cat_jump_test_idle.fbx", 0.5f, 0.2f, LOOP_ANIMATION));
	object->Bind_Anim_2_State(Object_State::STATE_JUMP_END, Animation_Binding_Info(L"cat_jump_test_end.fbx", 0.5f, 0.2f, ONCE_ANIMATION, Object_State::STATE_IDLE));
	object->Bind_Anim_2_State(Object_State::STATE_ACTION_ONE, Animation_Binding_Info(L"cat_paw.fbx", 1.0f, 0.2f, ONCE_ANIMATION, Object_State::STATE_IDLE, Restriction_Option::Restrict_Move));
	object->Set_Animated(true);
	object->Set_Phys(true);
	object->TP_Down(999.0f);

	m_object_manager->Add_Obj(L"player", L"mouse_mesh_edit.fbx", L"Object", DirectX::XMMatrixIdentity(), D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST, ObjectType::CHARACTER_OBJECT, true);
	m_object_manager->Set_Sklt_2_Obj(L"player", L"mouse_mesh_edit.fbx");

	m_object_manager->Get_Obj(L"player")->Set_Visible(false);


	
	for (const auto& obj : g_obbData)
	{
		m_object_manager->Add_Col_OBB_Obj(obj.second.name,
			DirectX::BoundingOrientedBox(
				obj.second.obb.Center,
				obj.second.obb.Extents,
				obj.second.obb.Orientation)
		);
	}

	
	NetworkManager& network = NetworkManager::GetInstance();

	for (int i = 0; i < CHEESE_NUM; ++i)
	{
		m_object_manager->Add_Voxel_Cheese(L"cheese" + std::to_wstring(i), CHEESE_POS[i], 1.0f, 1);
	}

	// main scene ui
	object = m_object_manager->Add_Text_UI_Obj(L"game_start", -0.8f, -0.3f, 0.2f, 0.2f, true, true);
	object->Set_Color_Mul(0.0f, 0.0f, 0.0f, 1.0f);
	object->Set_Custom_Fuction_One(Game_Start_UI_Function);
	((TextUIObject*)object)->Set_Text(L"게임 시작");

	object = m_object_manager->Add_Text_UI_Obj(L"game_end", -0.8f, -0.7f, 0.2f, 0.2f, true, true);
	object->Set_Color_Mul(0.0f, 0.0f, 0.0f, 1.0f);
	object->Set_Custom_Fuction_One(Game_End_UI_Function);
	((TextUIObject*)object)->Set_Text(L"게임 종료");

	object = m_object_manager->Add_UI_Obj(L"catcha_title", -0.3f, 0.4f, 1.0f, 0.75f,
		2880, 1755, 0.0f, 1920.0f, 405.0f, 2880.0f, false, true);

	// matching scene
	object = m_object_manager->Add_Text_UI_Obj(L"select_animal", -0.85f, 0.8f, 0.2f, 0.2f, false, false);
	object->Set_Color_Mul(1.0f, 1.0f, 0.0f, 1.0f);
	((TextUIObject*)object)->Set_Text(L"동물 선택");

	object = m_object_manager->Add_Text_UI_Obj(L"to_main", -0.9f, -0.85f, 0.1f, 0.1f, true, false);
	object->Set_Color_Mul(0.0f, 0.0f, 0.0f, 1.0f);
	object->Set_Custom_Fuction_One(To_Main_UI_Function);
	((TextUIObject*)object)->Set_Text(L"메인으로");

	object = m_object_manager->Add_UI_Obj(L"select_cat", -0.5f, -0.05f, 0.75f, 1.25f,
		2880, 1755, 405.0f, 1920.0f, 1080.0f, 2640.0f, true, false);
	object->Set_Custom_Fuction_One(Select_Cat_UI_Function);

	object = m_object_manager->Add_UI_Obj(L"select_mouse", 0.5f, -0.05f, 0.75f, 1.25f,
		2880, 1755, 1080.0f, 1920.0f, 1755.0f, 2640.0f, true, false);
	object->Set_Custom_Fuction_One(Select_Mouse_UI_Function);

	// game play scene ui
	object = m_object_manager->Add_Text_UI_Obj(L"aim_circle", 0.0f, 0.0f, 0.02f, 0.02f, false, false);
	object->Set_Color_Mul(1.0f, 1.0f, 0.0f, 1.0f);
	((TextUIObject*)object)->Set_Text(L"○");

	object = m_object_manager->Add_Text_UI_Obj(L"timer", -0.1f, 0.9f, 0.1f, 0.09f, false, false);
	object->Set_Color_Mul(0.0f, 0.0f, 0.0f, 1.0f);
	((TextUIObject*)object)->Set_Text(L"대기중..");
	network.SetTimeObject(*(TextUIObject*)object);

	object = m_object_manager->Add_Text_UI_Obj(L"reborn_comment", -0.25f, 0.3f, 0.1f, 0.09f, false, false);
	object->Set_Color_Mul(1.0f, 0.0f, 1.0f, 0.8f);
	((TextUIObject*)object)->Set_Text(L"환생까지 남은 시간");
	object = m_object_manager->Add_Text_UI_Obj(L"reborn_timer", 0.0f, 0.0f, 0.5f, 0.3f, false, false);
	object->Set_Color_Mul(1.0f, 0.0f, 0.0f, 1.0f);
	((TextUIObject*)object)->Set_Text(L"5");

	object = m_object_manager->Add_UI_Obj(L"attacked_ui", 0.0f, 0.0f, 2.0f, 2.0f,
		2880, 1755, 1080.0f, 0.0f, 1620, 960, false, false);

	object = m_object_manager->Add_UI_Obj(L"game_play_ui", 0.0f, 0.0f, 2.0f, 2.0f,
		2880, 1755, 0.0f, 0.0f, 1080.0f, 1920.0f, false, false);

	for (int i = 0; i < 20; ++i) {
		object = m_object_manager->Add_UI_Obj(L"gauge_ui_" + std::to_wstring(i), -1.0f + 0.22f + (float)i * 0.02f, -1.0f + ((100.0f + 1.0f) / 540.0f), 15.0f / 960.0f, 70.0f / 540.0f,
			2880, 1755, 425.0f, 1940.0f, 426.0f, 1941.0f, false, false);
	}

	object = m_object_manager->Add_UI_Obj(L"cat_portrait", -1.0f + (100.0f / 960.0f), -1.0f + ((100.0f + 1.0f) / 540.0f), 300.0f / 960.0f, 300.0f / 540.0f,
		2880, 1755, 1080.0f, 960.0f, 1380.0f, 1260.0f, false, false);

	object = m_object_manager->Add_UI_Obj(L"mouse_portrait", -1.0f + (100.0f / 960.0f), -1.0f + ((100.0f + 1.0f) / 540.0f), 300.0f / 960.0f, 300.0f / 540.0f,
		2880, 1755, 1080.0f, 1260.0f, 1380.0f, 1560.0f, false, false);

	for (int i = 0; i < 4; ++i) {
		object = m_object_manager->Add_UI_Obj(L"mouse_icon_" + std::to_wstring(i), -(0.675f - (float)i * 0.125f), 1.0f - (50.0f / 540.0f), 100.0f / 960.0f, 100.0f / 540.0f,
			2880, 1755, 1380.0f, 961.0f, 1680.0f, 1260.0f, false, false);
		network.AddMouseUIObject(*(UIObject*)object);
	}

	for (int i = 0; i < 4; ++i) {
		object = m_object_manager->Add_UI_Obj(L"mouse_icon_" + std::to_wstring(i + 4), 0.3f + (float)i * 0.125f, 1.0f - (50.0f / 540.0f), 100.0f / 960.0f, 100.0f / 540.0f,
			2880, 1755, 1380.0f, 1260.0f, 1680.0f, 1560.0f, false, false);
		network.AddMouseUIObject(*(UIObject*)object);
	}

	for (int i = 0; i < 4; ++i) {
		object = m_object_manager->Add_UI_Obj(L"cheese_icon_" + std::to_wstring(i), -0.12f + (float)i * 0.08f, 0.75f, 100.0f / 960.0f, 100.0f / 540.0f,
			2880, 1755, 1080.0f, 1560.0f, 1180.0f, 1660.0f, false, false);
		network.AddCheeseUIObject(*(UIObject*)object);
	}

	// test //
	object = m_object_manager->Add_Text_UI_Obj(L"result_test", 0.0f, 0.0f, 0.1f, 0.1f, true, true);
	object->Set_Color_Mul(1.0f, 1.0f, 0.0f, 1.0f);
	object->Set_Custom_Fuction_One(Result_Test_UI_Function);
	((TextUIObject*)object)->Set_Text(L"결과 테스트");

	// game end scene
	object = m_object_manager->Add_Text_UI_Obj(L"winner_is", -0.9f, 0.85f, 0.15f, 0.15f, false, false);
	object->Set_Color_Mul(1.0f, 1.0f, 0.0f, 1.0f);
	((TextUIObject*)object)->Set_Text(L"승자는..");

	object = m_object_manager->Add_Text_UI_Obj(L"winner", -0.75f, 0.4f, 0.4f, 0.4f, false, false);
	object->Set_Color_Mul(1.0f, 1.0f, 0.0f, 1.0f);
	((TextUIObject*)object)->Set_Text(L"생쥐!");

	// dissolve
	object = m_object_manager->Add_Text_UI_Obj(L"dissolve", 0.0f, 0.0f, 5.0f, 5.0f, false);
	object->Set_Color_Mul(0.0f, 0.0f, 0.0f, 1.0f);
	((TextUIObject*)object)->Set_Text(L"■");

	//
	object = m_object_manager->Add_Particle_Obj(L"particle");

	// [CS] total 타임 추가
	network.SetTotalTime(m_total_time);

	object = m_object_manager->Add_Obj(L"Door_Camera", L"zero_box");
	object->Set_Position(507.157806f, 42.9769135f, -122.538284f);
	object->Set_Visible(false);
	network.SetDoorObject(*object);
}

void TestScene::Build_C(D3DManager* d3d_manager) {
	auto main_camera = reinterpret_cast<Camera*>(m_object_manager->Add_Cam(L"maincamera", L"camera", L"main_scene_object",
		0.0f, 5.0f, 0.0f, 0.1f, ROTATE_SYNC_NONE));
	main_camera->Rotate_Pitch(0.25f);
	main_camera->Rotate_Right(-0.25f);

	main_camera->Set_Frustum(0.25f * MathHelper::Pi(), d3d_manager->Get_Aspect_Ratio(), 1.0f, 2000.0f);
	main_camera->Set_Limit_Rotate_Right(true, -RIGHT_ANGLE_RADIAN + 0.01f, RIGHT_ANGLE_RADIAN - 0.01f);
	//main_camera->Set_Limit_Rotate_Right(true, DirectX::XMConvertToRadians(1.0f), DirectX::XMConvertToRadians(60.0f));
	main_camera->Set_Lagging_Degree(1.0f);

	m_main_camera = main_camera;
	camera = main_camera;
}

void TestScene::Build_FR(ID3D12Device* device) {
	for (int i = 0; i < FRAME_RESOURCES_NUMBER; ++i) {
		m_frameresources.emplace_back(std::make_unique<FrameResorce>(device, PASS_NUMBER,
			(UINT)m_object_manager->Get_Obj_Count(), (UINT)m_object_manager->Get_Material_Manager().Get_Material_Count(),
			(UINT)m_object_manager->Get_Instc_Obj_Arr().size(), m_object_manager->Get_Max_Instc_Count()));
	}

	m_current_frameresource_index = (m_current_frameresource_index + 1) % FRAME_RESOURCES_NUMBER;
	m_current_frameresource = m_frameresources[m_current_frameresource_index].get();
}

void TestScene::Build_DH(ID3D12Device* device) {
	UINT object_count = (UINT)m_object_manager->Get_Obj_Count();
	UINT material_count = (UINT)m_object_manager->Get_Material_Manager().Get_Material_Count();
	UINT instance_object_count = (UINT)m_object_manager->Get_Instc_Obj_Arr().size();
	UINT texture_count = (UINT)m_texture_map.size();

	UINT descriptors_number = (object_count * 2 + material_count + PASS_NUMBER + instance_object_count) * FRAME_RESOURCES_NUMBER + texture_count + 1;	// object also has animation

	m_material_CBV_offset = object_count * FRAME_RESOURCES_NUMBER;
	m_pass_CBV_offset = m_material_CBV_offset + material_count * FRAME_RESOURCES_NUMBER;
	m_animation_CBV_offset = m_pass_CBV_offset + PASS_NUMBER * FRAME_RESOURCES_NUMBER;
	m_instance_SRV_offset = m_animation_CBV_offset + object_count * FRAME_RESOURCES_NUMBER;
	m_texture_SRV_offset = m_instance_SRV_offset + instance_object_count * FRAME_RESOURCES_NUMBER;
	m_shadow_map_SRV_offset = m_texture_SRV_offset + texture_count;

	D3D12_DESCRIPTOR_HEAP_DESC CBV_heap_desc;
	CBV_heap_desc.NumDescriptors = descriptors_number;
	CBV_heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	CBV_heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	CBV_heap_desc.NodeMask = 0;

	Throw_If_Failed(device->CreateDescriptorHeap(&CBV_heap_desc, IID_PPV_ARGS(&m_CBV_heap)));
}

void TestScene::Build_CBV(D3DManager* d3d_manager) {
	ID3D12Device* device = d3d_manager->Get_Device();

	UINT object_constant_buffer_size = Calc_CB_Size(sizeof(ObjectConstants));
	UINT object_count = (UINT)m_object_manager->Get_Obj_Count();

	for (int frameresource_index = 0; frameresource_index < FRAME_RESOURCES_NUMBER; ++frameresource_index) {
		auto object_constant_buffer = m_frameresources[frameresource_index]->object_constant_buffer->Get_Resource();

		for (UINT i = 0; i < object_count; ++i) {
			D3D12_GPU_VIRTUAL_ADDRESS constant_buffer_address = object_constant_buffer->GetGPUVirtualAddress();

			constant_buffer_address += i * object_constant_buffer_size;

			int descriptor_heap_index = frameresource_index * object_count + i;
			auto cpu_descriptor_handle = D3D12_CPU_DESCRIPTOR_HANDLE_EX(m_CBV_heap->GetCPUDescriptorHandleForHeapStart());
			cpu_descriptor_handle.Get_By_Offset(descriptor_heap_index, d3d_manager->Get_CBV_SRV_UAV_Descritpor_Size());

			D3D12_CONSTANT_BUFFER_VIEW_DESC constant_buffer_view_desc;
			constant_buffer_view_desc.BufferLocation = constant_buffer_address;
			constant_buffer_view_desc.SizeInBytes = object_constant_buffer_size;

			device->CreateConstantBufferView(&constant_buffer_view_desc, cpu_descriptor_handle);
		}
	}

	UINT material_constant_buffer_size = Calc_CB_Size(sizeof(MaterialConstants));
	UINT material_count = (UINT)m_object_manager->Get_Material_Manager().Get_Material_Count();

	for (int frameresource_index = 0; frameresource_index < FRAME_RESOURCES_NUMBER; ++frameresource_index) {
		auto material_constant_buffer = m_frameresources[frameresource_index]->material_constant_buffer->Get_Resource();

		for (UINT i = 0; i < material_count; ++i) {
			D3D12_GPU_VIRTUAL_ADDRESS constant_buffer_address = material_constant_buffer->GetGPUVirtualAddress();

			constant_buffer_address += i * material_constant_buffer_size;

			int descriptor_heap_index = m_material_CBV_offset + frameresource_index * material_count + i;
			auto cpu_descriptor_handle = D3D12_CPU_DESCRIPTOR_HANDLE_EX(m_CBV_heap->GetCPUDescriptorHandleForHeapStart());
			cpu_descriptor_handle.Get_By_Offset(descriptor_heap_index, d3d_manager->Get_CBV_SRV_UAV_Descritpor_Size());

			D3D12_CONSTANT_BUFFER_VIEW_DESC constant_buffer_view_desc;
			constant_buffer_view_desc.BufferLocation = constant_buffer_address;
			constant_buffer_view_desc.SizeInBytes = material_constant_buffer_size;

			device->CreateConstantBufferView(&constant_buffer_view_desc, cpu_descriptor_handle);
		}
	}

	UINT pass_constant_buffer_size = Calc_CB_Size(sizeof(PassConstants));

	for (int frameresource_index = 0; frameresource_index < FRAME_RESOURCES_NUMBER; ++frameresource_index) {
		auto pass_constant_buffer = m_frameresources[frameresource_index]->pass_constant_buffer->Get_Resource();

		for (UINT i = 0; i < PASS_NUMBER; ++i) {
			D3D12_GPU_VIRTUAL_ADDRESS constant_buffer_address = pass_constant_buffer->GetGPUVirtualAddress();

			constant_buffer_address += i * pass_constant_buffer_size;

			int descriptor_heap_index = m_pass_CBV_offset + frameresource_index * PASS_NUMBER + i;
			auto cpu_descriptor_handle = D3D12_CPU_DESCRIPTOR_HANDLE_EX(m_CBV_heap->GetCPUDescriptorHandleForHeapStart());
			cpu_descriptor_handle.Get_By_Offset(descriptor_heap_index, d3d_manager->Get_CBV_SRV_UAV_Descritpor_Size());

			D3D12_CONSTANT_BUFFER_VIEW_DESC constant_buffer_view_desc;
			constant_buffer_view_desc.BufferLocation = constant_buffer_address;
			constant_buffer_view_desc.SizeInBytes = pass_constant_buffer_size;

			device->CreateConstantBufferView(&constant_buffer_view_desc, cpu_descriptor_handle);
		}
	}

	UINT animation_constant_buffer_size = Calc_CB_Size(sizeof(AnimationConstants));
	object_count = (UINT)m_object_manager->Get_Obj_Count();

	for (int frameresource_index = 0; frameresource_index < FRAME_RESOURCES_NUMBER; ++frameresource_index) {
		auto animation_constant_buffer = m_frameresources[frameresource_index]->animation_constant_buffer->Get_Resource();

		for (UINT i = 0; i < object_count; ++i) {
			D3D12_GPU_VIRTUAL_ADDRESS constant_buffer_address = animation_constant_buffer->GetGPUVirtualAddress();

			constant_buffer_address += i * animation_constant_buffer_size;

			int descriptor_heap_index = m_animation_CBV_offset + frameresource_index * object_count + i;
			auto cpu_descriptor_handle = D3D12_CPU_DESCRIPTOR_HANDLE_EX(m_CBV_heap->GetCPUDescriptorHandleForHeapStart());
			cpu_descriptor_handle.Get_By_Offset(descriptor_heap_index, d3d_manager->Get_CBV_SRV_UAV_Descritpor_Size());

			D3D12_CONSTANT_BUFFER_VIEW_DESC constant_buffer_view_desc;
			constant_buffer_view_desc.BufferLocation = constant_buffer_address;
			constant_buffer_view_desc.SizeInBytes = animation_constant_buffer_size;

			device->CreateConstantBufferView(&constant_buffer_view_desc, cpu_descriptor_handle);
		}
	}

	UINT instance_data_buffer_size = (UINT)sizeof(InstanceData);
	object_count = (UINT)m_object_manager->Get_Instc_Obj_Arr().size();
	UINT max_instance_count = m_object_manager->Get_Max_Instc_Count();

	for (int frameresource_index = 0; frameresource_index < FRAME_RESOURCES_NUMBER; ++frameresource_index) {
		for (UINT i = 0; i < object_count; ++i) {
			auto instance_data_buffer = m_frameresources[frameresource_index]->instance_data_buffer_array[i]->Get_Resource();

			int descriptor_heap_index = m_instance_SRV_offset + frameresource_index * object_count + i;
			auto cpu_descriptor_handle = D3D12_CPU_DESCRIPTOR_HANDLE_EX(m_CBV_heap->GetCPUDescriptorHandleForHeapStart());
			cpu_descriptor_handle.Get_By_Offset(descriptor_heap_index, d3d_manager->Get_CBV_SRV_UAV_Descritpor_Size());

			D3D12_SHADER_RESOURCE_VIEW_DESC shader_resource_view_desc = {};
			shader_resource_view_desc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
			shader_resource_view_desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			shader_resource_view_desc.Buffer.FirstElement = 0;
			shader_resource_view_desc.Buffer.NumElements = max_instance_count;
			shader_resource_view_desc.Buffer.StructureByteStride = instance_data_buffer_size;
			shader_resource_view_desc.Format = DXGI_FORMAT_UNKNOWN;

			device->CreateShaderResourceView(instance_data_buffer, &shader_resource_view_desc, cpu_descriptor_handle);
		}
	}

	std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> texture_list;

	for (auto& t : m_texture_map) {
		texture_list.emplace_back(t.second->resource);
	}

	D3D12_SHADER_RESOURCE_VIEW_DESC SRV_desc = {};
	SRV_desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	SRV_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	SRV_desc.Texture2D.MostDetailedMip = 0;
	SRV_desc.Texture2D.ResourceMinLODClamp = 0.0f;

	for (UINT i = 0; i < (UINT)texture_list.size(); ++i) {
		D3D12_CPU_DESCRIPTOR_HANDLE_EX cpu_descriptor_handle(m_CBV_heap->GetCPUDescriptorHandleForHeapStart());
		cpu_descriptor_handle.Get_By_Offset(m_texture_SRV_offset + i, d3d_manager->Get_CBV_SRV_UAV_Descritpor_Size());

		SRV_desc.Format = texture_list[i]->GetDesc().Format;
		SRV_desc.Texture2D.MipLevels = texture_list[i]->GetDesc().MipLevels;
		device->CreateShaderResourceView(texture_list[i].Get(), &SRV_desc, cpu_descriptor_handle);
	}

	//
	m_shadow_map->Build_Descriptors(d3d_manager->Get_Device(),
		D3D12_CPU_DESCRIPTOR_HANDLE_EX(m_CBV_heap->GetCPUDescriptorHandleForHeapStart(), m_shadow_map_SRV_offset, d3d_manager->Get_CBV_SRV_UAV_Descritpor_Size()),
		D3D12_GPU_DESCRIPTOR_HANDLE_EX(m_CBV_heap->GetGPUDescriptorHandleForHeapStart(), m_shadow_map_SRV_offset, d3d_manager->Get_CBV_SRV_UAV_Descritpor_Size()),
		D3D12_CPU_DESCRIPTOR_HANDLE_EX(d3d_manager->Get_DSV(), 1, d3d_manager->Get_DSV_Descritpor_Size()));
}

void TestScene::Build_PSO(D3DManager* d3d_manager) {
	ID3D12Device* device = d3d_manager->Get_Device();

	//
	D3D12_RENDER_TARGET_BLEND_DESC render_target_blend_desc;
	render_target_blend_desc.BlendEnable = true;
	render_target_blend_desc.LogicOpEnable = false;
	render_target_blend_desc.SrcBlend = D3D12_BLEND_SRC_ALPHA;
	render_target_blend_desc.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
	render_target_blend_desc.BlendOp = D3D12_BLEND_OP_ADD;
	render_target_blend_desc.SrcBlendAlpha = D3D12_BLEND_ONE;
	render_target_blend_desc.DestBlendAlpha = D3D12_BLEND_ZERO;
	render_target_blend_desc.BlendOpAlpha = D3D12_BLEND_OP_ADD;
	render_target_blend_desc.LogicOp = D3D12_LOGIC_OP_NOOP;
	render_target_blend_desc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	D3D12_GRAPHICS_PIPELINE_STATE_DESC opaque_PSO_desc;
	ZeroMemory(&opaque_PSO_desc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
	opaque_PSO_desc.InputLayout = { m_input_layouts.data(), (UINT)m_input_layouts.size() };
	opaque_PSO_desc.pRootSignature = m_root_signature.Get();
	opaque_PSO_desc.VS = { reinterpret_cast<BYTE*>(m_shader_map[L"standard_VS"]->GetBufferPointer()), m_shader_map[L"standard_VS"]->GetBufferSize() };
	opaque_PSO_desc.PS = { reinterpret_cast<BYTE*>(m_shader_map[L"opaque_PS"]->GetBufferPointer()), m_shader_map[L"opaque_PS"]->GetBufferSize() };
	opaque_PSO_desc.RasterizerState = D3D12_RASTERIZER_DESC_EX(D3D12_DEFAULT());
	opaque_PSO_desc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
	//opaque_PSO_desc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	opaque_PSO_desc.BlendState = D3D12_BLEND_DESC_EX(D3D12_DEFAULT());
	opaque_PSO_desc.DepthStencilState = D3D12_DEPTH_STENCIL_DESC_EX(
	TRUE, D3D12_DEPTH_WRITE_MASK_ALL, D3D12_COMPARISON_FUNC_LESS,
		TRUE, 0xFF, 0xFF, D3D12_STENCIL_OP_KEEP, D3D12_STENCIL_OP_KEEP, D3D12_STENCIL_OP_REPLACE, D3D12_COMPARISON_FUNC_ALWAYS,
		D3D12_STENCIL_OP_KEEP, D3D12_STENCIL_OP_KEEP, D3D12_STENCIL_OP_KEEP, D3D12_COMPARISON_FUNC_ALWAYS);
	opaque_PSO_desc.SampleMask = UINT_MAX;
	opaque_PSO_desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	opaque_PSO_desc.NumRenderTargets = 1;
	opaque_PSO_desc.RTVFormats[0] = d3d_manager->Get_BB_Format();
	opaque_PSO_desc.SampleDesc.Count = d3d_manager->Is_4xMSAA() ? 4 : 1;
	opaque_PSO_desc.SampleDesc.Quality = d3d_manager->Is_4xMSAA() ? (d3d_manager->Get_4xMSAA_Qual() - 1) : 0;
	opaque_PSO_desc.DSVFormat = d3d_manager->Get_DS_Format();

	Throw_If_Failed(device->CreateGraphicsPipelineState(&opaque_PSO_desc, IID_PPV_ARGS(&m_pipeline_state_map[L"opaque"])));

	D3D12_GRAPHICS_PIPELINE_STATE_DESC opaque_PSO_desc_copy = opaque_PSO_desc;

	//
	opaque_PSO_desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
	opaque_PSO_desc.VS = { reinterpret_cast<BYTE*>(m_shader_map[L"particle_VS"]->GetBufferPointer()), m_shader_map[L"particle_VS"]->GetBufferSize() };
	opaque_PSO_desc.GS = { reinterpret_cast<BYTE*>(m_shader_map[L"particle_GS"]->GetBufferPointer()), m_shader_map[L"particle_GS"]->GetBufferSize() };
	opaque_PSO_desc.PS = { reinterpret_cast<BYTE*>(m_shader_map[L"particle_PS"]->GetBufferPointer()), m_shader_map[L"particle_PS"]->GetBufferSize() };

	Throw_If_Failed(device->CreateGraphicsPipelineState(&opaque_PSO_desc, IID_PPV_ARGS(&m_pipeline_state_map[L"particle"])));

	//
	opaque_PSO_desc.BlendState.RenderTarget[0] = render_target_blend_desc;
	opaque_PSO_desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	opaque_PSO_desc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	opaque_PSO_desc.VS = { reinterpret_cast<BYTE*>(m_shader_map[L"text_ui_VS"]->GetBufferPointer()), m_shader_map[L"text_ui_VS"]->GetBufferSize() };
	opaque_PSO_desc.GS = D3D12_SHADER_BYTECODE();
	opaque_PSO_desc.PS = { reinterpret_cast<BYTE*>(m_shader_map[L"text_ui_PS"]->GetBufferPointer()), m_shader_map[L"text_ui_PS"]->GetBufferSize() };

	Throw_If_Failed(device->CreateGraphicsPipelineState(&opaque_PSO_desc, IID_PPV_ARGS(&m_pipeline_state_map[L"text_ui"])));

	//
	opaque_PSO_desc.BlendState = D3D12_BLEND_DESC_EX(D3D12_DEFAULT());
	opaque_PSO_desc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	opaque_PSO_desc.VS = { reinterpret_cast<BYTE*>(m_shader_map[L"instance_VS"]->GetBufferPointer()), m_shader_map[L"instance_VS"]->GetBufferSize() };
	opaque_PSO_desc.PS = { reinterpret_cast<BYTE*>(m_shader_map[L"opaque_PS"]->GetBufferPointer()), m_shader_map[L"opaque_PS"]->GetBufferSize() };

	Throw_If_Failed(device->CreateGraphicsPipelineState(&opaque_PSO_desc, IID_PPV_ARGS(&m_pipeline_state_map[L"instance"])));

	//
	opaque_PSO_desc.RasterizerState.FillMode = D3D12_FILL_MODE_WIREFRAME;

	Throw_If_Failed(device->CreateGraphicsPipelineState(&opaque_PSO_desc, IID_PPV_ARGS(&m_pipeline_state_map[L"instance_wireframe"])));

	//
	opaque_PSO_desc.VS = { reinterpret_cast<BYTE*>(m_shader_map[L"standard_VS"]->GetBufferPointer()), m_shader_map[L"standard_VS"]->GetBufferSize() };

	Throw_If_Failed(device->CreateGraphicsPipelineState(&opaque_PSO_desc, IID_PPV_ARGS(&m_pipeline_state_map[L"opaque_wireframe"])));

	//
	opaque_PSO_desc.BlendState.RenderTarget[0] = render_target_blend_desc;
	opaque_PSO_desc.PS = { reinterpret_cast<BYTE*>(m_shader_map[L"silhouette_PS"]->GetBufferPointer()), m_shader_map[L"silhouette_PS"]->GetBufferSize() };
	opaque_PSO_desc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
	opaque_PSO_desc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_GREATER;
	opaque_PSO_desc.DepthStencilState.FrontFace.StencilPassOp = D3D12_STENCIL_OP_INCR;
	opaque_PSO_desc.DepthStencilState.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_EQUAL;
	//opaque_PSO_desc.DepthStencilState.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_NEVER;

	Throw_If_Failed(device->CreateGraphicsPipelineState(&opaque_PSO_desc, IID_PPV_ARGS(&m_pipeline_state_map[L"silhouette"])));

	//
	opaque_PSO_desc_copy.BlendState.RenderTarget[0] = render_target_blend_desc;
	opaque_PSO_desc_copy.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	opaque_PSO_desc_copy.VS = { reinterpret_cast<BYTE*>(m_shader_map[L"ui_VS"]->GetBufferPointer()), m_shader_map[L"ui_VS"]->GetBufferSize() };
	opaque_PSO_desc_copy.PS = { reinterpret_cast<BYTE*>(m_shader_map[L"ui_PS"]->GetBufferPointer()), m_shader_map[L"ui_PS"]->GetBufferSize() };

	Throw_If_Failed(device->CreateGraphicsPipelineState(&opaque_PSO_desc_copy, IID_PPV_ARGS(&m_pipeline_state_map[L"ui"])));

	//
	opaque_PSO_desc_copy.BlendState = D3D12_BLEND_DESC_EX(D3D12_DEFAULT());
	opaque_PSO_desc_copy.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	opaque_PSO_desc_copy.RasterizerState.DepthBias = 10000;
	opaque_PSO_desc_copy.RasterizerState.DepthBiasClamp = 0.0f;
	opaque_PSO_desc_copy.RasterizerState.SlopeScaledDepthBias = 1.0f;
	opaque_PSO_desc_copy.VS = { reinterpret_cast<BYTE*>(m_shader_map[L"shadow_VS"]->GetBufferPointer()), m_shader_map[L"shadow_VS"]->GetBufferSize() };
	opaque_PSO_desc_copy.PS = { reinterpret_cast<BYTE*>(m_shader_map[L"shadow_PS"]->GetBufferPointer()), m_shader_map[L"shadow_PS"]->GetBufferSize() };
	opaque_PSO_desc_copy.RTVFormats[0] = DXGI_FORMAT_UNKNOWN;
	opaque_PSO_desc_copy.NumRenderTargets = 0;

	Throw_If_Failed(device->CreateGraphicsPipelineState(&opaque_PSO_desc_copy, IID_PPV_ARGS(&m_pipeline_state_map[L"shadow"])));

	//
	opaque_PSO_desc_copy.VS = { reinterpret_cast<BYTE*>(m_shader_map[L"instance_shadow_VS"]->GetBufferPointer()), m_shader_map[L"instance_shadow_VS"]->GetBufferSize() };

	Throw_If_Failed(device->CreateGraphicsPipelineState(&opaque_PSO_desc_copy, IID_PPV_ARGS(&m_pipeline_state_map[L"instance_shadow"])));
}

void TestScene::Binding_Key() {
	m_input_manager->Bind_Key_Up(VK_LBUTTON, BindingInfo(L"", Action::CUSTOM_FUNCTION_TWO));

	m_input_manager->Bind_Mouse_Move(BindingInfo(L"", Action::PICKING, POINTF(0.0f, 0.0f)),
		BindingInfo(), BindingInfo());

	//m_input_manager->Set_Fix_Cursor(true);
}

void TestScene::Pairing_Collision_Set() {
	//m_object_manager->Add_Collision_Pair(L"")
}

void TestScene::Custom_Function_One() {
	m_render_silhouette = !m_render_silhouette;
}

void TestScene::Custom_Function_Two() {
	for (auto& o : m_object_manager->Get_Selected_Obj_Arr()) {
		o->Call_Custom_Function_One();
	}
}

// observing
void TestScene::Custom_Function_Three()
{
	const int num = 8;
	int start = m_observing_num;
	bool found = false;

	for (int i = 0; i < num; ++i)
	{
		int idx = (start + i) % num;
		auto selected = m_object_manager->Get_Selected_Obj_Arr()[idx];

		if (selected->Get_Visible())
		{
			m_object_manager->Bind_Cam_2_Obj(
				L"maincamera",
				selected->Get_Name(),
				0.0f, 0.0f, 0.0f, 100.0f,
				ROTATE_SYNC_NONE
			);

			m_main_camera->Rst_Rotate();

			m_observing_num = (idx + 1) % num;
			found = true;
			break;
		}
	}

	if (false == found)
	{
		m_observing_num = 0;
	}
}

void TestScene::Crt_Voxel(DirectX::XMFLOAT3 position, float scale, UINT detail_level) {
	if (detail_level == 0) {
		Object* object = m_object_manager->Add_Obj(L"voxel_" + std::to_wstring(m_voxel_count++), L"cheese", L"Object",
			DirectX::XMMATRIX(
				scale, 0.0f, 0.0f, 0.0f,
				0.0f, scale, 0.0f, 0.0f,
				0.0f, 0.0f, scale, 0.0f,
				position.x, position.y, position.z, 1.0f));
	}
	else {
		float half = scale / 2.0f;
		float quarter = scale / 4.0f;

		for (int i = -1; i < 2; ++++i) {
			for (int j = -1; j < 2; ++++j) {
				for (int k = -1; k < 2; ++++k) {
					Crt_Voxel(
						DirectX::XMFLOAT3(position.x + quarter * k, position.y + quarter * i, position.z + quarter * j),
						half, detail_level - 1);
				}
			}
		}
	}
}
// y * z * x = 1048
void TestScene::Crt_Voxel_Cheese(DirectX::XMFLOAT3 position, float scale, UINT detail_level) {
	int m_random_value = 10;
	int y_value = 8;
	int z_value = 21;
	int x_value = z_value / 2;

	//
	int count = 0;

	//std::random_device rd;
	std::uniform_int_distribution<int> uid(1, m_random_value);

	DirectX::XMFLOAT3 pivot_position = position;

	position.y += scale / 2.0f;

	for (int i = 0; i < VOXEL_CHEESE_HEIGHT; ++i) {
		position.z = pivot_position.z - scale * (float)(z_value / 2);

		for (int j = 1; j <= VOXEL_CHEESE_DEPTH; ++j) {
			position.x = pivot_position.x - scale * (float)(x_value / 2);

			for (int k = 0; k <= j / 2; ++k) {
				/*if ((i == y_value - 1 || j == z_value || k == 0 || k == j / 2) &&
					!(uid(rd) % m_random_value)) {
					position.x += scale;
					continue;
				}*/

				Crt_Voxel(position, scale, detail_level);
				position.x += scale;

				++count;
			}

			position.z += scale;
		}

		position.y += scale;
	}
}

void TestScene::Del_Voxel(int cheese_index, int voxel_index) {
	VoxelCheese* voxel_cheese_pointer = (VoxelCheese*)m_object_manager->Get_Obj(L"cheese_" + std::to_wstring(cheese_index));

	voxel_cheese_pointer->Remove_Voxel(voxel_index);
}

void TestScene::Chg_Scene_State(Scene_State scene_state) {
	//
	for (auto& o : m_object_manager->Get_Selected_Obj_Arr()) {
		o->Set_Additional_Scale(0.0f, 0.0f, 0.0f);
	}

	m_object_manager->Rst_Selected_Obj_Arr();
	m_object_manager->Hide_All_UI();
	m_object_manager->Get_Obj(L"dissolve")->Set_Visible(true);

	m_input_manager->Rst_Manager();

	//
	Object* object = nullptr;

	for (int i = 0; i < 2; ++i) {
		object = m_object_manager->Get_Obj(L"cat_model_" + std::to_wstring(i));
		object->Set_Visible(false);
		object->Set_Shade(false);
	}

	for (int i = 0; i < 9; ++i) {
		object = m_object_manager->Get_Obj(L"mouse_model_" + std::to_wstring(i));
		object->Set_Visible(false);
		object->Set_Shade(false);
	}

	switch (scene_state) {
		//
		m_sound_manager->Restart_Channel(L"bgm");

	case Scene_State::MAIN_STATE:
		m_object_manager->Get_Obj(L"game_start")->Set_Visible(true);
		m_object_manager->Get_Obj(L"game_end")->Set_Visible(true);
		m_object_manager->Get_Obj(L"catcha_title")->Set_Visible(true);

		m_main_camera->Set_Lagging_Degree(1.0f);


		NetworkManager::GetInstance().EndSceneInitCharacters();
		m_object_manager->RestorObjectMap();

		for (int i = 0; i < CHEESE_NUM; ++i)
		{
			((VoxelCheese*)m_object_manager->Get_Obj(
				L"cheese" + std::to_wstring(i)))->Rst_Voxel(
					CHEESE_POS[i].x,
					CHEESE_POS[i].y,
					CHEESE_POS[i].z,
					1.0f,
					1
				);
		}

		//
		object = m_object_manager->Get_Obj(L"cat_model_0");
		object->Set_Visible(true);
		object->Set_Shade(true);

		object = m_object_manager->Get_Obj(L"mouse_model_0");
		object->Set_Visible(true);
		object->Set_Shade(true);

		is_player_cat = false;

		//
		m_object_manager->Bind_Cam_2_Obj(L"maincamera", L"main_scene_object",
			0.0f, 5.0f, 0.0f, 0.1f, ROTATE_SYNC_NONE);
		m_main_camera->Rst_Rotate();
		m_main_camera->Rotate_Pitch(0.25f);
		m_main_camera->Rotate_Right(-0.25f);

		//
		m_input_manager->Bind_Key_Up(VK_LBUTTON, BindingInfo(L"", Action::CUSTOM_FUNCTION_TWO));

		m_input_manager->Bind_Mouse_Move(BindingInfo(L"", Action::PICKING, POINTF(0.0f, 0.0f)),
			BindingInfo(), BindingInfo());

		m_input_manager->Bind_Key_First_Down(VK_F1, BindingInfo(L"", Action::CHANGE_WIREFRAME_FLAG));
		break;
		//
		m_sound_manager->Restart_Channel(L"bgm");

		//
	case Scene_State::MATCHING_STATE:
		m_object_manager->Get_Obj(L"select_animal")->Set_Visible(true);
		m_object_manager->Get_Obj(L"to_main")->Set_Visible(true);
		m_object_manager->Get_Obj(L"select_cat")->Set_Visible(true);
		m_object_manager->Get_Obj(L"select_mouse")->Set_Visible(true);

		object = m_object_manager->Get_Obj(L"cat_model_0");
		object->Set_Visible(true);
		object->Set_Shade(true);

		object = m_object_manager->Get_Obj(L"mouse_model_0");
		object->Set_Visible(true);
		object->Set_Shade(true);

		//
		//
		m_object_manager->Bind_Cam_2_Obj(L"maincamera", L"main_scene_object",
			0.0f, 5.0f, 0.0f, 0.1f, ROTATE_SYNC_NONE);
		m_main_camera->Rst_Rotate();
		m_main_camera->Rotate_Pitch(0.25f);
		m_main_camera->Rotate_Right(-0.25f);

		//
		m_input_manager->Bind_Key_Up(VK_LBUTTON, BindingInfo(L"", Action::CUSTOM_FUNCTION_TWO));

		m_input_manager->Bind_Mouse_Move(BindingInfo(L"", Action::PICKING, POINTF(0.0f, 0.0f)),
			BindingInfo(), BindingInfo());

		m_input_manager->Bind_Key_First_Down(VK_F1, BindingInfo(L"", Action::CHANGE_WIREFRAME_FLAG));
		break;
		//
		m_sound_manager->Set_Channel_Paused(L"bgm", true);

		//
		m_input_manager->Set_Fix_Cursor(true);
		m_input_manager->Set_Hide_Cursor(true);

		//
	case Scene_State::PLAY_STATE:
		object = m_object_manager->Get_Obj(L"cat_model_0");
		object->Set_Visible(false);
		object->Set_Shade(false);

		object = m_object_manager->Get_Obj(L"mouse_model_0");
		object->Set_Visible(false);
		object->Set_Shade(false);

		m_object_manager->Get_Obj(L"aim_circle")->Set_Visible(true);
		m_object_manager->Get_Obj(L"timer")->Set_Visible(true);
		m_object_manager->Get_Obj(L"attacked_ui")->Set_Visible(true);
		m_object_manager->Get_Obj(L"game_play_ui")->Set_Visible(true);

		if (true == NetworkManager::GetInstance().IsPlayerCat())
		{
			m_object_manager->Get_Obj(L"cat_portrait")->Set_Visible(true);
		}
		else
		{
			m_object_manager->Get_Obj(L"mouse_portrait")->Set_Visible(true);
		}

		//m_object_manager->Get_Obj(L"player")->Set_Visible(false);

		for (int i = 0; i < 20; ++i) {
			object = m_object_manager->Get_Obj(L"gauge_ui_" + std::to_wstring(i));
			object->Set_Visible(true);
			//object->Set_Color_Mul(0.0f, 0.0f, 1.0f, 1.0f);
			object->Set_Color_Mul(1.0f, 0.0f, 0.0f, 1.0f);
		}

		for (int i = 0; i < 8; ++i) {
			m_object_manager->Get_Obj(L"mouse_icon_" + std::to_wstring(i))->Set_Visible(true);
			m_object_manager->Get_Obj(L"mouse_icon_" + std::to_wstring(i))->Set_Color_Mul(1.0f, 1.0f, 1.0f, 1.0f);
		}

		for (int i = 0; i < 4; ++i) {
			m_object_manager->Get_Obj(L"cheese_icon_" + std::to_wstring(i))->Set_Visible(true);
			m_object_manager->Get_Obj(L"cheese_icon_" + std::to_wstring(i))->Set_Color_Mul(1.0f, 1.0f, 1.0f, 1.0f);
		}

		m_object_manager->Get_Obj(L"Door_003")->Set_Visible(true);

		m_object_manager->Get_Obj(L"timer")->Set_Position(-0.1f, 0.9f, 0.0f);
		m_object_manager->Get_Obj(L"timer")->Set_Scale(0.06f, 0.08f, 0.0f);
		((TextUIObject*)m_object_manager->Get_Obj(L"timer"))->Set_Text(L"대기중..");

		Custom_Function_One();

		//
		m_input_manager->Bind_Key_Down(VK_W, BindingInfo(L"player", Action::MOVE_FORWARD, MOVE_ONLY_XZ));
		m_input_manager->Bind_Key_Down(VK_S, BindingInfo(L"player", Action::MOVE_BACK, MOVE_ONLY_XZ));
		m_input_manager->Bind_Key_Down(VK_A, BindingInfo(L"player", Action::MOVE_LEFT, MOVE_ONLY_XZ));
		m_input_manager->Bind_Key_Down(VK_D, BindingInfo(L"player", Action::MOVE_RIGHT, MOVE_ONLY_XZ));

		m_input_manager->Bind_Key_First_Down(VK_W, BindingInfo(L"player", Action::MOVE_FORWARD, MOVE_ONLY_XZ));
		m_input_manager->Bind_Key_First_Down(VK_S, BindingInfo(L"player", Action::MOVE_BACK, MOVE_ONLY_XZ));
		m_input_manager->Bind_Key_First_Down(VK_A, BindingInfo(L"player", Action::MOVE_LEFT, MOVE_ONLY_XZ));
		m_input_manager->Bind_Key_First_Down(VK_D, BindingInfo(L"player", Action::MOVE_RIGHT, MOVE_ONLY_XZ));

		m_input_manager->Bind_Key_First_Down(VK_SPACE, BindingInfo(L"player", Action::ACTION_JUMP));
		m_input_manager->Bind_Key_First_Down(VK_LBUTTON, BindingInfo(L"player", Action::ACTION_ONE));

		m_input_manager->Bind_Key_Up(VK_W, BindingInfo(L"player", Action::MOVE_FORWARD, MOVE_ONLY_XZ));
		m_input_manager->Bind_Key_Up(VK_S, BindingInfo(L"player", Action::MOVE_BACK, MOVE_ONLY_XZ));
		m_input_manager->Bind_Key_Up(VK_A, BindingInfo(L"player", Action::MOVE_LEFT, MOVE_ONLY_XZ));
		m_input_manager->Bind_Key_Up(VK_D, BindingInfo(L"player", Action::MOVE_RIGHT, MOVE_ONLY_XZ));

		m_input_manager->Bind_Mouse_Move(BindingInfo(), BindingInfo(L"maincamera", Action::ROTATE_PITCH, 0.01f),
			BindingInfo(L"maincamera", Action::ROTATE_RIGHT, 0.01f));

		m_input_manager->Bind_Key_First_Down(VK_F1, BindingInfo(L"", Action::CHANGE_WIREFRAME_FLAG));
		m_input_manager->Bind_Key_First_Down(VK_F2, BindingInfo(L"", Action::CHANGE_BOUNDINGBOX_FLAG));

		m_input_manager->Bind_Key_First_Down(VK_TAB, BindingInfo(L"", Action::HIDE_AND_FIX_CURSOR));
		break;
	case Scene_State::END_STATE:
		//
		m_sound_manager->Set_Channel_Paused(L"bgm", true);

		

		//
		m_object_manager->Get_Obj(L"winner_is")->Set_Visible(true);
		m_object_manager->Get_Obj(L"winner")->Set_Visible(true);
		m_object_manager->Get_Obj(L"to_main")->Set_Visible(true);

		//
		object = m_object_manager->Get_Obj(L"player");
		object->Set_Visible(false);
		object->Set_Shade(false);


		if (true == m_is_cat_winner)
		{
			for (int i = 1; i < 2; ++i) {
				object = m_object_manager->Get_Obj(L"cat_model_" + std::to_wstring(i));
				object->Set_Visible(true);
				object->Set_Shade(true);
			}
			((TextUIObject*)m_object_manager->Get_Obj(L"winner"))->Set_Text(L"고양이!");
		}
		else
		{
			for (int i = 1; i < 9; ++i) {
				object = m_object_manager->Get_Obj(L"mouse_model_" + std::to_wstring(i));
				object->Set_Visible(true);
				object->Set_Shade(true);
			}
			((TextUIObject*)m_object_manager->Get_Obj(L"winner"))->Set_Text(L"쥐!");
		}

		

		//
		m_object_manager->Bind_Cam_2_Obj(L"maincamera", L"end_scene_object",
			0.0f, 5.0f, 0.0f, 0.1f, ROTATE_SYNC_NONE);
		m_main_camera->Rst_Rotate();
		m_main_camera->Rotate_Pitch(0.25f);
		m_main_camera->Rotate_Right(0.25f);

		//
		m_input_manager->Bind_Key_Up(VK_LBUTTON, BindingInfo(L"", Action::CUSTOM_FUNCTION_TWO));

		m_input_manager->Bind_Mouse_Move(BindingInfo(L"", Action::PICKING, POINTF(0.0f, 0.0f)),
			BindingInfo(), BindingInfo());

		m_input_manager->Bind_Key_First_Down(VK_F1, BindingInfo(L"", Action::CHANGE_WIREFRAME_FLAG));


		break;
	default:
		break;
	}
}

void TestScene::Picking(POINTF screen_position) {
	for (auto& o : m_object_manager->Get_Selected_Obj_Arr()) {
		o->Set_Additional_Scale(0.0f, 0.0f, 0.0f);
	}

	m_object_manager->Rst_Selected_Obj_Arr();

	DirectX::XMFLOAT4X4 projection_matrix = m_main_camera->Get_PM_4x4f();

	float view_position_x_ui = +(2.0f * screen_position.x / (float)m_client_width - 1.0f);
	float view_position_y_ui = -(2.0f * screen_position.y / (float)m_client_height - 1.0f);

	float view_position_x = view_position_x_ui / projection_matrix._11;
	float view_position_y = view_position_y_ui / projection_matrix._22;

	DirectX::XMFLOAT4 origin_ray = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	DirectX::XMFLOAT4 ray_direction = DirectX::XMFLOAT4(view_position_x, view_position_y, 1.0f, 0.0f);
	DirectX::XMFLOAT4 ray_direction_ui = DirectX::XMFLOAT4(view_position_x_ui, view_position_y_ui, 1.0f, 0.0f);

	DirectX::XMMATRIX view_matrix = m_main_camera->Get_VM_M();
	DirectX::XMFLOAT4X4 inverse_view_matrix;
	DirectX::XMStoreFloat4x4(&inverse_view_matrix, DirectX::XMMatrixInverse(&DirectX::XMMatrixDeterminant(view_matrix), view_matrix));

	for (auto& o : m_object_manager->Get_UI_Obj_Arr()) {
		if (o->Get_Selectable() == false) {
			continue;
		}

		if (o->Get_Visible()) {
			if (o->Picking(origin_ray, ray_direction_ui, inverse_view_matrix, MAX_PICKING_DISTANCE)) {
				m_object_manager->Add_Selected_Obj(o);
				//OutputDebugStringW((o->Get_Name() + L" Selected\n").c_str());
			}
		}
	}

	for (auto& o : m_object_manager->Get_Text_UI_Obj_Arr()) {
		if (o->Get_Selectable() == false) {
			continue;
		}

		if (o->Get_Visible()) {
			if (o->Picking(origin_ray, ray_direction_ui, inverse_view_matrix, MAX_PICKING_DISTANCE)) {
				m_object_manager->Add_Selected_Obj(o);
				//OutputDebugStringW((o->Get_Name() + L" Selected\n").c_str());
			}
		}
	}

	for (auto& o : m_object_manager->Get_Selected_Obj_Arr()) {
		o->Set_Additional_Scale(0.02f, 0.02f, 0.02f);
	}
}



//
void Game_Start_UI_Function() {
	m_next_scene_state = Scene_State::MATCHING_STATE;
}

void Game_End_UI_Function() {
	PostQuitMessage(0);
}

void To_Main_UI_Function() {
	m_next_scene_state = Scene_State::MAIN_STATE;
}

void Select_Cat_UI_Function() {
	m_next_scene_state = Scene_State::PLAY_STATE;

	NetworkManager& network_manager = NetworkManager::GetInstance();
	network_manager.ChooseCharacter(true);
	is_player_cat = true;
	camera->Set_Freezing_Time(1.0f);
}

void Select_Mouse_UI_Function() {
	m_next_scene_state = Scene_State::PLAY_STATE;

	NetworkManager& network_manager = NetworkManager::GetInstance();
	network_manager.ChooseCharacter(false);
	is_player_cat = false;
	camera->Set_Freezing_Time(1.0f);
}

void TestScene::CharacterChange(bool is_cat, const std::wstring& key1, const std::wstring& key2)
{
	Camera* main_camera = reinterpret_cast<Camera*>(m_object_manager->Get_Obj(L"maincamera"));
	TextUIObject* Aim = (TextUIObject*)m_object_manager->Get_Obj(L"aim_circle");
	if (true == is_cat)
	{
		m_object_manager->Swap_Object(key1, key2);
		//m_object_manager->Bind_Cam_2_Obj(L"maincamera", L"player", 0.0f, 50.0f, 0.0f, 150.0f, ROTATE_SYNC_RPY);
		m_object_manager->Bind_Cam_2_Obj(L"maincamera", L"player", 25.0f, 35.0f, 20.0f, 100.0f, ROTATE_SYNC_RPY);
		m_object_manager->Set_Camera_4_Server(L"maincamera", true);
		main_camera->Set_Limit_Rotate_Right(true, -DirectX::XMConvertToRadians(45.0f), DirectX::XMConvertToRadians(80.0f));
		main_camera->Set_Lagging_Degree(4.0f);
		Aim->Set_Text(L"");
	}
	else
	{
		main_camera->Set_Lagging_Degree(10.0f);
		m_object_manager->Swap_Object(key1, key2);
		m_object_manager->Bind_Cam_2_Obj(L"maincamera", L"player", 0.0f, 4.0f, 0.0f, 0.1f, ROTATE_SYNC_RPY);
		m_object_manager->Set_Camera_4_Server(L"maincamera", true);

		m_object_manager->Get_Obj(L"player")->Set_Visible(true);
		SetCameraLagging(1.0f, 2.0f);
	}
}

void TestScene::InitCamera(DirectX::XMFLOAT4 rotate_quat)
{
	m_object_manager->Set_Camera_Init_4_Server(L"maincamera", rotate_quat);
}

void TestScene::OpenDoorEvent()
{	
	m_door_open = true;
	
	Camera* main_camera = reinterpret_cast<Camera*>(m_object_manager->Get_Obj(L"maincamera"));
	
	Object* obj = main_camera->Get_Bind_Obj();
	DirectX::XMFLOAT3 LUR = main_camera->Get_Bind_Offset_LUR();
	float distance = main_camera->Get_Distance();
	float lagging_degree = main_camera->Get_Lagging_Degree();
	DirectX::XMFLOAT4 quat = main_camera->Get_Rotate_RPY_4f();
	float right = main_camera->Get_Rotate_Right();

	// Door Scene
	main_camera->Set_Lagging_Degree(1.0f);
	main_camera->Bind_Obj(m_object_manager->Get_Obj(L"Door_Camera"), 0.0f, 20.0f, 0.0f, -300.0f);
	main_camera->Rst_Rotate();
	main_camera->Rotate_Right(0.5f);
	main_camera->Set_Rotate(DirectX::XMFLOAT4(0.0f, -0.7071f, 0.0f, 0.7071f));
	main_camera->Update(0.0f);
	main_camera->Set_Freezing_Time(DOOR_OPEN_TIME);

	// DOOR_OPEN_TIME 지나고, 원래 시점으로 변경
	main_camera->Bind_Obj(obj, LUR.x, LUR.y, LUR.z, distance);
	main_camera->Set_Lagging_Degree(lagging_degree);
	main_camera->Rst_Rotate();
	main_camera->Rotate_Right(right);
	main_camera->Set_Rotate(quat);
	
}

void TestScene::SetCameraLagging(float target_lagging, float time)
{
	m_Need_Lagging = true;
	m_camera_target_lagging = target_lagging;
	Camera_Lagging_Time = time;
}

void TestScene::SetEscape()
{
	Camera* main_camera = reinterpret_cast<Camera*>(m_object_manager->Get_Obj(L"maincamera"));
	main_camera->Set_Lagging_Degree(1.0f);
	main_camera->Bind_Obj(m_object_manager->Get_Obj(L"Door_Camera"), 0.0f, 20.0f, 0.0f, -300.0f);
	main_camera->Rst_Rotate();
	main_camera->Rotate_Right(0.5f);
	main_camera->Set_Rotate(DirectX::XMFLOAT4(0.0f, -0.7071f, 0.0f, 0.7071f));
	main_camera->Update(0.0f);
	main_camera->Set_Freezing_Time(DOOR_OPEN_TIME);

	ObservingMode();
}

void TestScene::ObservingMode()
{
	m_object_manager->Rst_Selected_Obj_Arr();
	NetworkManager& network = NetworkManager::GetInstance();
	for (int i = 0; i < 8; ++i)
	{
		m_object_manager->Add_Selected_Obj(network.GetObj(i));
	}

	m_input_manager->Rst_Manager();
	m_input_manager->Bind_Key_First_Down(VK_UP, BindingInfo(L"", Action::CUSTOM_FUNCTION_THREE));
	m_input_manager->Bind_Key_First_Down(VK_DOWN, BindingInfo(L"", Action::CUSTOM_FUNCTION_THREE));
	m_input_manager->Bind_Mouse_Move(BindingInfo(), BindingInfo(L"maincamera", Action::ROTATE_PITCH, 0.01f),
		BindingInfo(L"maincamera", Action::ROTATE_RIGHT, 0.01f));

	m_main_camera->Set_Lagging_Degree(10.0f);
}

void TestScene::ShowingResultScene(bool is_cat_winner)
{
	m_is_cat_winner = is_cat_winner;
	m_main_camera->Set_Lagging_Degree(1.0f);

	Result_Test_UI_Function();
}

void TestScene::RebornUICount()
{
	m_is_reborn = true;
}

void TestScene::AttackedUI()
{
	if (false == m_attacked)
	{
		m_attacked = true;
	}
}


