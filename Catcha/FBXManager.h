#pragma once
#include "common.h"

struct MeshData;

struct Weight_Info {
	UINT bone_count;
	UINT bone_indices[MAX_WEIGHT_BONE_COUNT];
	float bone_weights[MAX_WEIGHT_BONE_COUNT];
};

class ObjectManager;

class FBXManager {
private:
	//static FBXManager* fbx_manager;
	static std::unique_ptr<FBXManager> fbx_manager;

public:
	FBXManager() {}
	~FBXManager() {}

	static FBXManager* Get_Inst();	// Get Instance

	MeshData Ipt_Mesh_From_File(std::wstring file_name);

	void Find_N_Prcs_Node(FbxNode* node, MeshData& mesh_data);

	void Add_Mesh_From_Node(FbxNode* node, MeshData& mesh_data);

	DirectX::XMFLOAT3 Get_Pos(FbxMesh* mesh, UINT triangle_count, UINT index_count);	// Get Position
	DirectX::XMFLOAT3 Get_Norm(FbxMesh* mesh, UINT control_point_index, UINT vertex_count);	// Get Normal
	DirectX::XMFLOAT3 Get_Tan(FbxMesh* mesh, UINT control_point_index, UINT vertex_count);	// Get Tangent
	DirectX::XMFLOAT2 Get_UV(FbxMesh* mesh, UINT control_point_index, UINT vertex_count);	// Get UV

	void Ipt_From_File(ObjectManager* object_maanger, std::wstring file_name,
		bool merge_mesh, bool add_object, bool merge_object, BYTE info_flag, std::wstring skeleton_name);	// Import From File

	FbxScene* Ipt_Scene(FbxManager* manager, std::wstring file_name);	// Import Scene

	void Prcs_Node(
		std::wstring file_name, FbxNode* node, ObjectManager* object_maanger, std::vector<FbxNode*>& bone_node_array,
		std::vector<Bone_Info>& bone_array, std::unordered_map<std::wstring, UINT>& bone_index_map,
		std::vector<Material_Info*>& material_info_array,
		Mesh_Info& mesh_info, std::vector<Mesh>& mesh_array,
		bool merge_mesh, bool add_object, bool merge_object, BYTE info_flag);

	void Prcs_Mesh_Node(
		std::wstring file_name, FbxNode* node, ObjectManager* object_maanger,
		std::unordered_map<std::wstring, UINT>& bone_index_map,
		std::vector<Material_Info*>& material_info_array,
		Mesh_Info& mesh_info, std::vector<Mesh>& mesh_array,
		bool merge_mesh, bool add_object, bool merge_object, BYTE info_flag);

	void Prcs_Skeleton_Node(
		std::wstring file_name, FbxNode* node, std::vector<FbxNode*>& bone_node_array,
		std::vector<Bone_Info>& bone_array, std::unordered_map<std::wstring, UINT>& bone_index_map);

	void Prcs_Animation(std::wstring file_name, FbxScene* scene, ObjectManager* object_maanger,
		Skeleton_Info* skeleton_info, std::vector<FbxNode*>& bone_node_array, std::map<float, Keyframe_Info>& keyframe_map);

	void Calc_Keyframe_Times(std::set<FbxTime>& keyframe_times, FbxNode* node, FbxAnimLayer* animlayer);
	void Get_Keyframe_Times_From_Curve(std::set<FbxTime>& keyframe_times, FbxAnimCurve* curve);
	void Add_Keyframe(FbxTime time, Skeleton_Info* skeleton_info, std::vector<FbxNode*>& bone_node_array, std::map<float, Keyframe_Info>& keyframe_map);

	void Get_Null_Bone(FbxNode* node, std::vector<FbxNode*>& bone_node_array);
};