#include "AnimationManager.h"

Animation_Info* AnimationManager::Add_Animation(std::wstring animation_name, float animation_time, std::map<float, Ketframe_Info> keyframe_map) {
	std::unique_ptr<Animation_Info> animation_info = std::make_unique<Animation_Info>(animation_name, animation_time, keyframe_map);

	m_animation_map[animation_name] = std::move(animation_info);
	m_animation_count++;

	return m_animation_map[animation_name].get();
}

Animation_Info* AnimationManager::Get_Animation(std::wstring animation_name) {
	return m_animation_map[animation_name].get();
}
