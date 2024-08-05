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
};

