#include "InputManager.h"
#include "ObjectManager.h"

void InputManager::Bind_Key_Down(int key_id, BindingInfo binding_info) {
	m_key_down_map[key_id] = binding_info;
	m_key_set.insert(key_id);
}

void InputManager::Bind_Key_First_Down(int key_id, BindingInfo binding_info) {
	m_key_first_down_map[key_id] = binding_info;
	m_key_set.insert(key_id);
}

void InputManager::Bind_Key_Up(int key_id, BindingInfo binding_info) {
	m_key_up_map[key_id] = binding_info;
	m_key_set.insert(key_id);
}

void InputManager::Prcs_Input_Msg(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam) {
	switch (message) {
	case WM_KEYDOWN:
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_MBUTTONDOWN:
		m_state[wparam] = true;
		break;
	case WM_KEYUP:
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
	case WM_MBUTTONUP:
		m_state[wparam] = false;
		break;
	default:
		break;
	}
}

void InputManager::Prcs_Input() {
	for (auto& k : m_key_set) {
		if (m_state[k]) {
			if (m_previous_state[k]) {
				if (m_key_down_map.count(k)) {
					BindingInfo binding_info = m_key_down_map[k];

					Prcs_Binding_Info(binding_info);
				}
			}
			else {
				if (m_key_first_down_map.count(k)) {
					BindingInfo binding_info = m_key_first_down_map[k];

					Prcs_Binding_Info(binding_info);
				}
			}
		}
		else {
			if (m_previous_state[k] && m_key_up_map.count(k)) {
				BindingInfo binding_info = m_key_up_map[k];

				Prcs_Binding_Info(binding_info);
			}
		}

		m_previous_state[k] = m_state[k];
	}
}

void InputManager::Prcs_Binding_Info(BindingInfo binding_info) {
	if (binding_info.object_name != L"") {
		switch (binding_info.action) {
		case Action::MOVE_FORWARD:
		case Action::MOVE_BACK:
		case Action::MOVE_LEFT:
		case Action::MOVE_RIGHT:
			m_object_manager->Move(binding_info.object_name, binding_info.action);
			break;
		case Action::TELEPORT_FORWARD:
		case Action::TELEPORT_BACK:
		case Action::TELEPORT_LEFT:
		case Action::TELEPORT_RIGHT:
		case Action::TELEPORT_UP:
		case Action::TELEPORT_DOWN:
			m_object_manager->Teleport(binding_info.object_name, binding_info.action, binding_info.value);
			break;
		case Action::ROTATE_ROLL:
		case Action::ROTATE_PITCH:
		case Action::ROTATE_YAW:
			m_object_manager->Rotate(binding_info.object_name, binding_info.action, binding_info.value);
			break;
		}
	}
}