#include "SceneManager.h"
#include "Scene.h"
#include "DummyScene.h"

void SceneManager::Update() {
	if (!m_scene_stack.empty()) {
		m_scene_stack.top()->Update();
	}
}

void SceneManager::Draw() {
	if (!m_scene_stack.empty()) {
		m_scene_stack.top()->Draw(m_d3d_manager);
	}
}

void SceneManager::Prcs_Input(UINT message, WPARAM wparam, LPARAM lparam) {
	if (!m_scene_stack.empty()) {
		m_scene_stack.top()->Prcs_Input(message, wparam, lparam);
	}
}

void SceneManager::Chg_Scene(Scene* scene) {
	if (!m_scene_stack.empty()) {
		m_scene_stack.top()->Exit();
		m_scene_stack.pop();
	}

	m_scene_stack.push(Add_Scene_2_Map(scene));
	scene->Set_SM(this);
	scene->Enter();
}

void SceneManager::Push_Scene(Scene* scene, bool pause) {
	if (!m_scene_stack.empty()) {
		if (pause) {
			m_scene_stack.top()->Pause();
		}
	}

	m_scene_stack.push(Add_Scene_2_Map(scene));
	scene->Set_SM(this);
	scene->Enter();
}

void SceneManager::Pop_Scene() {
	if (!m_scene_stack.empty()) {
		m_scene_stack.top()->Exit();
		m_scene_stack.pop();
	}

	if (!m_scene_stack.empty()) {
		m_scene_stack.top()->Resume();
	}
}

Scene* SceneManager::Crt_Scene(std::wstring scene_name) {
	if (scene_name == L"Dummy") {
		Scene* result = new DummyScene(scene_name);
		return result;
	}
}

Scene* SceneManager::Add_Scene_2_Map(Scene* scene) {
	std::wstring scene_name = scene->Get_Name();

	if (!m_scene_map.count(scene_name)) {
		m_scene_map[scene_name] = scene;

		return scene;
	}

	OutputDebugString(L"Scene Already In Map\n");
	return nullptr;
}

Scene* SceneManager::Get_Scene(std::wstring scene_name) {
	if (m_scene_map.count(scene_name)) {
		return m_scene_map[scene_name];
	}

	OutputDebugString(L"Scene Not Found\n");
	return nullptr;
}
