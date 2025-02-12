#pragma once
#include "common.h"
#include "FrameResource.h"
#include "Camera.h"
#include "ObjectManager.h"
#include "InputManager.h"
#include "Shadowmap.h"

class SceneManager;
class D3DManager;

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

	std::unordered_map<std::wstring, Microsoft::WRL::ComPtr<ID3DBlob>> m_shader_map;
	std::unordered_map<std::wstring, Microsoft::WRL::ComPtr<ID3D12PipelineState>> m_pipeline_state_map;

	std::vector<D3D12_INPUT_ELEMENT_DESC> m_input_layouts;

	std::unique_ptr<ObjectManager> m_object_manager;

	std::unique_ptr<InputManager> m_input_manager;

	PassConstants m_main_pass_constant_buffer;

	UINT m_material_CBV_offset = 0;
	UINT m_pass_CBV_offset = 0;
	//
	UINT m_animation_CBV_offset = 0;

	UINT m_shadow_map_SRV_offset = 0;

	//
	UINT m_texture_SRV_offset = 0;

	//
	UINT m_instance_SRV_offset = 0;

	bool m_wireframe = false;
	bool m_render_boundingbox = false;

	//std::unordered_map<std::wstring, std::unique_ptr<Camera>> m_camera_map;

	Camera* m_main_camera = nullptr;

	DirectX::XMFLOAT3 m_camera_position = { 0.0f, 0.0f, 0.0f };

	DirectX::XMFLOAT4X4 m_view_matrix = MathHelper::Identity_4x4();
	DirectX::XMFLOAT4X4 m_projection_matrix = MathHelper::Identity_4x4();

	float m_theta = 1.5f * DirectX::XM_PI;
	float m_phi = 0.2f * DirectX::XM_PI;
	float m_radius = 15.0f;

	//
	DirectX::BoundingSphere m_shadow_bounding_sphere;
	std::unique_ptr<Shadowmap> m_shadow_map;

	PassConstants m_shadow_pass_constant_buffer;

	float m_light_near_z = 0.0f;
	float m_light_far_z = 0.0f;
	DirectX::XMFLOAT3 m_light_position = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
	DirectX::XMFLOAT4X4 m_light_view_matrix = MathHelper::Identity_4x4();
	DirectX::XMFLOAT4X4 m_light_projection_matrix = MathHelper::Identity_4x4();
	DirectX::XMFLOAT4X4 m_shadow_transform_matrix = MathHelper::Identity_4x4();

	//
	std::unordered_map<std::wstring, std::unique_ptr<Texture_Info>> m_texture_map;

	//
	float m_total_time = 0.0f;

	//
	int m_client_width;
	int m_client_height;

public:
	Scene(std::wstring name, Scene* back_scene = nullptr) : m_name(name), m_back_scene(back_scene) {}

	virtual void Enter(D3DManager* d3d_manager) {}
	virtual void Pause() { m_paused = true; }
	virtual void Resume() { m_paused = false; }
	virtual void Exit(D3DManager* d3d_manager) {}

	virtual void Update(D3DManager* d3d_manager, float elapsed_time) {}
	virtual void Resize(D3DManager* d3d_manager) { Flush_Cmd_Q(d3d_manager); }
	virtual void Draw(D3DManager* d3d_manager, ID3D12CommandList** command_lists) {}

	virtual void Prcs_Input_Msg(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam) {}	// Process Message

	virtual std::wstring Get_Name() { return m_name; }

	virtual void Load_Texture(ID3D12Device* device, ID3D12GraphicsCommandList* command_list) {}

	virtual void Build_RS(ID3D12Device* device) {}	// Build Root Signature
	virtual void Build_S_N_L() {}	// Build Shaders And Layouts
	virtual void Build_Mesh(ID3D12Device* device, ID3D12GraphicsCommandList* command_list) {}	// Build Meshs
	virtual void Build_Material() {}	// Build Materials
	virtual void Build_O() {}	// Build Objects
	virtual void Build_C(D3DManager* d3d_manager) {}	// Build Cameras
	virtual void Build_FR(ID3D12Device* device) {}	// Build FrameResources
	virtual void Build_DH(ID3D12Device* device) {}	// Build Descriptor Heaps
	virtual void Build_CBV(D3DManager* d3d_manager) {}	// Build Constant Buffer Views
	virtual void Build_PSO(D3DManager* d3d_manager) {}	// Build Pipeline State Objects

	virtual void Binding_Key() {}
	virtual void Pairing_Collision_Set() {}

	virtual void Set_BS(Scene* back_scene) { m_back_scene = back_scene; }	// Set Back Scene
	virtual void Set_SM(SceneManager* scene_manager) { m_scene_manager = scene_manager; }	// Set Scene Manager

	virtual void Custom_Function_One() {}
	virtual void Custom_Function_Two() {}
	virtual void Custom_Function_Three() {}

	//
	void Chg_Wireframe_Flag() { m_wireframe = !m_wireframe; }
	void Chg_Boundingbox_Flag() { m_render_boundingbox = !m_render_boundingbox; }

	void Flush_Cmd_Q(D3DManager* d3d_manager);

	//
	float Get_Total_Time() { return m_total_time; }

	//
	virtual void Picking(POINTF screen_position) {}
};

