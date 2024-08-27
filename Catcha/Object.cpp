#include "Object.h"

Object::Object(std::wstring object_name, MeshInfo* mesh_info, std::wstring mesh_name,
	MaterialInfo* material_info, UINT constant_buffer_index, D3D12_PRIMITIVE_TOPOLOGY primitive_topology, bool physics)
{
	Set_Name(object_name);
	Set_CB_Index(constant_buffer_index);
	Set_Mesh_Info(mesh_info, mesh_name);
	Set_Material_Info(material_info);
	Set_PT(primitive_topology);
	Set_Phys(physics);
}

void Object::Set_Mesh_Info(MeshInfo* mesh_info, std::wstring mesh_name) {
	m_mesh_info = mesh_info;
	m_index_count = m_mesh_info->submesh_map[mesh_name].index_count;
	m_start_index_location = m_mesh_info->submesh_map[mesh_name].start_index_location;
	m_base_vertex_location = m_mesh_info->submesh_map[mesh_name].base_vertex_location;
}

void Object::Chg_Mesh(std::wstring mesh_name) {
	m_index_count = m_mesh_info->submesh_map[mesh_name].index_count;
	m_start_index_location = m_mesh_info->submesh_map[mesh_name].start_index_location;
	m_base_vertex_location = m_mesh_info->submesh_map[mesh_name].base_vertex_location;
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
		m_position = MathHelper::Add(Get_Position(), m_delta_position);

		m_moving = false;
		m_dirty = true;
	}
}

//void Object::Move_N_Solve_Collision() {
//	m_position = MathHelper::Add(Get_Position(), m_delta_position);
//
//	// solve collision
//}

void Object::Udt_WM() {
	if (m_dirty) {
		DirectX::XMMATRIX trans_matrix = DirectX::XMMatrixTranslation(m_position.x, m_position.y, m_position.z);
		DirectX::XMMATRIX rotate_matrix = DirectX::XMMatrixRotationRollPitchYaw(
			DirectX::XMConvertToRadians(m_rotate.x), DirectX::XMConvertToRadians(m_rotate.y), DirectX::XMConvertToRadians(m_rotate.z));
		DirectX::XMMATRIX scale_matrix = DirectX::XMMatrixTranslation(m_scale.x, m_scale.y, m_scale.z);

		DirectX::XMStoreFloat4x4(&m_world_matrix, scale_matrix * rotate_matrix * trans_matrix);

		Rst_Dirty_Count();

		m_dirty = false;
	}
}

void Object::Move_Forward() {
	m_velocity = MathHelper::Add(Get_Vel(), Get_Look(), m_acceleration);

	m_moving = true;
}

void Object::Move_Back() {
	m_velocity = MathHelper::Add(Get_Vel(), Get_Look(), -m_acceleration);

	m_moving = true;
}

void Object::Move_Left() {
	m_velocity = MathHelper::Add(Get_Vel(), Get_Right(), -m_acceleration);

	m_moving = true;
}

void Object::Move_Right() {
	m_velocity = MathHelper::Add(Get_Vel(), Get_Right(), m_acceleration);

	m_moving = true;
}

void Object::Move_Up() {
	m_velocity = MathHelper::Add(Get_Vel(), Get_Up(), m_acceleration);

	m_moving = true;
}

void Object::Move_Down() {
	m_velocity = MathHelper::Add(Get_Vel(), Get_Up(), -m_acceleration);

	m_moving = true;
}

void Object::TP_Forward(float distance) {
	m_position = MathHelper::Add(Get_Position(), Get_Look(), distance);

	m_dirty = true;
}

void Object::TP_Back(float distance) {
	m_position = MathHelper::Add(Get_Position(), Get_Look(), -distance);

	m_dirty = true;
}

void Object::TP_Left(float distance) {
	m_position = MathHelper::Add(Get_Position(), Get_Right(), -distance);

	m_dirty = true;
}

void Object::TP_Right(float distance) {
	m_position = MathHelper::Add(Get_Position(), Get_Right(), distance);

	m_dirty = true;
}

void Object::TP_Up(float distance) {
	m_position = MathHelper::Add(Get_Position(), Get_Up(), distance);

	m_dirty = true;
}

void Object::TP_Down(float distance) {
	m_position = MathHelper::Add(Get_Position(), Get_Up(), distance);

	m_dirty = true;
}
