#include "SceneManager.h"
#include "DummyScene.h"
#include "TestScene.h"

void SceneManager::Update() {
	if (!m_scene_stack.empty()) {
		m_scene_stack.top()->Update(m_d3d_manager);
	}
}

void SceneManager::Resize() {
	if (!m_scene_stack.empty()) {
		m_scene_stack.top()->Resize(m_d3d_manager);
	}
}

void SceneManager::Draw(ID3D12CommandList** command_lists) {
	if (!m_scene_stack.empty()) {
		m_scene_stack.top()->Draw(m_d3d_manager, command_lists);
	}
}

void SceneManager::Prcs_Input(UINT message, WPARAM wparam, LPARAM lparam) {
	if (!m_scene_stack.empty()) {
		m_scene_stack.top()->Prcs_Input(message, wparam, lparam);
	}
}

void SceneManager::Chg_Scene(std::unique_ptr<Scene> scene) {
	if (!m_scene_stack.empty()) {
		m_scene_stack.top()->Exit();
		m_scene_stack.pop();
	}

	scene->Set_SM(this);
	scene->Enter(m_d3d_manager);

	m_scene_stack.push(Add_Scene_2_Map(std::move(scene)));
}

void SceneManager::Chg_Scene(std::wstring scene_name, std::wstring back_scene_name) {
	if (!m_scene_stack.empty()) {
		m_scene_stack.top()->Exit();
		m_scene_stack.pop();
	}

	auto scene = Crt_Scene(scene_name);

	if (back_scene_name != L"") {
		scene->Set_BS(Get_Scene(back_scene_name));
	}

	scene->Set_SM(this);
	scene->Enter(m_d3d_manager);

	m_scene_stack.push(Add_Scene_2_Map(std::move(scene)));
}

void SceneManager::Push_Scene(std::unique_ptr<Scene> scene, bool pause) {
	if (!m_scene_stack.empty()) {
		if (pause) {
			m_scene_stack.top()->Pause();
		}
	}

	scene->Set_SM(this);
	scene->Enter(m_d3d_manager);

	m_scene_stack.push(Add_Scene_2_Map(std::move(scene)));
}

void SceneManager::Push_Scene(std::wstring scene_name, std::wstring back_scene_name, bool pause) {
	if (!m_scene_stack.empty()) {
		if (pause) {
			m_scene_stack.top()->Pause();
		}
	}

	auto scene = Crt_Scene(scene_name);

	if (back_scene_name != L"") {
		scene->Set_BS(Get_Scene(back_scene_name));
	}

	scene->Set_SM(this);
	scene->Enter(m_d3d_manager);

	m_scene_stack.push(Add_Scene_2_Map(std::move(scene)));
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

std::unique_ptr<Scene> SceneManager::Crt_Scene(std::wstring scene_name) {
	if (scene_name == L"Dummy") {
		auto result = std::make_unique<DummyScene>(scene_name);
		return result;
	}
	else if (scene_name == L"Test") {
		auto result = std::make_unique<TestScene>(scene_name);
		return result;
	}

	OutputDebugString(L"Scene Case Not Found\n");
	return nullptr;
}

Scene* SceneManager::Add_Scene_2_Map(std::unique_ptr<Scene> scene) {
	std::wstring scene_name = scene->Get_Name();

	if (!m_scene_map.count(scene_name)) {
		m_scene_map[scene_name] = std::move(scene);

		return m_scene_map[scene_name].get();
	}

	OutputDebugString(L"Scene Already In Map\n");
	return nullptr;
}

Scene* SceneManager::Get_Scene(std::wstring scene_name) {
	if (m_scene_map.count(scene_name)) {
		return m_scene_map[scene_name].get();
	}

	OutputDebugString(L"Scene Not Found\n");
	return nullptr;
}
