#include "Shadowmap.h"

Shadowmap::Shadowmap(ID3D12Device* device, UINT width, UINT height) {
	m_width = width;
	m_height = height;

	m_viewport = { 0.0f, 0.0f, (float)m_width, (float)m_height, 0.0f, 1.0f };
	m_scissor_rect = { 0, 0, (int)m_width, (int)m_height };

	Build_Resource(device);
}

void Shadowmap::Build_Resource(ID3D12Device* device) {
	D3D12_RESOURCE_DESC resource_desc;
	ZeroMemory(&resource_desc, sizeof(D3D12_RESOURCE_DESC));
	resource_desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	resource_desc.Alignment = 0;
	resource_desc.Width = m_width;
	resource_desc.Height = m_height;
	resource_desc.DepthOrArraySize = 1;
	resource_desc.MipLevels = 1;
	resource_desc.Format = m_format;
	resource_desc.SampleDesc.Count = 1;
	resource_desc.SampleDesc.Quality = 0;
	resource_desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	resource_desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_CLEAR_VALUE clear_value;
	clear_value.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	clear_value.DepthStencil.Depth = 1.0f;
	clear_value.DepthStencil.Stencil = 0;

	Throw_If_Failed(device->CreateCommittedResource(
		&D3D12_HEAP_PROPERTIES_EX(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&resource_desc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		&clear_value,
		IID_PPV_ARGS(&m_shadowmap_resource)));
}

void Shadowmap::Build_Descriptors(ID3D12Device* device) {
	D3D12_SHADER_RESOURCE_VIEW_DESC srv_desc = {};
	srv_desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srv_desc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	srv_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srv_desc.Texture2D.MostDetailedMip = 0;
	srv_desc.Texture2D.MipLevels = 1;
	srv_desc.Texture2D.ResourceMinLODClamp = 0.0f;
	srv_desc.Texture2D.PlaneSlice = 0;
	device->CreateShaderResourceView(m_shadowmap_resource.Get(), &srv_desc, m_srv_cpu_handle);

	D3D12_DEPTH_STENCIL_VIEW_DESC dsv_desc;
	dsv_desc.Flags = D3D12_DSV_FLAG_NONE;
	dsv_desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dsv_desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dsv_desc.Texture2D.MipSlice = 0;
	device->CreateDepthStencilView(m_shadowmap_resource.Get(), &dsv_desc, m_dsv_cpu_handle);

}

void Shadowmap::Build_Descriptors(ID3D12Device* device, D3D12_CPU_DESCRIPTOR_HANDLE_EX srv_cpu_handle,
	D3D12_GPU_DESCRIPTOR_HANDLE_EX srv_gpu_handle, D3D12_CPU_DESCRIPTOR_HANDLE_EX dsv_cpu_handle
) {
	m_srv_cpu_handle = srv_cpu_handle;
	m_srv_gpu_handle = srv_gpu_handle;
	m_dsv_cpu_handle = dsv_cpu_handle;

	Build_Descriptors(device);
}

void Shadowmap::Resize(ID3D12Device* device, UINT new_width, UINT new_height) {
	if (m_width != new_width || m_height != new_height) {
		m_width = new_width;
		m_height = new_height;

		Build_Resource(device);
		Build_Descriptors(device);
	}
}
