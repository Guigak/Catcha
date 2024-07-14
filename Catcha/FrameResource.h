#pragma once
#include "common.h"
#include "UploadBuffer.h"

struct ObjectConstants {
	DirectX::XMFLOAT4X4 world_matrix = MathHelper::Identity_4x4();
};

struct PassConstants {
	DirectX::XMFLOAT4X4 view_matrix = MathHelper::Identity_4x4();
	DirectX::XMFLOAT4X4 inverse_view_matrix = MathHelper::Identity_4x4();
	DirectX::XMFLOAT4X4 projection_matrix = MathHelper::Identity_4x4();
	DirectX::XMFLOAT4X4 inverse_projection_matrix = MathHelper::Identity_4x4();
	DirectX::XMFLOAT4X4 view_projection_matrix = MathHelper::Identity_4x4();
	DirectX::XMFLOAT4X4 inverse_view_projection_matrix = MathHelper::Identity_4x4();

	DirectX::XMFLOAT3 camera_position = { 0.0f, 0.0f, 0.0f };
	float buffer_padding = 0.0f;

	DirectX::XMFLOAT2 render_target_size = { 0.0f, 0.0f };
	DirectX::XMFLOAT2 inverse_render_target_size = { 0.0f, 0.0f };

	float near_z = 0.0f;
	float far_z = 0.0f;

	float total_time = 0.0f;
	float delta_time = 0.0f;
};

struct Vertex {
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT4 color;
};

struct FrameResorce {
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> command_allocator;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> command_list;

	std::unique_ptr<UploadBuffer<ObjectConstants>> object_constant_buffer = nullptr;
	std::unique_ptr<UploadBuffer<PassConstants>> pass_constant_buffer = nullptr;

	UINT64 fence = 0;

	FrameResorce(ID3D12Device* device, UINT pass_count, UINT object_count) {
		Throw_If_Failed(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(command_allocator.GetAddressOf())));

		Throw_If_Failed(device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, command_allocator.Get(), nullptr, IID_PPV_ARGS(command_list.GetAddressOf())));

		command_list->Close();

		object_constant_buffer = std::make_unique<UploadBuffer<ObjectConstants>>(device, object_count, true);
		pass_constant_buffer = std::make_unique<UploadBuffer<PassConstants>>(device, pass_count, true);
	}
};