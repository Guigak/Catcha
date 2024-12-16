#include "common.h"
#include "WindowManager.h"
#include "D3DManager.h"
#include "SceneManager.h"
#include "Timer.h"

WindowManager g_window_manager;
D3DManager g_d3d_manager;
SceneManager g_scene_manager;
Timer g_timer;

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

		g_timer.Reset();

		// loop
		while (message.message != WM_QUIT) {
			if (PeekMessage(&message, 0, 0, 0, PM_REMOVE)) {
				TranslateMessage(&message);
				DispatchMessage(&message);
			}
			else {
				g_timer.Tick();

				//
				static int frame_count = 0;
				static float time_elapsed = 0.0f;

				frame_count++;

				if ((g_timer.Get_Total_Time() - time_elapsed) >= 1.0f) {
					float fps = (float)frame_count;

					OutputDebugStringW(std::to_wstring(fps).c_str());
					OutputDebugStringW(L"\n");

					frame_count = 0;
					time_elapsed += 1.0f;
				}

				g_scene_manager.Update(g_timer.Get_Elapsed_Time());
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