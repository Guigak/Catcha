#pragma once
#include "Scene.h"

class DummyScene : public Scene {
public:
	DummyScene(std::wstring name, Scene* back_scene = nullptr);

	virtual void Enter();
	virtual void Pause();
	virtual void Resume();
	virtual void Exit();

	virtual void Update();
	virtual void Draw(D3DManager* d3d_manager);

	virtual void Prcs_Input(UINT message, WPARAM wparam, LPARAM lparam);	// Process Message
};

