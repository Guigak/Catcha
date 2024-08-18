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

					if (binding_info.object) {
						switch (binding_info.action) {
						case Action::MOVE_FORWARD:
							m_object_manager->Teleport_Forward(binding_info.object);
							break;
						}
					}
				}
			}
			else {
				if (m_key_first_down_map.count(k)) {
					BindingInfo binding_info = m_key_first_down_map[k];

					if (binding_info.object) {
						switch (binding_info.action) {
						case Action::MOVE_FORWARD:
							m_object_manager->Teleport_Forward(binding_info.object);
							break;
						}
					}
				}
			}
		}
		else {
			if (m_previous_state[k] && m_key_up_map.count(k)) {
				BindingInfo binding_info = m_key_up_map[k];

				if (binding_info.object) {
					switch (binding_info.action) {

					}
				}
			}


		}

		m_previous_state[k] = m_state[k];
	}
}
