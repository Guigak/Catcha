#include "Player.h"
#include "ObjectManager.h"
#include "AnimationManager.h"
#include "Camera.h"

#define CHARGING_JUMP_FORCE 250.0f
#define MAX_CHARGING_TIME 3.0f

Player::Player(ObjectManager* object_manager, std::wstring object_name, Mesh_Info* mesh, DirectX::XMMATRIX world_matrix, UINT constant_buffer_index, D3D12_PRIMITIVE_TOPOLOGY primitive_topology, bool physics, bool visible)
	: Object(object_manager, object_name, mesh, world_matrix, constant_buffer_index, primitive_topology, physics, visible) {
	m_friction = 100.0f;
}

void Player::Update(float elapsed_time) {
	if (m_bool_values[L"jump_ready"]) {
		m_next_state = Object_State::STATE_ACTION_FOUR;

		m_float_values[L"charging_time"] += elapsed_time;

		m_float_values[L"charging_time"] = MathHelper::Max(m_float_values[L"charging_time"], MAX_CHARGING_TIME);
	}

	Object::Update(elapsed_time);
}

void Player::Act_Four() {
	m_bool_values[L"jump_ready"] = true;

	m_next_state = Object_State::STATE_ACTION_FOUR;
	m_animated_time = 0.0f;

	m_float_values[L"charging_time"] = 0.0f;
}

void Player::Act_Five() {
	m_bool_values[L"jump_ready"] = false;

	m_next_state = Object_State::STATE_JUMP_START;

	if (m_camera) {
		m_force = MathHelper::Add(m_force, MathHelper::Multiply(
			m_camera->Get_Look(), CHARGING_JUMP_FORCE * m_float_values[L"charging_time"]));
	}
}
