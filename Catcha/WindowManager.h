#pragma once
#include "common.h"

class SceneManager;

class WindowManager {
private:
	HWND m_main_hwnd;

	int m_client_width;
	int m_client_height;

public:
	WindowManager() {}
	~WindowManager() {}

	bool Initialize(HINSTANCE hinstance, int width, int height, std::wstring name);

	HWND Get_Main_Hwnd() { return m_main_hwnd; }
	int Get_Client_Width() { return m_client_width; }
	int Get_Client_Height() { return m_client_height; }
};

void Set_SM(SceneManager* scene_manager);
