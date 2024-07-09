#pragma once
#include "common.h"

class Scene;
class D3DManager;

class SceneManager {
private:
	std::stack<Scene*> m_scene_stack;

	D3DManager* m_d3d_manager = nullptr;

	std::unordered_map<std::wstring, Scene*> m_scene_map;

public:
	SceneManager() {}
	~SceneManager() {}

	void Update();
	void Draw();

	void Prcs_Input(UINT message, WPARAM wparam, LPARAM lparam);	// Process Input

	void Chg_Scene(Scene* scene);	// Change Scene
	void Chg_Scene(std::wstring scene_name, std::wstring back_scene_name = L"");	// Change Scene
	void Push_Scene(Scene* scene, bool pause = false);
	void Push_Scene(std::wstring scene_name, std::wstring back_scene_name = L"", bool pause = false);
	void Pop_Scene();

	Scene* Crt_Scene(std::wstring scene_name);
	Scene* Add_Scene_2_Map(Scene* scene);
	Scene* Get_Scene(std::wstring scene_name);

	void Set_D3DM(D3DManager* d3d_manager) { m_d3d_manager = d3d_manager; }	// Set D3DManager
};

