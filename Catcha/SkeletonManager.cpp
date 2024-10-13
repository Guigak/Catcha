#include "SkeletonManager.h"

Skeleton_Info* SkeletonManager::Add_Skeleton(std::wstring skeleton_name, std::unique_ptr<Skeleton_Info> skeleton_info) {
    m_skeleton_map[skeleton_name] = std::move(skeleton_info);
    ++m_skeleton_count;

    return m_skeleton_map[skeleton_name].get();
}

Skeleton_Info* SkeletonManager::Add_Skeleton(std::wstring skeleton_name, std::vector<Bone_Info>& bone_array) {
    std::unique_ptr<Skeleton_Info> skeleton_info = std::make_unique<Skeleton_Info>(skeleton_name, bone_array);

    m_skeleton_map[skeleton_name] = std::move(skeleton_info);
    ++m_skeleton_count;

    return m_skeleton_map[skeleton_name].get();
}

Skeleton_Info* SkeletonManager::Get_Skeleton(std::wstring skeleton_name) {
    return m_skeleton_map[skeleton_name].get();
}
