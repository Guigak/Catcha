#include "WindowManager.h"
#include "SceneManager.h"

SceneManager* m_scene_manager = nullptr;

void Set_SM(SceneManager* scene_manager) {
    m_scene_manager = scene_manager;
}

LRESULT CALLBACK Main_Wnd_Proc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);  // Main Window Procedure

bool WindowManager::Initialize(HINSTANCE hinstance, int width, int height, std::wstring name) {
    WNDCLASS window_class;
    window_class.style = CS_HREDRAW | CS_VREDRAW;
    window_class.lpfnWndProc = Main_Wnd_Proc;
    window_class.cbClsExtra = 0;
    window_class.cbWndExtra = 0;
    window_class.hInstance = hinstance;
    window_class.hIcon = LoadIcon(0, IDI_APPLICATION);
    window_class.hCursor = LoadCursor(0, IDC_ARROW);
    window_class.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
    window_class.lpszMenuName = 0;
    window_class.lpszClassName = L"MainWindow";

	if (!RegisterClass(&window_class)) {
		MessageBox(0, L"RegisterClass() Failed!!", 0, 0);

		return false;
    }

    m_client_width = width;
    m_client_height = height;

    RECT client_rect = { 0, 0, m_client_width, m_client_height };
    AdjustWindowRect(&client_rect, WS_OVERLAPPEDWINDOW, false);
    int new_width = client_rect.right - client_rect.left;
    int new_height = client_rect.bottom - client_rect.top;

    m_main_hwnd = CreateWindow(window_class.lpszClassName, name.c_str(), WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, new_width, new_height, 0, 0, hinstance, 0);
    
    if (!m_main_hwnd) {
        MessageBox(0, L"CreateWindow() Failed!!", 0, 0);

        return false;
    }

    ShowWindow(m_main_hwnd, SW_SHOW);
    UpdateWindow(m_main_hwnd);

    return true;
}

LRESULT CALLBACK Main_Wnd_Proc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam) {
    switch (message) {
    case WM_DESTROY:
        PostQuitMessage(0);

        return 0;
    case WM_KEYUP:
        if (wparam == VK_ESCAPE) {
            PostQuitMessage(0);
        }
    case WM_KEYDOWN:
    case WM_LBUTTONUP:
    case WM_LBUTTONDOWN:
    case WM_RBUTTONUP:
    case WM_RBUTTONDOWN:
    case WM_MBUTTONUP:
    case WM_MBUTTONDOWN:
        if (m_scene_manager) {
            m_scene_manager->Prcs_Input_Msg(hwnd, message, wparam, lparam);
        }

        return 0;
    default:
        return DefWindowProc(hwnd, message, wparam, lparam);
    }
}
