#pragma once
#include "common.h"

class Shadowmap {
private:
	D3D12_VIEWPORT m_viewport;
	D3D12_RECT m_scissor_rect;

	UINT m_width = 0;
	UINT m_height = 0;
	DXGI_FORMAT m_format = DXGI_FORMAT_R24G8_TYPELESS;

	D3D12_CPU_DESCRIPTOR_HANDLE_EX m_srv_cpu_handle;
	D3D12_GPU_DESCRIPTOR_HANDLE_EX m_srv_gpu_handle;
	D3D12_CPU_DESCRIPTOR_HANDLE_EX m_dsv_cpu_handle;

	Microsoft::WRL::ComPtr<ID3D12Resource> m_shadowmap_resource = nullptr;

public:
	Shadowmap(ID3D12Device* device, UINT width, UINT height);
	~Shadowmap() {}

	D3D12_VIEWPORT Get_Viewport() { return m_viewport; }
	D3D12_RECT Get_Scissor_Rect() { return m_scissor_rect; }

	D3D12_GPU_DESCRIPTOR_HANDLE_EX Get_SRV_GPU_Handle() { return m_srv_gpu_handle; }
	D3D12_CPU_DESCRIPTOR_HANDLE_EX Get_DSV_CPU_Handle() { return m_dsv_cpu_handle; }

	UINT Get_Width() { return m_width; }
	UINT Get_Height() { return m_height; }

	ID3D12Resource* Get_Resource() { return m_shadowmap_resource.Get(); }

	void Build_Resource(ID3D12Device* device);

	void Build_Descriptors(ID3D12Device* device);
	void Build_Descriptors(ID3D12Device* device, D3D12_CPU_DESCRIPTOR_HANDLE_EX srv_cpu_handle,
		D3D12_GPU_DESCRIPTOR_HANDLE_EX srv_gpu_handle, D3D12_CPU_DESCRIPTOR_HANDLE_EX dsv_cpu_handle);

	void Resize(ID3D12Device* device, UINT new_width, UINT new_height);
};