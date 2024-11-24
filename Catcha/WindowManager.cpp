#include "WindowManager.h"
#include "SceneManager.h"
#include "D3DManager.h"

SceneManager* m_scene_manager = nullptr;
D3DManager* m_d3d_manager = nullptr;

bool m_active = true;
bool m_command_console = false;

void Set_SM(SceneManager* scene_manager) {
    m_scene_manager = scene_manager;
}

void Set_D3DM(D3DManager* d3d_manager) {
    m_d3d_manager = d3d_manager;
}

void Prcs_Console_Cmd();

DWORD WINAPI Prcs_Console(LPVOID argument) {
    AllocConsole();
    SetConsoleTitle(TEXT("command console"));

    FILE* file_pointer;
    _tfreopen_s(&file_pointer, _T("CONOUT$"), _T("w"), stdout);
    _tfreopen_s(&file_pointer, _T("CONIN$"), _T("r"), stdin);
    _tfreopen_s(&file_pointer, _T("CONERR$"), _T("w"), stderr);
    _tsetlocale(LC_ALL, _T(""));

    Prcs_Console_Cmd();

    HWND hWndConsole = GetConsoleWindow();
    ShowWindow(hWndConsole, SW_HIDE);

    FreeConsole();

    return 0;
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

    m_main_hwnd = CreateWindow(window_class.lpszClassName, name.c_str(), WS_OVERLAPPEDWINDOW, 10, 10, new_width, new_height, 0, 0, hinstance, 0);
    
    if (!m_main_hwnd) {
        MessageBox(0, L"CreateWindow() Failed!!", 0, 0);

        return false;
    }

    ShowWindow(m_main_hwnd, SW_SHOW);
    UpdateWindow(m_main_hwnd);

    return true;
}

LRESULT CALLBACK Main_Wnd_Proc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam) {
    if (!m_active) {
        PostQuitMessage(0);

        return 0;
    }

    switch (message) {
    case WM_DESTROY:
        PostQuitMessage(0);

        return 0;
    case WM_SIZE:
        if (m_d3d_manager) {
            int client_width = LOWORD(lparam);
            int client_height = HIWORD(lparam);

            m_d3d_manager->Resize();
        }
        return 0;
    case WM_KEYUP:
        if (wparam == VK_ESCAPE) {
            PostQuitMessage(0);
        }
    case WM_KEYDOWN:
        if (wparam == VK_F8 && !m_command_console) {
            HANDLE hThread;

            hThread = CreateThread(NULL, 0, Prcs_Console, NULL, 0, NULL);

            if (hThread == NULL) {
                OutputDebugStringW(L"Create console fail\n");
            }
            else {
                CloseHandle(hThread);
            }

            m_command_console = true;
        }
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

void Prcs_Console_Cmd() {
    std::wstring command;

    while (1) {
        std::wcout << std::endl;
        std::wcout << L"Please enter a command" << std::endl;
        std::wcout << L"Close command console - exit" << std::endl;
        std::wcout << L"Quit program - quit" << std::endl;

        std::wcin >> command;

        while (std::wcin.get() != '\n');

        if (command == L"exit") {
            m_command_console = false;
            break;
        }
        else if (command == L"quit") {
            m_command_console = false;
            m_active = false;
            break;
        }
    }
}