#pragma once
#include "common.h"

class Scene;
class Object;
class ObjectManager;

enum class Action {
	ACTION_NONE,
	MOVE_FORWARD, MOVE_BACK, MOVE_LEFT, MOVE_RIGHT, MOVE_UP, MOVE_DOWN,
	TELEPORT_FORWARD, TELEPORT_BACK, TELEPORT_LEFT, TELEPORT_RIGHT, TELEPORT_UP, TELEPORT_DOWN,
	ROTATE, ROTATE_ROLL, ROTATE_PITCH, ROTATE_YAW, ROTATE_RIGHT, ROTATE_LOOK
};

struct BindingInfo {
	std::wstring object_name = L"";
	Action action = Action::ACTION_NONE;
	std::variant<BYTE, int, float, std::wstring> value = 1.0f;
};

class InputManager {
private:
	bool m_state[256] = { false };
	bool m_previous_state[256] = { false };

	std::unordered_set<int> m_key_set;

	std::unordered_map<int, BindingInfo> m_key_down_map;
	std::unordered_map<int, BindingInfo> m_key_first_down_map;
	std::unordered_map<int, BindingInfo> m_key_up_map;

	POINT m_previous_point = { -1, -1 };
	BindingInfo m_mouse_move_info[2];	// 0 : x, 1 : y

	Scene* m_scene = nullptr;
	ObjectManager* m_object_manager = nullptr;

public:
	InputManager() {}
	InputManager(Scene* scene, ObjectManager* object_manager) : m_scene(scene), m_object_manager(object_manager) {}
	~InputManager() {}

	void Bind_Key_Down(int key_id, BindingInfo binding_info);
	void Bind_Key_First_Down(int key_id, BindingInfo binding_info);
	void Bind_Key_Up(int key_id, BindingInfo binding_info);

	void Bind_Mouse_Move(BindingInfo binding_info_x, BindingInfo binding_info_y);

	void Prcs_Input_Msg(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);

	void Prcs_Input();

	void Prcs_Binding_Info(BindingInfo binding_info);
};

