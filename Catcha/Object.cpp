#include "Object.h"
#include "Camera.h"
#include "ObjectManager.h"

Object::Object(ObjectManager* object_manager, std::wstring object_name, Mesh_Info* mesh, DirectX::XMMATRIX world_matrix, UINT constant_buffer_index, D3D12_PRIMITIVE_TOPOLOGY primitive_topology, bool physics, bool visiable) {
	Set_Object_Manager(object_manager);
	Set_Name(object_name);
	Set_CB_Index(constant_buffer_index);
	Add_Mesh(mesh);
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
		// Calc Grav
		m_velocity.y -= m_gravity * elapsed_time;

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
		if (m_moving == false && m_grounded &&
			(m_state != Object_State::STATE_JUMP_START && m_state != Object_State::STATE_JUMP_IDLE)) {
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

		// Move
		m_position = MathHelper::Add(Get_Position_3f(), m_delta_position);

		// test	// modify later
		if (m_position.y < -61.592f) {
			m_position.y = -61.592f;
			m_velocity.y = 0.0f;

			m_grounded = true;

			if (m_state == Object_State::STATE_JUMP_IDLE) {
				m_next_state = Object_State::STATE_JUMP_END;
			}
		}
		else {
			if (m_next_state == Object_State::STATE_IDLE || m_next_state == Object_State::STATE_MOVE) {
				m_next_state = Object_State::STATE_JUMP_IDLE;
			}

			m_grounded = false;
		}

		Udt_WM();

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
		AnimationManager& animation_manager = m_object_manager->Get_Animation_Manager();

		if (m_grounded &&
			(m_next_state == Object_State::STATE_IDLE || m_next_state == Object_State::STATE_MOVE ||
			(m_moving == true && m_state == Object_State::STATE_JUMP_END))) {
			if (Get_Spd() > 0.05f) {
				m_next_state = Object_State::STATE_MOVE;
			}
			else {
				m_next_state = Object_State::STATE_IDLE;
			}
		}

		Animation_Binding_Info animation_binding_info = m_animation_binding_map[m_state];

		// checking animation end
		if (animation_binding_info.loop == false) {
			if (animation_manager.Get_Animation(animation_binding_info.binded_animation_name)->animation_time < m_animated_time) {
				m_next_state = animation_binding_info.next_object_state;
			}
		}

		// check next state
		if (m_state != m_next_state) {
			Animation_Binding_Info next_animation_binding_info = m_animation_binding_map[m_next_state];

			if (next_animation_binding_info.blending_time > 0.0f) {
				animation_manager.Get_Animated_Transform(
					animation_binding_info.binded_animation_name, m_animated_time, animation_binding_info.loop, m_blending_source_transform_info_array);
			}

			m_state = m_next_state;
			m_animated_time = 0.0f;

			m_movable = next_animation_binding_info.movable;
		}

		// calculate animation
		animation_binding_info = m_animation_binding_map[m_state];
		std::array<Transform_Info, MAX_BONE_COUNT> transform_info_array;
		animation_manager.Get_Animated_Transform(
			animation_binding_info.binded_animation_name, m_animated_time, animation_binding_info.loop, transform_info_array);

		// animation blending
		if (animation_binding_info.blending_time > m_animated_time) {
			for (UINT i = 0; i < m_skeleton_info->bone_count; ++i) {
				transform_info_array[i] = Interp_Trans_Info(m_blending_source_transform_info_array[i], transform_info_array[i],
					m_animated_time / animation_binding_info.blending_time);
			}
		}

		for (UINT i = 0; i < m_skeleton_info->bone_count; ++i) {
			DirectX::XMMATRIX translate_matrix = MathHelper::XMMATRIX_Translation(transform_info_array[i].translate);
			DirectX::XMMATRIX rotate_matrix = MathHelper::XMMATRIX_Rotation(transform_info_array[i].rotate);
			DirectX::XMMATRIX scale_matrix = MathHelper::XMMATRIX_Scaling(transform_info_array[i].scale);

			DirectX::XMMATRIX animation_matrix = scale_matrix * rotate_matrix * translate_matrix;
			DirectX::XMStoreFloat4x4(&m_animation_matrix_array[i], DirectX::XMMatrixTranspose(animation_matrix));
		}

		m_animated_time += elapsed_time;
		Rst_Dirty_Count();
	}

	if (m_dirty) {
		if (m_camera) {
			switch (m_camera_rotate_synchronization_flag) {
			case ROTATE_SYNC_NONE:
				break;
			case ROTATE_SYNC_ALL:
				m_rotate_roll_pitch_yaw = m_camera->Get_Rotate_RPY_4f();
				m_rotate_right = m_camera->Get_Rotate_Right();
				m_rotate_look = m_camera->Get_Rotate_Look();
				break;
			case ROTATE_SYNC_RPY:
				m_rotate_roll_pitch_yaw = m_camera->Get_Rotate_RPY_4f();
				break;
			default:
				break;
			}
		}

		Calc_Rotate();
		Udt_WM();
		Udt_LUR();

		Rst_Dirty_Count();

		m_dirty = false;
	}

	m_moving = false;
}

void Object::Udt_WM() {
	DirectX::XMMATRIX translate_matrix = DirectX::XMMatrixTranslation(m_position.x, m_position.y, m_position.z);
	DirectX::XMMATRIX rotate_matrix = DirectX::XMMatrixRotationQuaternion(DirectX::XMLoadFloat4(&m_rotate));
	DirectX::XMMATRIX scale_matrix = DirectX::XMMatrixScaling(m_scale.x, m_scale.y, m_scale.z);

	DirectX::XMStoreFloat4x4(&m_world_matrix, scale_matrix * rotate_matrix * translate_matrix);
}

void Object::Udt_LUR() {
	DirectX::XMMATRIX rotate_matrix = DirectX::XMMatrixRotationQuaternion(DirectX::XMLoadFloat4(&m_rotate));

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

void Object::Set_Rotate(float rotate_x, float rotate_y, float rotate_z, float rotate_w) {
	m_rotate = DirectX::XMFLOAT4(rotate_x, rotate_y, rotate_z, rotate_w);
}

void Object::Set_Scale(float scale_x, float scale_y, float scale_z) {
	m_scale = DirectX::XMFLOAT3(scale_x, scale_y, scale_z);
}

void Object::Set_Position(DirectX::XMFLOAT3 position) {
	m_position = position;
}

void Object::Set_Rotate(DirectX::XMFLOAT4 rotate) {
	m_rotate_roll_pitch_yaw = rotate;
}

void Object::Set_Scale(DirectX::XMFLOAT3 scale) {
	m_scale = scale;
}

void Object::Move(DirectX::XMFLOAT3 direction) {
	m_velocity = MathHelper::Add(Get_Vel(), direction, m_acceleration);

	m_moving = true;
}

void Object::Move_Forward(BYTE flag) {
	if (m_camera) {
		switch (flag) {
		case MOVE_ALL_AXIS:
			m_velocity = MathHelper::Add(Get_Vel(), m_camera->Get_Look(), m_acceleration);
			break;
		case MOVE_ONLY_XZ:
			m_velocity = MathHelper::Add(Get_Vel(), MathHelper::Get_XZ_Norm(m_camera->Get_Look()), m_acceleration);
			break;
		default:
			break;
		}
	}
	else {
		switch (flag) {
		case MOVE_ALL_AXIS:
			m_velocity = MathHelper::Add(Get_Vel(), Get_Look(), m_acceleration);
			break;
		case MOVE_ONLY_XZ:
			m_velocity = MathHelper::Add(Get_Vel(), MathHelper::Get_XZ_Norm(Get_Look()), m_acceleration);
			break;
		default:
			break;
		}
	}

	m_moving = true;
}

void Object::Move_Back(BYTE flag) {
	if (m_camera) {
		switch (flag) {
		case MOVE_ALL_AXIS:
			m_velocity = MathHelper::Add(Get_Vel(), m_camera->Get_Look(), -m_acceleration);
			break;
		case MOVE_ONLY_XZ:
			m_velocity = MathHelper::Add(Get_Vel(), MathHelper::Get_XZ_Norm(m_camera->Get_Look()), -m_acceleration);
			break;
		default:
			break;
		}
	}
	else {
		switch (flag) {
		case MOVE_ALL_AXIS:
			m_velocity = MathHelper::Add(Get_Vel(), Get_Look(), -m_acceleration);
			break;
		case MOVE_ONLY_XZ:
			m_velocity = MathHelper::Add(Get_Vel(), MathHelper::Get_XZ_Norm(Get_Look()), -m_acceleration);
			break;
		default:
			break;
		}
	}

	m_moving = true;
}

void Object::Move_Left(BYTE flag) {
	if (m_camera) {
		switch (flag) {
		case MOVE_ALL_AXIS:
			m_velocity = MathHelper::Add(Get_Vel(), m_camera->Get_Right(), -m_acceleration);
			break;
		case MOVE_ONLY_XZ:
			m_velocity = MathHelper::Add(Get_Vel(), MathHelper::Get_XZ_Norm(m_camera->Get_Right()), -m_acceleration);
			break;
		default:
			break;
		}
	}
	else {
		switch (flag) {
		case MOVE_ALL_AXIS:
			m_velocity = MathHelper::Add(Get_Vel(), Get_Right(), -m_acceleration);
			break;
		case MOVE_ONLY_XZ:
			m_velocity = MathHelper::Add(Get_Vel(), MathHelper::Get_XZ_Norm(Get_Right()), -m_acceleration);
			break;
		default:
			break;
		}
	}

	m_moving = true;
}

void Object::Move_Right(BYTE flag) {
	if (m_camera) {
		switch (flag) {
		case MOVE_ALL_AXIS:
			m_velocity = MathHelper::Add(Get_Vel(), m_camera->Get_Right(), m_acceleration);
			break;
		case MOVE_ONLY_XZ:
			m_velocity = MathHelper::Add(Get_Vel(), MathHelper::Get_XZ_Norm(m_camera->Get_Right()), m_acceleration);
			break;
		default:
			break;
		}
	}
	else {
		switch (flag) {
		case MOVE_ALL_AXIS:
			m_velocity = MathHelper::Add(Get_Vel(), Get_Right(), m_acceleration);
			break;
		case MOVE_ONLY_XZ:
			m_velocity = MathHelper::Add(Get_Vel(), MathHelper::Get_XZ_Norm(Get_Right()), m_acceleration);
			break;
		default:
			break;
		}
	}

	m_moving = true;
}

void Object::Move_Up(BYTE flag) {
	if (m_camera) {
		switch (flag) {
		case MOVE_ALL_AXIS:
			m_velocity = MathHelper::Add(Get_Vel(), m_camera->Get_Up(), m_acceleration);
			break;
		case MOVE_ONLY_XZ:
			m_velocity = MathHelper::Add(Get_Vel(), MathHelper::Get_XZ_Norm(m_camera->Get_Up()), m_acceleration);
			break;
		default:
			break;
		}
	}
	else {
		switch (flag) {
		case MOVE_ALL_AXIS:
			m_velocity = MathHelper::Add(Get_Vel(), Get_Up(), m_acceleration);
			break;
		case MOVE_ONLY_XZ:
			m_velocity = MathHelper::Add(Get_Vel(), MathHelper::Get_XZ_Norm(Get_Up()), m_acceleration);
			break;
		default:
			break;
		}
	}

	m_moving = true;
}

void Object::Move_Down(BYTE flag) {
	if (m_camera) {
		switch (flag) {
		case MOVE_ALL_AXIS:
			m_velocity = MathHelper::Add(Get_Vel(), m_camera->Get_Up(), -m_acceleration);
			break;
		case MOVE_ONLY_XZ:
			m_velocity = MathHelper::Add(Get_Vel(), MathHelper::Get_XZ_Norm(m_camera->Get_Up()), -m_acceleration);
			break;
		default:
			break;
		}
	}
	else {
		switch (flag) {
		case MOVE_ALL_AXIS:
			m_velocity = MathHelper::Add(Get_Vel(), Get_Up(), -m_acceleration);
			break;
		case MOVE_ONLY_XZ:
			m_velocity = MathHelper::Add(Get_Vel(), MathHelper::Get_XZ_Norm(Get_Up()), -m_acceleration);
			break;
		default:
			break;
		}
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
	DirectX::XMStoreFloat4(&m_rotate_roll_pitch_yaw,
		DirectX::XMQuaternionMultiply(DirectX::XMLoadFloat4(&m_rotate_roll_pitch_yaw),
			DirectX::XMQuaternionRotationAxis(DirectX::XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f), degree)));

	m_dirty = true;
}

void Object::Rotate_Pitch(float degree) {
	DirectX::XMStoreFloat4(&m_rotate_roll_pitch_yaw,
		DirectX::XMQuaternionMultiply(DirectX::XMLoadFloat4(&m_rotate_roll_pitch_yaw),
			DirectX::XMQuaternionRotationAxis(DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f), degree)));

	m_dirty = true;
}

void Object::Rotate_Yaw(float degree) {
	DirectX::XMStoreFloat4(&m_rotate_roll_pitch_yaw,
		DirectX::XMQuaternionMultiply(DirectX::XMLoadFloat4(&m_rotate_roll_pitch_yaw),
			DirectX::XMQuaternionRotationAxis(DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f), degree)));

	m_dirty = true;
}

void Object::Rotate_Right(float degree) {
	m_rotate_right += degree;

	//m_rotate_right = MathHelper::Min(RIGHT_ANGLE_RADIAN, MathHelper::Max(m_rotate_right, -RIGHT_ANGLE_RADIAN));

	m_dirty = true;
}

void Object::Rotate_Look(float degree) {
	m_rotate_look += degree;

	m_dirty = true;
}

void Object::Jump() {
	m_next_state = Object_State::STATE_JUMP_START;
	m_velocity.y = m_jump_power;
}

void Object::Act_One() {
	m_next_state = Object_State::STATE_ACTION_ONE;
}

void Object::Act_Two() {
	m_next_state = Object_State::STATE_ACTION_TWO;
}

void Object::Act_Three() {
	m_next_state = Object_State::STATE_ACTION_THREE;
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
	DirectX::XMVECTOR translate, rotate, scale;

	DirectX::XMMatrixDecompose(&scale, &rotate, &translate, world_matrix);

	DirectX::XMStoreFloat3(&m_position, translate);
	DirectX::XMStoreFloat4(&m_rotate_roll_pitch_yaw, rotate);
	DirectX::XMStoreFloat3(&m_scale, scale);

	Calc_Rotate();
	Udt_WM();

	//m_world_matrix = XMMATRIX_2_XMFLOAT4X4(world_matrix);
}

void Object::Draw(ID3D12GraphicsCommandList* command_list) {
	for (auto& m : m_meshes) {
		m.mesh_info->Draw(command_list);
	}
}

void Object::Bind_Anim_2_State(Object_State object_state, Animation_Binding_Info animation_binding_info) {
	m_animation_binding_map[object_state] = animation_binding_info;
}

void Object::Calc_Rotate() {
	DirectX::XMStoreFloat4(&m_rotate, DirectX::XMLoadFloat4(&m_rotate_roll_pitch_yaw));
	Udt_LUR();

	DirectX::XMStoreFloat4(&m_rotate, DirectX::XMQuaternionMultiply(
		DirectX::XMLoadFloat4(&m_rotate),
		DirectX::XMQuaternionRotationAxis(DirectX::XMLoadFloat3(&Get_Right()), m_rotate_right)));
	Udt_LUR();

	DirectX::XMStoreFloat4(&m_rotate, DirectX::XMQuaternionMultiply(
		DirectX::XMLoadFloat4(&m_rotate),
		DirectX::XMQuaternionRotationAxis(DirectX::XMLoadFloat3(&Get_Look()), m_rotate_look)));
}

void Object::Set_OBB(DirectX::BoundingOrientedBox obb) {
	m_OBB = obb;

	Set_Position(obb.Center);
	Set_Rotate(obb.Orientation);
	Set_Scale(obb.Extents);

	m_dirty = true;
}
