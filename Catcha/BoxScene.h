#pragma once
#include "common.h"
#include "Scene.h"

class BoxScene : public Scene {
public:
	BoxScene(std::wstring name, Scene* back_scene = nullptr) : Scene(name, back_scene) {}

	virtual void Enter(D3DManager* d3d_manager);
	virtual void Exit();

	virtual void Update(D3DManager* d3d_manager);
	virtual void Resize(D3DManager* d3d_manager);
	virtual void Draw(D3DManager* d3d_manager, ID3D12CommandList** command_lists);

	virtual void Prcs_Input(UINT message, WPARAM wparam, LPARAM lparam) {}	// Process Message

	virtual void Build_RS(ID3D12Device* device);	// Build Root Signature
	virtual void Build_S_N_L();	// Build Shaders And Layouts
	virtual void Build_M(ID3D12Device* device, ID3D12GraphicsCommandList* command_list);	// Build Meshs
	virtual void Build_O();	// Build Objects
	virtual void Build_FR(ID3D12Device* device);	// Build FrameResources
	virtual void Build_DH(ID3D12Device* device);	// Build Descriptor Heaps
	virtual void Build_CBV(D3DManager* d3d_manager);	// Build Constant Buffer Views
	virtual void Build_PSO(D3DManager* d3d_manager);	// Build Pipeline State Objects
};
