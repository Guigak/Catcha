#pragma once
#include "common.h"

class SkeletonManager {
private:
	int m_skeleton_count = 0;

	std::unordered_map<std::wstring, std::unique_ptr<Skeleton_Info>> m_skeleton_map;

public:
	SkeletonManager() {}
	~SkeletonManager() {}

	Skeleton_Info* Add_Skeleton(std::wstring skeleton_name, std::unique_ptr<Skeleton_Info> skeleton_info);
	Skeleton_Info* Add_Skeleton(std::wstring skeleton_name, std::vector<Bone_Info>& bone_array);

	Skeleton_Info* Get_Skeleton(std::wstring skeleton_name);
};

