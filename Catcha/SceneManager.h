#pragma once
#include "common.h"
#include "Scene.h"

class D3DManager;

class SceneManager {
private:
	std::stack<Scene*> m_scene_stack;

	D3DManager* m_d3d_manager = nullptr;

	std::unordered_map<std::wstring, std::unique_ptr<Scene>> m_scene_map;

public:
	SceneManager() {}
	~SceneManager();

	void Update(float elapsed_time);
	void Resize();
	void Draw(ID3D12CommandList** command_lists);

	void Prcs_Input_Msg(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);	// Process Input

	void Chg_Scene(std::unique_ptr<Scene>);	// Change Scene
	void Chg_Scene(std::wstring scene_name, std::wstring back_scene_name = L"");	// Change Scene
	void Push_Scene(std::unique_ptr<Scene>, bool pause = false);
	void Push_Scene(std::wstring scene_name, std::wstring back_scene_name = L"", bool pause = false);
	void Pop_Scene();

	std::unique_ptr<Scene> Crt_Scene(std::wstring scene_name);
	Scene* Add_Scene_2_Map(std::unique_ptr<Scene> scene);
	Scene* Get_Scene(std::wstring scene_name);

	void Set_D3DM(D3DManager* d3d_manager) { m_d3d_manager = d3d_manager; }	// Set D3DManager
};

