#pragma once
#include "common.h"

//struct Mesh {
//	Mesh_Info* mesh_info = nullptr;
//	DirectX::XMFLOAT4X4 local_transform_matrix = MathHelper::Identity_4x4();
//};

//struct Material_Info {
//	std::wstring name;
//
//	UINT constant_buffer_index = -1;
//
//	UINT diffuse_heap_index = -1;
//	UINT normal_heap_index = -1;
//
//	int dirty_frame_count = FRAME_RESOURCES_NUMBER;
//
//	DirectX::XMFLOAT4 diffuse_albedo = { 1.0f, 1.0f, 1.0f, 1.0f };
//	DirectX::XMFLOAT3 fresnel = { 0.01f, 0.01f, 0.01f };
//	float roughness = 0.25f;
//};
//
//struct BoneInfo {
//	std::wstring name;
//};
//
//struct SkeletonInfo {
//	std::unique_ptr<BoneInfo> root_bone;
//};
//
//struct KeyframeInfo {
//	float time;
//};
//
//struct AnimationInfo {
//	std::wstring name;
//};