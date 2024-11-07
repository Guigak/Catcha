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

void InputManager::Bind_Mouse_Move(BindingInfo binding_info_x, BindingInfo binding_info_y) {
	m_mouse_move_info[0] = binding_info_x;
	m_mouse_move_info[1] = binding_info_y;
}

void InputManager::Prcs_Input() {
	if (GetActiveWindow() != GetForegroundWindow()) {
		m_previous_point.x = -1;
		m_previous_point.y = -1;
		return;
	}
	else {
	}

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

	POINT new_point;
	GetCursorPos(&new_point);
	if (m_previous_point.x != -1 && m_previous_point.y != -1) {
		BindingInfo binding_info;

		if (m_previous_point.x != new_point.x) {
			binding_info = m_mouse_move_info[0];
			binding_info.value = std::get<float>(binding_info.value) * (float)(new_point.x - m_previous_point.x);

			Prcs_Binding_Info(binding_info);
		}

		if (m_previous_point.y != new_point.y) {
			binding_info = m_mouse_move_info[1];
			binding_info.value = std::get<float>(binding_info.value) * (float)(new_point.y - m_previous_point.y);

			Prcs_Binding_Info(binding_info);
		}
	}

	m_previous_point = new_point;
}

void InputManager::Prcs_Binding_Info(BindingInfo binding_info) {
	if (binding_info.object_name != L"") {
		switch (binding_info.action) {
		case Action::MOVE_FORWARD:
		case Action::MOVE_BACK:
		case Action::MOVE_LEFT:
		case Action::MOVE_RIGHT:
		case Action::MOVE_UP:
		case Action::MOVE_DOWN:
			m_object_manager->Move(binding_info.object_name, binding_info.action, std::get<bool>(binding_info.value));
			break;
		case Action::TELEPORT_FORWARD:
		case Action::TELEPORT_BACK:
		case Action::TELEPORT_LEFT:
		case Action::TELEPORT_RIGHT:
		case Action::TELEPORT_UP:
		case Action::TELEPORT_DOWN:
			m_object_manager->Teleport(binding_info.object_name, binding_info.action, std::get<float>(binding_info.value));
			break;
		case Action::ROTATE:
		case Action::ROTATE_ROLL:
		case Action::ROTATE_PITCH:
		case Action::ROTATE_YAW:
		case Action::ROTATE_RIGHT:
		case Action::ROTATE_LOOK:
			m_object_manager->Rotate(binding_info.object_name, binding_info.action, std::get<float>(binding_info.value));
			break;
		default:
			break;
		}
	}
}
