#include "D3DManager.h"
#include "SceneManager.h"

bool D3DManager::Initialize(HWND hwnd, int width, int height) {
#if defined(DEBUG) || defined(_DEBUG)
	{
		Microsoft::WRL::ComPtr<ID3D12Debug> debug;
		Throw_If_Failed(D3D12GetDebugInterface(IID_PPV_ARGS(&debug)));
		debug->EnableDebugLayer();
	}
#endif

	Throw_If_Failed(CreateDXGIFactory1(IID_PPV_ARGS(&m_factory)));

	HRESULT hresult = D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_device));

	if (FAILED(hresult)) {
		Microsoft::WRL::ComPtr<IDXGIAdapter> warp_adapter;
		Throw_If_Failed(m_factory->EnumWarpAdapter(IID_PPV_ARGS(&warp_adapter)));

		Throw_If_Failed(D3D12CreateDevice(warp_adapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_device)));
	}

	Throw_If_Failed(m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)));

	m_RTV_descriptor_size = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	m_DSV_descriptor_size = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	m_CBV_SRV_UAV_descriptor_size = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS multisample_quality_levels;
	multisample_quality_levels.Format = m_back_buffer_format;
	multisample_quality_levels.SampleCount = 4;
	multisample_quality_levels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
	multisample_quality_levels.NumQualityLevels = 0;

	Throw_If_Failed(m_device->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &multisample_quality_levels, sizeof(multisample_quality_levels)));

	m_4xMSAA_quality = multisample_quality_levels.NumQualityLevels;
	assert(m_4xMSAA_quality > 0 && "Unexpected MSAA Quality Level");

#ifdef _DEBUG
	Log_Adapters();
#endif

	m_client_width = width;
	m_client_height = height;

	Crt_Cmd_Objs();
	Crt_SwapChain(hwnd, m_client_width, m_client_height);
	Crt_RTV_N_DSV_Descriptor_Heap();

	Resize();

	return true;
}

void D3DManager::Crt_Cmd_Objs() {
	D3D12_COMMAND_QUEUE_DESC queue_desc = {};
	queue_desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	queue_desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;

	Throw_If_Failed(m_device->CreateCommandQueue(&queue_desc, IID_PPV_ARGS(&m_command_queue)));

	Throw_If_Failed(m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(m_command_allocator.GetAddressOf())));

	Throw_If_Failed(m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_command_allocator.Get(), nullptr, IID_PPV_ARGS(m_command_list.GetAddressOf())));

	m_command_list->Close();
}

void D3DManager::Crt_SwapChain(HWND hwnd, int width, int height) {
	m_swapchain.Reset();

	DXGI_SWAP_CHAIN_DESC swapchain_desc;
	swapchain_desc.BufferDesc.Width = width;
	swapchain_desc.BufferDesc.Height = height;
	swapchain_desc.BufferDesc.RefreshRate.Numerator = 60;
	swapchain_desc.BufferDesc.RefreshRate.Denominator = 1;
	swapchain_desc.BufferDesc.Format = m_back_buffer_format;
	swapchain_desc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapchain_desc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	swapchain_desc.SampleDesc.Count = m_4xMSAA ? 4 : 1;
	swapchain_desc.SampleDesc.Quality = m_4xMSAA ? (m_4xMSAA_quality - 1) : 0;
	swapchain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapchain_desc.BufferCount = m_swapchain_buffer_count;
	swapchain_desc.OutputWindow = hwnd;
	swapchain_desc.Windowed = true;
	swapchain_desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapchain_desc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	Throw_If_Failed(m_factory->CreateSwapChain(m_command_queue.Get(), &swapchain_desc, m_swapchain.GetAddressOf()));
}

void D3DManager::Crt_RTV_N_DSV_Descriptor_Heap() {
	D3D12_DESCRIPTOR_HEAP_DESC RTV_heap_desc;
	RTV_heap_desc.NumDescriptors = m_swapchain_buffer_count;
	RTV_heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	RTV_heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	RTV_heap_desc.NodeMask = 0;

	Throw_If_Failed(m_device->CreateDescriptorHeap(&RTV_heap_desc, IID_PPV_ARGS(m_RTV_heap.GetAddressOf())));

	D3D12_DESCRIPTOR_HEAP_DESC DSV_heap_desc;
	DSV_heap_desc.NumDescriptors = 1;
	DSV_heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	DSV_heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	DSV_heap_desc.NodeMask = 0;

	Throw_If_Failed(m_device->CreateDescriptorHeap(&DSV_heap_desc, IID_PPV_ARGS(m_DSV_heap.GetAddressOf())));
}

void D3DManager::Resize() {
	assert(m_device);
	assert(m_swapchain);
	assert(m_command_allocator);

	Flush_Cmd_Q();

	Throw_If_Failed(m_command_list->Reset(m_command_allocator.Get(), nullptr));

	for (int i = 0; i < m_swapchain_buffer_count; ++i) {
		m_swapchain_buffer[i].Reset();
	}

	m_depth_stencil_buffer.Reset();

	Throw_If_Failed(m_swapchain->ResizeBuffers(m_swapchain_buffer_count, m_client_width, m_client_height, m_back_buffer_format, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH));

	m_current_back_buffer = 0;

	D3D12_CPU_DESCRIPTOR_HANDLE_EX RTV_descriptor_handle(m_RTV_heap->GetCPUDescriptorHandleForHeapStart());

	for (UINT i = 0; i < m_swapchain_buffer_count; ++i) {
		Throw_If_Failed(m_swapchain->GetBuffer(i, IID_PPV_ARGS(&m_swapchain_buffer[i])));
		m_device->CreateRenderTargetView(m_swapchain_buffer[i].Get(), nullptr, RTV_descriptor_handle);
		RTV_descriptor_handle.Offset(1, m_RTV_descriptor_size);
	}

	D3D12_RESOURCE_DESC depth_stencil_desc;
	depth_stencil_desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	depth_stencil_desc.Alignment = 0;
	depth_stencil_desc.Width = m_client_width;
	depth_stencil_desc.Height = m_client_height;
	depth_stencil_desc.DepthOrArraySize = 1;
	depth_stencil_desc.MipLevels = 1;
	depth_stencil_desc.Format = DXGI_FORMAT_R24G8_TYPELESS;
	depth_stencil_desc.SampleDesc.Count = m_4xMSAA ? 4 : 1;
	depth_stencil_desc.SampleDesc.Quality = m_4xMSAA ? (m_4xMSAA_quality - 1) : 0;
	depth_stencil_desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	depth_stencil_desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_CLEAR_VALUE clear_option;
	clear_option.Format = m_depth_stencil_format;
	clear_option.DepthStencil.Depth = 1.0f;
	clear_option.DepthStencil.Stencil = 0;

	Throw_If_Failed(m_device->CreateCommittedResource(&D3D12_HEAP_PROPERTIES_EX(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE,
		&depth_stencil_desc, D3D12_RESOURCE_STATE_COMMON, &clear_option, IID_PPV_ARGS(m_depth_stencil_buffer.GetAddressOf())));

	D3D12_DEPTH_STENCIL_VIEW_DESC DSV_desc;
	DSV_desc.Flags = D3D12_DSV_FLAG_NONE;
	DSV_desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	DSV_desc.Format = m_depth_stencil_format;
	DSV_desc.Texture2D.MipSlice = 0;

	m_device->CreateDepthStencilView(m_depth_stencil_buffer.Get(), &DSV_desc, Get_DSV());

	m_command_list->ResourceBarrier(1, &D3D12_RESOURCE_BARRIER_HELPER::Transition(m_depth_stencil_buffer.Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_DEPTH_WRITE));

	Throw_If_Failed(m_command_list->Close());

	ID3D12CommandList* command_lists[] = { m_command_list.Get() };
	m_command_queue->ExecuteCommandLists(_countof(command_lists), command_lists);

	Flush_Cmd_Q();

	m_viewport.TopLeftX = 0;
	m_viewport.TopLeftY = 0;
	m_viewport.Width = (float)m_client_width;
	m_viewport.Height = (float)m_client_height;
	m_viewport.MinDepth = 0.0f;
	m_viewport.MaxDepth = 1.0f;

	m_scissor_rect = { 0, 0, m_client_width, m_client_height };
}

void D3DManager::Draw() {
	Throw_If_Failed(m_command_allocator->Reset());

	Throw_If_Failed(m_command_list->Reset(m_command_allocator.Get(), nullptr));

	m_command_list->ResourceBarrier(1, &D3D12_RESOURCE_BARRIER_HELPER::Transition(Get_Curr_BB(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

	m_command_list->RSSetViewports(1, &m_viewport);
	m_command_list->RSSetScissorRects(1, &m_scissor_rect);

	Clr_RTV();
	Clr_DSV();

	m_command_list->OMSetRenderTargets(1, &Get_Curr_BBV(), true, &Get_DSV());

	m_command_list->ResourceBarrier(1, &D3D12_RESOURCE_BARRIER_HELPER::Transition(Get_Curr_BB(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

	Throw_If_Failed(m_command_list->Close());

	ID3D12CommandList* command_lists[] = { m_command_list.Get() };
	m_command_queue->ExecuteCommandLists(_countof(command_lists), command_lists);

	Throw_If_Failed(m_swapchain->Present(0, 0));
	m_current_back_buffer = (m_current_back_buffer + 1) % m_swapchain_buffer_count;

	Flush_Cmd_Q();
}

void D3DManager::Draw_Scene() {
	Prepare_Render();
	Render_Scene();
	Present_Scene();
}

void D3DManager::Prepare_Render() {
	Throw_If_Failed(m_command_allocator->Reset());

	Throw_If_Failed(m_command_list->Reset(m_command_allocator.Get(), nullptr));

	m_command_list->ResourceBarrier(1, &D3D12_RESOURCE_BARRIER_HELPER::Transition(Get_Curr_BB(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

	m_command_list->RSSetViewports(1, &m_viewport);
	m_command_list->RSSetScissorRects(1, &m_scissor_rect);

	m_command_list->OMSetRenderTargets(1, &Get_Curr_BBV(), true, &Get_DSV());
}

void D3DManager::Render_Scene() {
	if (m_scene_manager) {
		m_scene_manager->Draw();
	}
	else {
		OutputDebugString(L"SceneManager is null\n");
	}
}

void D3DManager::Present_Scene() {
	m_command_list->ResourceBarrier(1, &D3D12_RESOURCE_BARRIER_HELPER::Transition(Get_Curr_BB(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

	Throw_If_Failed(m_command_list->Close());

	ID3D12CommandList* command_lists[] = { m_command_list.Get() };
	m_command_queue->ExecuteCommandLists(_countof(command_lists), command_lists);

	Throw_If_Failed(m_swapchain->Present(0, 0));
	m_current_back_buffer = (m_current_back_buffer + 1) % m_swapchain_buffer_count;

	Flush_Cmd_Q();
}

void D3DManager::Clr_RTV() {
	m_command_list->ClearRenderTargetView(Get_Curr_BBV(), DirectX::Colors::LightSteelBlue, 0, nullptr);
}

void D3DManager::Clr_DSV() {
	m_command_list->ClearDepthStencilView(Get_DSV(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
}

void D3DManager::Flush_Cmd_Q() {
	++m_current_fence;

	Throw_If_Failed(m_command_queue->Signal(m_fence.Get(), m_current_fence));

	if (m_fence->GetCompletedValue() < m_current_fence) {
		HANDLE handle = CreateEventEx(nullptr, false, false, EVENT_ALL_ACCESS);

		Throw_If_Failed(m_fence->SetEventOnCompletion(m_current_fence, handle));

		WaitForSingleObject(handle, INFINITE);
		CloseHandle(handle);
	}
}

void D3DManager::Log_Adapters() {
	UINT i = 0;
	IDXGIAdapter* adapter = nullptr;
	std::vector<IDXGIAdapter*> adapters;

	while (m_factory->EnumAdapters(i, &adapter) != DXGI_ERROR_NOT_FOUND) {
		DXGI_ADAPTER_DESC adapter_desc;
		adapter->GetDesc(&adapter_desc);

		std::wstring wstr = L"Adapter : ";
		wstr += adapter_desc.Description;
		wstr += L"\n";

		OutputDebugString(wstr.c_str());

		adapters.emplace_back(adapter);

		++i;
	}

	for (size_t j = 0; j < adapters.size(); ++j) {
		Log_Outputs(adapters[j]);
		Release_Com(adapters[j]);
	}
}

void D3DManager::Log_Outputs(IDXGIAdapter* adapter) {
	UINT i = 0;
	IDXGIOutput* output = nullptr;
	
	while (adapter->EnumOutputs(i, &output) != DXGI_ERROR_NOT_FOUND) {
		DXGI_OUTPUT_DESC output_desc;
		output->GetDesc(&output_desc);

		std::wstring wstr = L"Output : ";
		wstr += output_desc.DeviceName;
		wstr += L"\n";

		OutputDebugString(wstr.c_str());

		Log_Display_Modes(output, m_back_buffer_format);

		Release_Com(output);

		++i;
	}
}

void D3DManager::Log_Display_Modes(IDXGIOutput* output, DXGI_FORMAT format) {
	UINT count = 0;
	UINT flags = 0;

	output->GetDisplayModeList(format, flags, &count, nullptr);

	if (count == 0) {
		OutputDebugString(L"No Available Display Modes\n");

		return;
	}

	std::vector<DXGI_MODE_DESC> modes_desc(count);
	output->GetDisplayModeList(format, flags, &count, &modes_desc[0]);

	for (auto& desc : modes_desc) {
		std::wstring wstr =
			L"Width : " + std::to_wstring(desc.Width) + L" " +
			L"Height : " + std::to_wstring(desc.Height) + L" " +
			L"RefreshRate : " + std::to_wstring(desc.RefreshRate.Numerator) + L" / " + std::to_wstring(desc.RefreshRate.Denominator) + L"\n";

		OutputDebugString(wstr.c_str());
	}
}