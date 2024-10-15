#pragma once
#include "common.h"

class AnimationManager {
private:
	int m_animation_count = 0;

	std::unordered_map<std::wstring, std::unique_ptr<Animation_Info>> m_animation_map;

public:
	AnimationManager() {}
	~AnimationManager() {}

	Animation_Info* Add_Animation(std::wstring animation_name, UINT bone_count, float animation_time, std::map<float, Keyframe_Info>& keyframe_map);

	Animation_Info* Get_Animation(std::wstring animation_name);

	void Get_Animated_Matrix(std::wstring animation_name, float animation_time,
		std::array<DirectX::XMFLOAT4X4, MAX_BONE_COUNT>& animation_matrix_array);
};

