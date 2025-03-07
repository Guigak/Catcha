#include "AnimationManager.h"

Animation_Info* AnimationManager::Add_Animation(std::wstring animation_name, UINT bone_count, float animation_time, std::map<float, Keyframe_Info>& keyframe_map) {
	std::unique_ptr<Animation_Info> animation_info = std::make_unique<Animation_Info>(animation_name, bone_count, animation_time, keyframe_map);

	m_animation_map[animation_name] = std::move(animation_info);
	m_animation_count++;

	return m_animation_map[animation_name].get();
}

Animation_Info* AnimationManager::Get_Animation(std::wstring animation_name) {
	return m_animation_map[animation_name].get();
}

void AnimationManager::Get_Animated_Transform(std::wstring animation_name, float multiply_time, float animated_time, bool loop,
	std::array<Transform_Info, MAX_BONE_COUNT>& transform_info_array
) {
	Animation_Info* animation_info = m_animation_map[animation_name].get();

	animated_time = animated_time * multiply_time;

	if (loop) {
		animated_time = std::fmod(animated_time, animation_info->animation_time);
	}

	float keyframe_time = animation_info->Get_Upper_Keyframe_Time(animated_time);
	transform_info_array = animation_info->keyframe_map[keyframe_time].animation_transform_array;

	//for (UINT i = 0; i < animation_info->bone_count; ++i) {
	//	DirectX::XMMATRIX translate_matrix = MathHelper::XMMATRIX_Translation(transform_info_array[i].translate);
	//	DirectX::XMMATRIX rotate_matrix = MathHelper::XMMATRIX_Rotation(transform_info_array[i].rotate);
	//	DirectX::XMMATRIX scale_matrix = MathHelper::XMMATRIX_Scaling(transform_info_array[i].scale);

	//	DirectX::XMMATRIX animation_matrix = scale_matrix * rotate_matrix * translate_matrix;
	//	//DirectX::XMStoreFloat4x4(&animation_matrix_array[i], animation_matrix);
	//	DirectX::XMStoreFloat4x4(&animation_matrix_array[i], DirectX::XMMatrixTranspose(animation_matrix));
	//}
}
