#pragma once
#include "common.h"

class Scene;
class Object;
class ObjectManager;

enum class Action {
	ACTION_NONE,
	HIDE_CURSOR, FIX_CURSOR, HIDE_AND_FIX_CURSOR,
	CHANGE_WIREFRAME_FLAG, CHANGE_BOUNDINGBOX_FLAG,
	PICKING,
	CUSTOM_FUNCTION_ONE, CUSTOM_FUNCTION_TWO, CUSTOM_FUNCTION_THREE,
	MOVE_FORWARD, MOVE_BACK, MOVE_LEFT, MOVE_RIGHT, MOVE_UP, MOVE_DOWN,
	TELEPORT_FORWARD, TELEPORT_BACK, TELEPORT_LEFT, TELEPORT_RIGHT, TELEPORT_UP, TELEPORT_DOWN,
	ROTATE, ROTATE_ROLL, ROTATE_PITCH, ROTATE_YAW, ROTATE_RIGHT, ROTATE_LOOK,
	ACTION_JUMP, ACTION_ONE, ACTION_TWO, ACTION_THREE, ACTION_FOUR, ACTION_FIVE
};

struct BindingInfo {
	std::wstring object_name = L"";
	Action action = Action::ACTION_NONE;
	std::variant<BYTE, POINTF, int, float, std::wstring> value = 1.0f;
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
	BindingInfo m_mouse_move_info[3];	// 0 : x, y / 1 : x / 2 : y

	Scene* m_scene = nullptr;
	ObjectManager* m_object_manager = nullptr;

	//
	bool m_captured = false;

	int m_client_width = 0;
	int m_client_height = 0;

	bool m_hide_cursor = false;
	bool m_cursor = true;
	bool m_fix_cursor = false;

	//
	POINT m_screen_point = { -1, -1 };

public:
	InputManager() {}
	InputManager(Scene* scene, ObjectManager* object_manager, int client_width, int client_height)
		: m_scene(scene), m_object_manager(object_manager), m_client_width(client_width), m_client_height(client_height) {}
	~InputManager() {}

	void Rst_Manager();

	void Bind_Key_Down(int key_id, BindingInfo binding_info);
	void Bind_Key_First_Down(int key_id, BindingInfo binding_info);
	void Bind_Key_Up(int key_id, BindingInfo binding_info);

	void Bind_Mouse_Move(BindingInfo binding_info_xy, BindingInfo binding_info_x, BindingInfo binding_info_y);

	void Prcs_Input_Msg(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);

	void Prcs_Input();

	void Prcs_Binding_Info(BindingInfo binding_info);

	void Set_Hide_Cursor(bool hide_cursor) { m_hide_cursor = hide_cursor; }
	void Set_Fix_Cursor(bool fix_cursor) { m_fix_cursor = fix_cursor; }
};

