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

void InputManager::Bind_Mouse_Move(BindingInfo binding_info) {
	m_mouse_move_info = binding_info;
}

void InputManager::Prcs_Input() {
	// NetworkManager 싱글톤 인스턴스 사용
	NetworkManager& network_manager = NetworkManager::GetInstance();

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

				// [CS] 키보드 입력이 끝났음을 알림
				input_key_ = (static_cast<uint8_t>(binding_info.action) << 1) | false;
				network_manager.SendInput(input_key_);
			}
		}

		m_previous_state[k] = m_state[k];
	}

	POINT new_point;
	GetCursorPos(&new_point);

	if (m_previous_point.x != -1 && m_previous_point.y != -1) {
		if (m_previous_point.x != new_point.x || m_previous_point.y != new_point.y) {
			m_mouse_move_info.value = POINTF((float)(new_point.x - m_previous_point.x), (float)(new_point.y - m_previous_point.y));

			Prcs_Binding_Info(m_mouse_move_info);
		}
	}

	m_previous_point = new_point;
}

void InputManager::Prcs_Binding_Info(BindingInfo binding_info) {
	if (binding_info.object_name != L"") {
		// NetworkManager 싱글톤 인스턴스 사용
		NetworkManager& network_manager = NetworkManager::GetInstance();

		switch (binding_info.action) {
		case Action::MOVE_FORWARD:
		case Action::MOVE_BACK:
		case Action::MOVE_LEFT:
		case Action::MOVE_RIGHT:
		case Action::MOVE_UP:
		case Action::MOVE_DOWN:
			m_object_manager->Move(binding_info.object_name, binding_info.action);

			// [CS] 키보드 입력이 시작되었음을 알림
			input_key_ = (static_cast<uint8_t>(binding_info.action) << 1) | true;
			network_manager.SendInput(input_key_);

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
			m_object_manager->Rotate(binding_info.object_name, binding_info.action, std::get<POINTF>(binding_info.value));
			break;
		default:
			break;
		}
	}
}
