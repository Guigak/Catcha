#include "Object.h"
#include "Camera.h"
#include "ObjectManager.h"

Object::Object(ObjectManager* object_manager, std::wstring object_name, MeshInfo* mesh_info, std::wstring mesh_name,
	MaterialInfo* material_info, UINT constant_buffer_index, D3D12_PRIMITIVE_TOPOLOGY primitive_topology, bool physics, bool visiable)
{
	Set_Object_Manager(object_manager);
	Set_Name(object_name);
	Set_CB_Index(constant_buffer_index);
	Set_Mesh_Info(mesh_info, mesh_name);
	Set_Material_Info(material_info);
	Set_PT(primitive_topology);
	Set_Phys(physics);
	Set_Visiable(visiable);
}

Object::Object(ObjectManager* object_manager, std::wstring object_name, Mesh_Info* mesh, DirectX::XMMATRIX world_matrix, UINT constant_buffer_index, D3D12_PRIMITIVE_TOPOLOGY primitive_topology, bool physics, bool visiable) {
	Set_Object_Manager(object_manager);
	Set_Name(object_name);
	Set_CB_Index(constant_buffer_index);
	Add_Mesh(mesh, MathHelper::Identity_4x4());
	Set_WM(world_matrix);
	Set_PT(primitive_topology);
	Set_Phys(physics);
	Set_Visiable(visiable);
}

Object::Object(ObjectManager* object_manager, std::wstring object_name, std::vector<Mesh>& mesh_array, UINT constant_buffer_index, D3D12_PRIMITIVE_TOPOLOGY primitive_topology, bool physics, bool visiable) {
	Set_Object_Manager(object_manager);
	Set_Name(object_name);
	Set_CB_Index(constant_buffer_index);
	Add_Mesh(mesh_array);
	Set_PT(primitive_topology);
	Set_Phys(physics);
	Set_Visiable(visiable);
}

void Object::Set_Mesh_Info(MeshInfo* mesh_info, std::wstring mesh_name) {
	if (mesh_info == nullptr) {
		return;
	}

	m_mesh_info = mesh_info;

	m_submesh_name = mesh_name;

	m_index_count = m_mesh_info->submesh_map[mesh_name].index_count;
	m_start_index_location = m_mesh_info->submesh_map[mesh_name].start_index_location;
	m_base_vertex_location = m_mesh_info->submesh_map[mesh_name].base_vertex_location;
}

void Object::Chg_Mesh(std::wstring mesh_name) {
	if (mesh_name == L"") {
		return;
	}

	m_submesh_name = mesh_name;

	m_index_count = m_mesh_info->submesh_map[mesh_name].index_count;
	m_start_index_location = m_mesh_info->submesh_map[mesh_name].start_index_location;
	m_base_vertex_location = m_mesh_info->submesh_map[mesh_name].base_vertex_location;
}

void Object::Crt_Simple_OBB() {
	DirectX::XMFLOAT3 center(
		(m_mesh_info->submesh_map[m_submesh_name].maximum_x + m_mesh_info->submesh_map[m_submesh_name].minimum_x) / 2.0f,
		(m_mesh_info->submesh_map[m_submesh_name].maximum_y + m_mesh_info->submesh_map[m_submesh_name].minimum_y) / 2.0f,
		(m_mesh_info->submesh_map[m_submesh_name].maximum_z + m_mesh_info->submesh_map[m_submesh_name].minimum_z) / 2.0f
	);

	DirectX::XMFLOAT3 extents(
		(m_mesh_info->submesh_map[m_submesh_name].maximum_x - m_mesh_info->submesh_map[m_submesh_name].minimum_x) / 2.0f,
		(m_mesh_info->submesh_map[m_submesh_name].maximum_y - m_mesh_info->submesh_map[m_submesh_name].minimum_y) / 2.0f,
		(m_mesh_info->submesh_map[m_submesh_name].maximum_z - m_mesh_info->submesh_map[m_submesh_name].minimum_z) / 2.0f
	);

	DirectX::XMFLOAT4 orientation(0.0f, 0.0f, 0.0f, 1.0f);

	m_OBB.Center = center;
	m_OBB.Extents = extents;
	m_OBB.Orientation = orientation;
}

DirectX::XMMATRIX Object::Get_OBB_WM() {
	DirectX::XMMATRIX scale_matrix = DirectX::XMMatrixScaling(m_OBB.Extents.x * 2.0f, m_OBB.Extents.y * 2.0f, m_OBB.Extents.z * 2.0f);
	DirectX::XMMATRIX rotate_matrix = DirectX::XMMatrixRotationQuaternion(DirectX::XMLoadFloat4(&m_OBB.Orientation));
	DirectX::XMMATRIX translate_matrix = DirectX::XMMatrixTranslation(m_OBB.Center.x, m_OBB.Center.y, m_OBB.Center.z);

	return scale_matrix * rotate_matrix * translate_matrix;
}

DirectX::BoundingOrientedBox Object::Get_Calcd_OBB() {
	DirectX::BoundingOrientedBox Calculated_OBB;
	Calculated_OBB.Transform(m_OBB, DirectX::XMLoadFloat4x4(&m_world_matrix));

	return Calculated_OBB;
}

void Object::Calc_Delta(float elapsed_time) {
	m_delta_position = DirectX::XMFLOAT3();

	if (m_physics) {
		// Calc Vel
		m_speed = MathHelper::Length_XZ(Get_Vel());

		if (m_speed > m_max_speed) {
			m_velocity.x *= (m_max_speed / m_speed);
			m_velocity.z *= (m_max_speed / m_speed);

			m_speed = m_max_speed;
		}

		DirectX::XMFLOAT3 delta = MathHelper::Multiply(Get_Vel(), elapsed_time);

		m_delta_position = MathHelper::Add(m_delta_position, delta);

		// Calc deceleration
		if (m_moving == false && m_state == Object_State::IDLE_STATE) {
			if (m_speed > 0.0f) {
				float deceleration = m_deceleration * elapsed_time;
				float new_speed = MathHelper::Max(m_speed - deceleration, 0.0f);

				m_velocity = MathHelper::Multiply(Get_Vel(), new_speed / m_speed);
			}
		}

		// Calc Force
		if (m_force.x != 0.0f || m_force.y != 0.0f || m_force.z != 0.0f) {
			delta = MathHelper::Multiply(Get_Force(), elapsed_time);

			m_delta_position = MathHelper::Add(m_delta_position, delta);
		}

		// Calc friction
		float speed = MathHelper::Length(delta);

		if (speed > 0.0f) {
			float deceleration = m_deceleration * elapsed_time;
			float new_speed = MathHelper::Max(speed - deceleration, 0.0f);

			m_force = MathHelper::Multiply(Get_Force(), new_speed / speed);
		}
		
		// Calc Grav
		//m_velocity.y += m_gravity;

		// Move
		m_position = MathHelper::Add(Get_Position_3f(), m_delta_position);

		Udt_WM();

		m_moving = false;
		m_dirty = true;
	}
}

//void Object::Move_N_Solve_Collision() {
//	m_position = MathHelper::Add(Get_Position(), m_delta_position);
//
//	// solve collision
//}

void Object::Update(float elapsed_time) {
	if (m_animated) {
		m_animated_time += elapsed_time / 5.0f;

		m_object_manager->Get_Animation_Manager().Get_Animated_Matrix(m_playing_animation_name, m_animated_time, m_animation_matrix_array);

		Rst_Dirty_Count();
	}

	if (m_dirty) {
		if (m_camera) {
			//m_rotate = m_camera->Get_Rotate_3f();
		}

		Udt_WM();
		Udt_LUR();

		Rst_Dirty_Count();

		m_dirty = false;
	}
}

void Object::Udt_WM() {
	DirectX::XMMATRIX translate_matrix = DirectX::XMMatrixTranslation(m_position.x, m_position.y, m_position.z);
	//DirectX::XMMATRIX rotate_matrix = DirectX::XMMatrixRotationRollPitchYaw(
	//	DirectX::XMConvertToRadians(m_rotate.x), DirectX::XMConvertToRadians(m_rotate.y), DirectX::XMConvertToRadians(m_rotate.z));
	DirectX::XMMATRIX rotate_matrix = DirectX::XMMatrixRotationQuaternion(DirectX::XMLoadFloat4(&m_rotate_quat));
	DirectX::XMMATRIX scale_matrix = DirectX::XMMatrixScaling(m_scale.x, m_scale.y, m_scale.z);

	DirectX::XMStoreFloat4x4(&m_world_matrix, scale_matrix * rotate_matrix * translate_matrix);
}

void Object::Udt_LUR() {
	//DirectX::XMMATRIX rotate_matrix = DirectX::XMMatrixRotationRollPitchYaw(
	//	DirectX::XMConvertToRadians(m_rotate.x), DirectX::XMConvertToRadians(m_rotate.y), DirectX::XMConvertToRadians(m_rotate.z));
	DirectX::XMMATRIX rotate_matrix = DirectX::XMMatrixRotationQuaternion(DirectX::XMLoadFloat4(&m_rotate_quat));

	m_look = MathHelper::Normalize(MathHelper::Multiply(DirectX::XMFLOAT3(0.0f, 0.0f, 1.0f), rotate_matrix));
	m_up = MathHelper::Normalize(MathHelper::Multiply(DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f), rotate_matrix));
	m_right = MathHelper::Normalize(MathHelper::Cross(Get_Up(), Get_Look()));
}

void Object::Set_Position(float position_x, float position_y, float position_z) {
	m_position.x = position_x;
	m_position.y = position_y;
	m_position.z = position_z;

	m_dirty = true;
}

void Object::Move(DirectX::XMFLOAT3 direction) {
	m_velocity = MathHelper::Add(Get_Vel(), direction, m_acceleration);

	m_moving = true;
}

void Object::Move_Forward() {
	if (m_camera) {
		m_velocity = MathHelper::Add(Get_Vel(), m_camera->Get_Look(), m_acceleration);
	}
	else {
		m_velocity = MathHelper::Add(Get_Vel(), Get_Look(), m_acceleration);
	}

	m_moving = true;
}

void Object::Move_Back() {
	if (m_camera) {
		m_velocity = MathHelper::Add(Get_Vel(), m_camera->Get_Look(), -m_acceleration);
	}
	else {
		m_velocity = MathHelper::Add(Get_Vel(), Get_Look(), -m_acceleration);
	}

	m_moving = true;
}

void Object::Move_Left() {
	if (m_camera) {
		m_velocity = MathHelper::Add(Get_Vel(), m_camera->Get_Right(), -m_acceleration);
	}
	else {
		m_velocity = MathHelper::Add(Get_Vel(), Get_Right(), -m_acceleration);
	}

	m_moving = true;
}

void Object::Move_Right() {
	if (m_camera) {
		m_velocity = MathHelper::Add(Get_Vel(), m_camera->Get_Right(), m_acceleration);
	}
	else {
		m_velocity = MathHelper::Add(Get_Vel(), Get_Right(), m_acceleration);
	}

	m_moving = true;
}

void Object::Move_Up() {
	if (m_camera) {
		m_velocity = MathHelper::Add(Get_Vel(), m_camera->Get_Up(), m_acceleration);
	}
	else {
		m_velocity = MathHelper::Add(Get_Vel(), Get_Up(), m_acceleration);
	}

	m_moving = true;
}

void Object::Move_Down() {
	if (m_camera) {
		m_velocity = MathHelper::Add(Get_Vel(), m_camera->Get_Up(), -m_acceleration);
	}
	else {
		m_velocity = MathHelper::Add(Get_Vel(), Get_Up(), -m_acceleration);
	}

	m_moving = true;
}

void Object::Teleport(DirectX::XMFLOAT3 direction, float distance) {
	m_position = MathHelper::Add(Get_Position_3f(), direction, distance);

	m_dirty = true;
}

void Object::TP_Forward(float distance) {
	if (m_camera) {
		m_position = MathHelper::Add(Get_Position_3f(), m_camera->Get_Look(), distance);
	}
	else {
		m_position = MathHelper::Add(Get_Position_3f(), Get_Look(), distance);
	}

	m_dirty = true;
}

void Object::TP_Back(float distance) {
	if (m_camera) {
		m_position = MathHelper::Add(Get_Position_3f(), m_camera->Get_Look(), -distance);
	}
	else {
		m_position = MathHelper::Add(Get_Position_3f(), Get_Look(), -distance);
	}

	m_dirty = true;
}

void Object::TP_Left(float distance) {
	if (m_camera) {
		m_position = MathHelper::Add(Get_Position_3f(), m_camera->Get_Right(), -distance);
	}
	else {
		m_position = MathHelper::Add(Get_Position_3f(), Get_Right(), -distance);
	}

	m_dirty = true;
}

void Object::TP_Right(float distance) {
	if (m_camera) {
		m_position = MathHelper::Add(Get_Position_3f(), m_camera->Get_Right(), distance);
	}
	else {
		m_position = MathHelper::Add(Get_Position_3f(), Get_Right(), distance);
	}

	m_dirty = true;
}

void Object::TP_Up(float distance) {
	if (m_camera) {
		m_position = MathHelper::Add(Get_Position_3f(), m_camera->Get_Up(), distance);
	}
	else {
		m_position = MathHelper::Add(Get_Position_3f(), Get_Up(), distance);
	}

	m_dirty = true;
}

void Object::TP_Down(float distance) {
	if (m_camera) {
		m_position = MathHelper::Add(Get_Position_3f(), m_camera->Get_Up(), -distance);
	}
	else {
		m_position = MathHelper::Add(Get_Position_3f(), Get_Up(), -distance);
	}

	m_dirty = true;
}

void Object::Rotate(float degree_roll, float degree_pitch, float degree_yaw) {
	Rotate_Roll(degree_roll / 100.0f);
	Rotate_Pitch(degree_pitch / 100.0f);
	Rotate_Yaw(degree_yaw / 100.0f);
}

void Object::Rotate_Roll(float degree) {
	DirectX::XMStoreFloat4(&m_rotate_quat,
		DirectX::XMQuaternionMultiply(DirectX::XMLoadFloat4(&m_rotate_quat),
			DirectX::XMQuaternionRotationAxis(DirectX::XMLoadFloat3(&Get_Right()), degree)));

	m_dirty = true;
}

void Object::Rotate_Pitch(float degree) {
	DirectX::XMStoreFloat4(&m_rotate_quat,
		DirectX::XMQuaternionMultiply(DirectX::XMLoadFloat4(&m_rotate_quat),
			DirectX::XMQuaternionRotationAxis(DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f), degree)));

	m_dirty = true;
}

void Object::Rotate_Yaw(float degree) {
	DirectX::XMStoreFloat4(&m_rotate_quat,
		DirectX::XMQuaternionMultiply(DirectX::XMLoadFloat4(&m_rotate_quat),
			DirectX::XMQuaternionRotationAxis(DirectX::XMLoadFloat3(&Get_Look()), degree)));

	m_dirty = true;
}

void Object::Bind_Camera(Camera* camera) {
	m_camera = camera;
}

void Object::Add_Mesh(Mesh_Info* mesh_info, DirectX::XMFLOAT4X4 local_transform_matrix) {
	m_meshes.emplace_back(mesh_info, local_transform_matrix);
}

void Object::Add_Mesh(std::vector<Mesh>& mesh_array) {
	m_meshes.insert(m_meshes.end(), mesh_array.begin(), mesh_array.end());
}

void Object::Set_WM(DirectX::XMMATRIX world_matrix) {
	//world_matrix = DirectX::XMMatrixMultiply(world_matrix, DirectX::XMMatrixRotationRollPitchYaw(0.0f, 0.0f, DirectX::XMConvertToRadians(180.0f)));

	DirectX::XMVECTOR translate, rotate, scale;

	DirectX::XMMatrixDecompose(&scale, &rotate, &translate, world_matrix);

	DirectX::XMStoreFloat3(&m_position, translate);
	DirectX::XMStoreFloat4(&m_rotate_quat, rotate);
	DirectX::XMStoreFloat3(&m_scale, scale);

	Udt_WM();

	//m_world_matrix = XMMATRIX_2_XMFLOAT4X4(world_matrix);
}

void Object::Draw(ID3D12GraphicsCommandList* command_list) {
	for (auto& m : m_meshes) {
		m.mesh_info->Draw(command_list);
	}
}
