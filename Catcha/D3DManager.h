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
	std::vector<Microsoft::WRL::ComPtr<ID3D12CommandAllocator>> m_command_allocators;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_command_list;

	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_additional_command_allocator;
	std::vector<Microsoft::WRL::ComPtr<ID3D12CommandAllocator>> m_additional_command_allocators;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_additional_command_list;

	int m_current_frameresource_index = 0;

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

	void Draw_Scene_With_FR();	// Draw Scene With FrameResource

	void Set_VP(ID3D12GraphicsCommandList* command_list) { command_list->RSSetViewports(1, &m_viewport); }	// Set Viewport
	void Set_SR(ID3D12GraphicsCommandList* command_list) { command_list->RSSetScissorRects(1, &m_scissor_rect); }	// Set Scissor Rect
	void Set_RTV_N_DSV(ID3D12GraphicsCommandList* command_list) { command_list->OMSetRenderTargets(1, &Get_Curr_BBV(), true, &Get_DSV()); }	// Set Render Target View And Depth Stencil View
	
	void BB_RB_Transition(D3D12_RESOURCE_STATES before_state, D3D12_RESOURCE_STATES after_state);	// Back Buffer Resource Barrier Transition

	void Clr_RTV(ID3D12GraphicsCommandList* command_list = nullptr);	// Clear Render Target View
	void Clr_DSV(ID3D12GraphicsCommandList* command_list = nullptr);	// Clear Depth Stencil View

	void Rst_Cmd_List() { Throw_If_Failed(m_command_list->Reset(m_command_allocator.Get(), nullptr)); }
	void Cls_Cmd_List() { Throw_If_Failed(m_command_list->Close()); }
	void Exct_Cmd_List() {
		ID3D12CommandList* command_lists[] = { m_command_list.Get() };
		m_command_queue->ExecuteCommandLists(_countof(command_lists), command_lists);
	}

	void Flush_Cmd_Q();

	ID3D12Resource* Get_Curr_BB() { return m_swapchain_buffer[m_current_back_buffer].Get(); }	// Ger Current Back Buffer
	D3D12_CPU_DESCRIPTOR_HANDLE Get_Curr_BBV() { return D3D12_CPU_DESCRIPTOR_HANDLE_EX(m_RTV_heap->GetCPUDescriptorHandleForHeapStart(), m_current_back_buffer, m_RTV_descriptor_size); }	// Get Current Back Buffer View
	D3D12_CPU_DESCRIPTOR_HANDLE Get_DSV() { return m_DSV_heap->GetCPUDescriptorHandleForHeapStart(); }	// Get Depth Stencil View

	ID3D12Device* Get_Device() { return m_device.Get(); }
	ID3D12GraphicsCommandList* Get_Cmd_List() { return m_command_list.Get(); }
	ID3D12Fence* Get_Fence() { return m_fence.Get(); }

	UINT64 Get_Curr_Fence() { return m_current_fence; }	// Get Current Fence

	UINT Get_RTV_Descritpor_Size() { return m_RTV_descriptor_size; }
	UINT Get_DSV_Descritpor_Size() { return m_DSV_descriptor_size; }
	UINT Get_CBV_SRV_UAV_Descritpor_Size() { return m_CBV_SRV_UAV_descriptor_size; }

	DXGI_FORMAT Get_BB_Format() { return m_back_buffer_format; }	// Get Back Buffer Format
	DXGI_FORMAT Get_DS_Format() { return m_depth_stencil_format; }	// Get Depth Stencil Format

	bool Is_4xMSAA() { return m_4xMSAA; }
	UINT Get_4xMSAA_Qual() { return m_4xMSAA_quality; }	// Get 4x MSAA Quality

	int Get_Client_Width() { return m_client_width; }
	int Get_Client_Height() { return m_client_height; }
	float Get_Aspect_Ratio() { return (float)m_client_width / (float)m_client_height; }

	void Log_Adapters();
	void Log_Outputs(IDXGIAdapter* adapter);
	void Log_Display_Modes(IDXGIOutput* output, DXGI_FORMAT format);

	//
	void Set_SM(SceneManager* scene_manager) { m_scene_manager = scene_manager; }	// Set SceneManager
};

