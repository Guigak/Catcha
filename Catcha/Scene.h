#pragma once
#include "common.h"

class SceneManager;
class D3DManager;

class Scene {
protected:
	std::wstring m_name = L"name";

	bool m_paused = false;

	Scene* m_back_scene = nullptr;

	SceneManager* m_scene_manager = nullptr;

public:
	Scene(std::wstring name, Scene* back_scene = nullptr) : m_name(name), m_back_scene(back_scene) {}

	virtual void Enter() {}
	virtual void Pause() { m_paused = true; }
	virtual void Resume() { m_paused = false; }
	virtual void Exit() {}

	virtual void Update() {}
	virtual void Draw(D3DManager* d3d_manager) {}

	virtual void Prcs_Input(UINT message, WPARAM wparam, LPARAM lparam) {}	// Process Message

	virtual std::wstring Get_Name() { return m_name; }

	virtual void Set_BS(Scene* back_scene) { m_back_scene = back_scene; }	// Set Back Scene
	virtual void Set_SM(SceneManager* scene_manager) { m_scene_manager = scene_manager; }	// Set Scene Manager
};

