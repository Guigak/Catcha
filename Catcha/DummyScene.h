#pragma once
#include "Scene.h"

class DummyScene : public Scene {
public:
	DummyScene(std::wstring name, Scene* back_scene = nullptr);

	virtual void Enter(D3DManager* d3d_manager);
	virtual void Pause();
	virtual void Resume();
	virtual void Exit(D3DManager* d3d_manager);

	virtual void Update(D3DManager* d3d_manager);
	virtual void Draw(D3DManager* d3d_manager, ID3D12CommandList** command_lists);

	virtual void Prcs_Input(UINT message, WPARAM wparam, LPARAM lparam);	// Process Message

	void CharacterChange(bool is_cat, const std::wstring& key1, const std::wstring& key2) {};
	// 접속시 카메라 값 초기화
	void InitCamera(DirectX::XMFLOAT4 rotate_quat = DirectX::XMFLOAT4{ 0.0f, 0.0f, 0.0f, 1.0f }) {};

	void OpenDoorEvent() {};
};

