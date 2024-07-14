#pragma once
#include "common.h"
#include "FrameResource.h"

class SceneManager;
class D3DManager;

constexpr int FRAME_RESOURCES_NUMBER = 3;

class Scene {
protected:
	std::wstring m_name = L"name";
	bool m_paused = false;
	Scene* m_back_scene = nullptr;

	SceneManager* m_scene_manager = nullptr;

	//
	std::vector<std::unique_ptr<FrameResorce>> m_frameresources;
	FrameResorce* m_current_frameresource = nullptr;
	int m_current_frameresource_index = 0;

	Microsoft::WRL::ComPtr<ID3D12RootSignature> m_root_signature = nullptr;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_CBV_heap = nullptr;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_SRV_heap = nullptr;

	std::unordered_map<std::wstring, std::unique_ptr<MeshInfo>> m_mesh_map;
	std::unordered_map<std::wstring, Microsoft::WRL::ComPtr<ID3DBlob>> m_shader_map;
	std::unordered_map<std::wstring, Microsoft::WRL::ComPtr<ID3D12PipelineState>> m_pipeline_state_map;

	std::vector<D3D12_INPUT_ELEMENT_DESC> m_input_layouts;

	std::vector<std::unique_ptr<ObjectInfo>> m_objects;

	std::vector<ObjectInfo*> m_opaque_objects;
	std::vector<ObjectInfo*> m_transparent_objects;

	PassConstants m_main_pass_constant_buffer;

	UINT m_pass_CBV_offset = 0;

	bool m_wireframe = false;

	DirectX::XMFLOAT3 m_camera_position = { 0.0f, 0.0f, 0.0f };

	DirectX::XMFLOAT4X4 m_view_matrix = MathHelper::Identity_4x4();
	DirectX::XMFLOAT4X4 m_projection_matrix = MathHelper::Identity_4x4();

	float m_theta = 1.5f * DirectX::XM_PI;
	float m_phi = 0.2f * DirectX::XM_PI;
	float m_radius = 15.0f;

public:
	Scene(std::wstring name, Scene* back_scene = nullptr) : m_name(name), m_back_scene(back_scene) {}

	virtual void Enter(D3DManager* d3d_manager) {}
	virtual void Pause() { m_paused = true; }
	virtual void Resume() { m_paused = false; }
	virtual void Exit() {}

	virtual void Update(D3DManager* d3d_manager) {}
	virtual void Resize(D3DManager* d3d_manager) {}
	virtual void Draw(D3DManager* d3d_manager, ID3D12CommandList** command_lists) {}

	virtual void Prcs_Input(UINT message, WPARAM wparam, LPARAM lparam) {}	// Process Message

	virtual std::wstring Get_Name() { return m_name; }

	virtual void Build_RS(ID3D12Device* device) {}	// Build Root Signature
	virtual void Build_S_N_L() {}	// Build Shaders And Layouts
	virtual void Build_M(ID3D12Device* device, ID3D12GraphicsCommandList* command_list) {}	// Build Meshs
	virtual void Build_O() {}	// Build Objects
	virtual void Build_FR(ID3D12Device* device) {}	// Build FrameResources
	virtual void Build_DH(ID3D12Device* device) {}	// Build Descriptor Heaps
	virtual void Build_CBV(D3DManager* d3d_manager) {}	// Build Constant Buffer Views
	virtual void Build_PSO(D3DManager* d3d_manager) {}	// Build Pipeline State Objects

	virtual void Set_BS(Scene* back_scene) { m_back_scene = back_scene; }	// Set Back Scene
	virtual void Set_SM(SceneManager* scene_manager) { m_scene_manager = scene_manager; }	// Set Scene Manager
};

