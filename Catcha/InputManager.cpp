#include "InputManager.h"
#include "ObjectManager.h"
#include "Scene.h"

void InputManager::Rst_Manager() {
	//m_state[256] = { false };
	//m_previous_state[256] = { false };

	m_key_set.clear();

	m_key_down_map.clear();
	m_key_first_down_map.clear();
	m_key_up_map.clear();

	POINT m_previous_point = { -1, -1 };
	for (size_t i = 0; i < 3; ++i) {
		m_mouse_move_info[i] = BindingInfo();
	}

	//
	m_captured = false;

	//m_client_width = 0;
	//m_client_height = 0;

	m_hide_cursor = false;
	m_cursor = true;
	m_fix_cursor = false;

	//
	m_screen_point = { -1, -1 };
}

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
		m_state[wparam] = false;
		break;
	case WM_LBUTTONUP:
		m_state[VK_LBUTTON] = false;
		break;
	case WM_RBUTTONUP:
		m_state[VK_RBUTTON] = false;
		break;
	case WM_MBUTTONUP:
		m_state[VK_MBUTTON] = false;
		break;
	case WM_MOUSEMOVE:
		m_screen_point = { (int)(short)LOWORD(lparam), (int)(short)HIWORD(lparam) };
		break;
	default:
		break;
	}
}

void InputManager::Bind_Mouse_Move(BindingInfo binding_info_xy, BindingInfo binding_info_x, BindingInfo binding_info_y) {
	m_mouse_move_info[0] = binding_info_xy;
	m_mouse_move_info[1] = binding_info_x;
	m_mouse_move_info[2] = binding_info_y;
}

void InputManager::Prcs_Input() {
	if (GetActiveWindow() != GetForegroundWindow()) {
		m_previous_point.x = -1;
		m_previous_point.y = -1;

		if (m_captured) {
			ReleaseCapture();
			m_captured = false;
		}

		m_fix_cursor = false;
		m_hide_cursor = false;
		return;
	}
	else {
		if (!m_captured) {
			SetCapture(GetActiveWindow());
			m_captured = true;
		}
	}

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

					// [CS] 키보드 입력이 시작되었음을 알림
					switch (binding_info.action) {
					case Action::MOVE_FORWARD:
					case Action::MOVE_BACK:
					case Action::MOVE_LEFT:
					case Action::MOVE_RIGHT:
						input_key_ = (static_cast<uint8_t>(binding_info.action) << 1) | true;
						network_manager.SendInput(input_key_);
						break;
					}

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
		BindingInfo binding_info;

		if (m_mouse_move_info[0].action == Action::ACTION_NONE) {
			if (m_previous_point.x != new_point.x) {
				binding_info = m_mouse_move_info[1];
				binding_info.value = std::get<float>(binding_info.value) * (float)(new_point.x - m_previous_point.x);

				Prcs_Binding_Info(binding_info);
			}

			if (m_previous_point.y != new_point.y) {
				binding_info = m_mouse_move_info[2];
				binding_info.value = std::get<float>(binding_info.value) * (float)(new_point.y - m_previous_point.y);

				Prcs_Binding_Info(binding_info);
			}
		}
		else {
			if ((m_previous_point.x != new_point.x) || (m_previous_point.y != new_point.y)) {
				binding_info = m_mouse_move_info[0];
				
				POINTF pointf = std::get<POINTF>(binding_info.value);

				if (pointf.x == 0.0f && pointf.y == 0.0f) {
					binding_info.value = POINTF((float)m_screen_point.x, (float)m_screen_point.y);
				}
				else {
					binding_info.value = POINTF(
						pointf.x * ((float)(new_point.x - m_previous_point.x)),
						pointf.y * ((float)(new_point.y - m_previous_point.y)));
				}

				Prcs_Binding_Info(binding_info);
			}
		}
	}

	if (m_fix_cursor) {
		m_previous_point = { m_client_width / 2, m_client_height / 2 };
		ClientToScreen(GetActiveWindow(), &m_previous_point);
		SetCursorPos(m_previous_point.x, m_previous_point.y);
	}
	else {
		m_previous_point = new_point;
	}

	if (m_hide_cursor) {
		if (m_cursor) {
			while (ShowCursor(false) >= 0);
			m_cursor = false;
		}
	}
	else {
		if (!m_cursor) {
			while (ShowCursor(true) < 0);
			m_cursor = true;
		}
	}
}

void InputManager::Prcs_Binding_Info(BindingInfo binding_info) {
	if (binding_info.object_name != L"") {
		if (m_object_manager->Get_Obj(binding_info.object_name)->Get_Processable_Input() == false) {
			return;
		}

		switch (binding_info.action) {
		case Action::MOVE_FORWARD:
		case Action::MOVE_BACK:
		case Action::MOVE_LEFT:
		case Action::MOVE_RIGHT:
		case Action::MOVE_UP:
		case Action::MOVE_DOWN:
			m_object_manager->Move(binding_info.object_name, binding_info.action, std::get<BYTE>(binding_info.value));
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
			m_object_manager->Actions(binding_info.object_name, binding_info.action);
			break;
		}
	}
	else {
		switch (binding_info.action) {
		case Action::CHANGE_WIREFRAME_FLAG:
			m_scene->Chg_Wireframe_Flag();
			break;
		case Action::CHANGE_BOUNDINGBOX_FLAG:
			m_scene->Chg_Boundingbox_Flag();
			break;
		case Action::HIDE_CURSOR:
			Set_Hide_Cursor(!m_hide_cursor);
			break;
		case Action::FIX_CURSOR:
			Set_Fix_Cursor(!m_fix_cursor);
			break;
		case Action::HIDE_AND_FIX_CURSOR:
			Set_Hide_Cursor(!m_hide_cursor);
			Set_Fix_Cursor(!m_fix_cursor);
			break;
		case Action::CUSTOM_FUNCTION_ONE:
			m_scene->Custom_Function_One();
			break;
		case Action::CUSTOM_FUNCTION_TWO:
			m_scene->Custom_Function_Two();
			break;
		case Action::CUSTOM_FUNCTION_THREE:
			m_scene->Custom_Function_Three();
			break;
		case Action::PICKING:
			m_scene->Picking(std::get<POINTF>(binding_info.value));
			break;
		default:
			break;
		}
	}
}
