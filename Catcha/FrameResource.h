#pragma once
#include "common.h"
#include "UploadBuffer.h"

struct ObjectConstants {
	DirectX::XMFLOAT4X4 world_matrix = MathHelper::Identity_4x4();
	DirectX::XMFLOAT4 color_multiplier = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	UINT animated = 0;
};

struct MaterialConstants {
	std::array<Material_Factor, MAX_MATERIAL_COUNT> material_array;
};

struct PassConstants {
	DirectX::XMFLOAT4X4 view_matrix = MathHelper::Identity_4x4();
	DirectX::XMFLOAT4X4 inverse_view_matrix = MathHelper::Identity_4x4();
	DirectX::XMFLOAT4X4 projection_matrix = MathHelper::Identity_4x4();
	DirectX::XMFLOAT4X4 inverse_projection_matrix = MathHelper::Identity_4x4();
	DirectX::XMFLOAT4X4 view_projection_matrix = MathHelper::Identity_4x4();
	DirectX::XMFLOAT4X4 inverse_view_projection_matrix = MathHelper::Identity_4x4();
	DirectX::XMFLOAT4X4 shadow_transform_matrix = MathHelper::Identity_4x4();

	DirectX::XMFLOAT3 camera_position = { 0.0f, 0.0f, 0.0f };
	float buffer_padding = 0.0f;

	DirectX::XMFLOAT2 render_target_size = { 0.0f, 0.0f };
	DirectX::XMFLOAT2 inverse_render_target_size = { 0.0f, 0.0f };

	float near_z = 0.0f;
	float far_z = 0.0f;

	float total_time = 0.0f;
	float delta_time = 0.0f;

	DirectX::XMFLOAT4 ambient_light = { 0.0f, 0.0f, 0.0f, 1.0f };

	LightInfo lights[MAX_LIGHTS];
};

struct AnimationConstants {
	std::array<DirectX::XMFLOAT4X4, MAX_BONE_COUNT> animation_transform_matrix;
};

struct Vertex {
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT3 normal;
};

struct FrameResorce {
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> command_allocator;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> command_list;

	std::unique_ptr<UploadBuffer<ObjectConstants>> object_constant_buffer = nullptr;
	std::unique_ptr<UploadBuffer<MaterialConstants>> material_constant_buffer = nullptr;
	std::unique_ptr<UploadBuffer<PassConstants>> pass_constant_buffer = nullptr;
	std::unique_ptr<UploadBuffer<AnimationConstants>> animation_constant_buffer = nullptr;

	UINT64 fence = 0;

	FrameResorce(ID3D12Device* device, UINT pass_count, UINT object_count, UINT material_count) {
		Throw_If_Failed(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(command_allocator.GetAddressOf())));

		Throw_If_Failed(device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, command_allocator.Get(), nullptr, IID_PPV_ARGS(command_list.GetAddressOf())));

		command_list->Close();

		object_constant_buffer = std::make_unique<UploadBuffer<ObjectConstants>>(device, object_count, true);
		material_constant_buffer = std::make_unique<UploadBuffer<MaterialConstants>>(device, material_count, true);
		pass_constant_buffer = std::make_unique<UploadBuffer<PassConstants>>(device, pass_count, true);
		animation_constant_buffer = std::make_unique<UploadBuffer<AnimationConstants>>(device, object_count, true);
	}
};