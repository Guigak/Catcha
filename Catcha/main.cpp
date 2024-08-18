#include "common.h"
#include "WindowManager.h"
#include "D3DManager.h"
#include "SceneManager.h"

WindowManager g_window_manager;
D3DManager g_d3d_manager;
SceneManager g_scene_manager;

int WINAPI WinMain(HINSTANCE hinstance, HINSTANCE prev_hinstance, LPSTR command_line, int command_show) {
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	try {
		if (!g_window_manager.Initialize(hinstance, CLIENT_WIDTH, CLIENT_HEIGHT, L"windowtest")) {
			return 0;
		}
		Set_SM(&g_scene_manager);

		if (!g_d3d_manager.Initialize(g_window_manager.Get_Main_Hwnd(), CLIENT_WIDTH, CLIENT_HEIGHT)) {
			return 0;
		}
		g_d3d_manager.Set_SM(&g_scene_manager);

		g_scene_manager.Set_D3DM(&g_d3d_manager);

		g_scene_manager.Chg_Scene(L"Test");

		MSG message = { 0 };

		while (message.message != WM_QUIT) {
			if (PeekMessage(&message, 0, 0, 0, PM_REMOVE)) {
				TranslateMessage(&message);
				DispatchMessage(&message);
			}
			else {
				g_scene_manager.Update();
				g_d3d_manager.Draw_Scene_With_FR();
			}
		}

		return 0;
	}
	catch (DXException& e) {
		MessageBox(nullptr, e.To_WStr().c_str(), L"Initialize Failed!!", MB_OK);

		return 0;
	}
}