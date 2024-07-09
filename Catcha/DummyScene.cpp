#include "DummyScene.h"
#include "D3DManager.h"

DummyScene::DummyScene(std::wstring name, Scene* back_scene) : Scene(name, back_scene) {
}

void DummyScene::Enter() {
	std::wstring wstr = m_name + L" Enter\n";
	OutputDebugString(wstr.c_str());
}

void DummyScene::Pause() {
	Scene::Pause();

	std::wstring wstr = m_name + L" Pause\n";
	OutputDebugString(wstr.c_str());
}

void DummyScene::Resume() {
	Scene::Resume();

	std::wstring wstr = m_name + L" Resume\n";
	OutputDebugString(wstr.c_str());
}

void DummyScene::Exit() {
	std::wstring wstr = m_name + L" Exit\n";
	OutputDebugString(wstr.c_str());
}

void DummyScene::Update() {
	std::wstring wstr = m_name + L" Update\n";
	OutputDebugString(wstr.c_str());
}

void DummyScene::Draw(D3DManager* d3d_manager) {
	d3d_manager->Clr_RTV();
	d3d_manager->Clr_RTV();

	std::wstring wstr = m_name + L" Draw\n";
	OutputDebugString(wstr.c_str());
}

void DummyScene::Prcs_Input(UINT message, WPARAM wparam, LPARAM lparam) {
	std::wstring wstr = m_name + L" Process Input\n";
	OutputDebugString(wstr.c_str());
}
