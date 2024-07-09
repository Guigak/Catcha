#pragma once
#include "common.h"

class SceneManager;

class D3DManager {
private:
	bool m_paused = false;
	bool m_minimized = false;
	bool m_maxmized = false;
	bool m_resizing = false;
	bool m_fullscreen = false;

	bool m_4xMSAA = false;
	UINT m_4xMSAA_quality = 0;

	Microsoft::WRL::ComPtr<IDXGIFactory4> m_factory;
	Microsoft::WRL::ComPtr<IDXGISwapChain> m_swapchain;
	Microsoft::WRL::ComPtr<ID3D12Device> m_device;

	Microsoft::WRL::ComPtr<ID3D12Fence> m_fence;
	UINT64 m_current_fence = 0;

	Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_command_queue;
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_command_allocator;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_command_list;

	static const int m_swapchain_buffer_count = 2;
	int m_current_back_buffer = 0;
	Microsoft::WRL::ComPtr<ID3D12Resource> m_swapchain_buffer[m_swapchain_buffer_count];
	Microsoft::WRL::ComPtr<ID3D12Resource> m_depth_stencil_buffer;

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_RTV_heap;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_DSV_heap;

	D3D12_VIEWPORT m_viewport;
	D3D12_RECT m_scissor_rect;

	UINT m_RTV_descriptor_size = 0;
	UINT m_DSV_descriptor_size = 0;
	UINT m_CBV_SRV_UAV_descriptor_size = 0;

	DXGI_FORMAT m_back_buffer_format = DXGI_FORMAT_R8G8B8A8_UNORM;
	DXGI_FORMAT m_depth_stencil_format = DXGI_FORMAT_D24_UNORM_S8_UINT;

	int m_client_width;
	int m_client_height;

	//
	SceneManager* m_scene_manager = nullptr;

public:
	D3DManager() {}
	~D3DManager() {}

	bool Initialize(HWND hwnd, int width, int height);

	void Crt_Cmd_Objs();	// Create Command Objects
	void Crt_SwapChain(HWND hwnd, int width, int height);	// Create SwapChain
	void Crt_RTV_N_DSV_Descriptor_Heap();	// Create RTV and DSV Descriptor Heap

	void Resize();
	void Draw();

	void Draw_Scene();
	void Prepare_Render();
	void Render_Scene();
	void Present_Scene();

	void Clr_RTV();	// Clear Render Target View
	void Clr_DSV();	// Clear Depth Stencil View

	void Flush_Cmd_Q();

	ID3D12Resource* Get_Curr_BB() { return m_swapchain_buffer[m_current_back_buffer].Get(); }	// Ger Current Back Buffer
	D3D12_CPU_DESCRIPTOR_HANDLE Get_Curr_BBV() { return D3D12_CPU_DESCRIPTOR_HANDLE_EX(m_RTV_heap->GetCPUDescriptorHandleForHeapStart(), m_current_back_buffer, m_RTV_descriptor_size); }	// Get Current Back Buffer View
	D3D12_CPU_DESCRIPTOR_HANDLE Get_DSV() { return m_DSV_heap->GetCPUDescriptorHandleForHeapStart(); }	// Get Depth Stencil View

	void Log_Adapters();
	void Log_Outputs(IDXGIAdapter* adapter);
	void Log_Display_Modes(IDXGIOutput* output, DXGI_FORMAT format);

	//
	void Set_SM(SceneManager* scene_manager) { m_scene_manager = scene_manager; }	// Set SceneManager
};

