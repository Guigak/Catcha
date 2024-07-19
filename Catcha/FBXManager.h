#pragma once
#include "common.h"

struct MeshData;

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
};

