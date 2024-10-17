#include "FBXManager.h"
#include "MeshCreater.h"
#include "ObjectManager.h"

FbxAMatrix Normalization_FbxMatrix(const FbxAMatrix& fbxamatrix) {
    FbxAMatrix result;

    result[0][0] = fbxamatrix[0][0];
    result[1][0] = fbxamatrix[1][0];
    result[2][0] = fbxamatrix[2][0];
    result[3][0] = fbxamatrix[3][0];

    result[0][1] = -fbxamatrix[0][2];
    result[1][1] = -fbxamatrix[1][2];
    result[2][1] = -fbxamatrix[2][2];
    result[3][1] = -fbxamatrix[3][2];

    result[0][2] = fbxamatrix[0][1];
    result[1][2] = fbxamatrix[1][1];
    result[2][2] = fbxamatrix[2][1];
    result[3][2] = fbxamatrix[3][1];

    result[0][3] = fbxamatrix[0][3];
    result[1][3] = fbxamatrix[1][3];
    result[2][3] = fbxamatrix[2][3];
    result[3][3] = fbxamatrix[3][3];

    return result;
}

std::unique_ptr<FBXManager> FBXManager::fbx_manager = nullptr;

FBXManager* FBXManager::Get_Inst() {
    if (!fbx_manager) {
        fbx_manager = std::make_unique<FBXManager>();
    }

    return fbx_manager.get();
}

MeshData FBXManager::Ipt_Mesh_From_File(std::wstring file_name) {
    FbxManager* manager = FbxManager::Create();

    FbxIOSettings* ios = FbxIOSettings::Create(manager, IOSROOT);
    manager->SetIOSettings(ios);

    FbxImporter* importer = FbxImporter::Create(manager, "");
    bool result = importer->Initialize(WStr_2_Str(file_name).c_str(), -1, manager->GetIOSettings());

    if (!result) {
        OutputDebugString(L"Importer Initialize Failed");
        exit(-1);
    }

    FbxScene* scene = FbxScene::Create(manager, "scene");

    importer->Import(scene);
    importer->Destroy();

    FbxAxisSystem axis_system = FbxAxisSystem::OpenGL;
    axis_system.ConvertScene(scene);

    FbxGeometryConverter converter(manager);
    converter.Triangulate(scene, true);

    //
    MeshData mesh_data;

    Find_N_Prcs_Node(scene->GetRootNode(), mesh_data);

    //
    manager->Destroy();

    return mesh_data;
}

void FBXManager::Find_N_Prcs_Node(FbxNode* node, MeshData& mesh_data) {
    FbxNodeAttribute* node_attribute = node->GetNodeAttribute();

    if (node_attribute) {
        switch (node_attribute->GetAttributeType()) {
        case FbxNodeAttribute::eMesh:
            Add_Mesh_From_Node(node, mesh_data);
            break;
        default:
            break;
        }
    }

    UINT child_count = node->GetChildCount();

    for (UINT i = 0; i < child_count; ++i) {
        Find_N_Prcs_Node(node->GetChild(i), mesh_data);
    }
}

void FBXManager::Add_Mesh_From_Node(FbxNode* node, MeshData& mesh_data) {
    //OutputDebugStringA(node->GetName());
    //OutputDebugStringA("\n");

    FbxVector4 pivot_position = node->EvaluateGlobalTransform().GetT();

    char str[1024];
    sprintf_s(str, "Mesh : %s\nPivot %f %f %f", node->GetName(), pivot_position[0], pivot_position[1], pivot_position[2]);
    OutputDebugStringA(str);

    FbxAMatrix local_transform_matrix = node->EvaluateLocalTransform();
    FbxAMatrix global_transform_matrix = node->EvaluateGlobalTransform();

    //FbxAMatrix transform_matrix = local_transform_matrix;
    FbxAMatrix transform_matrix = global_transform_matrix;// *local_transform_matrix;
    FbxAMatrix rotate_matrix;
    rotate_matrix.SetR(global_transform_matrix.GetR());

    DirectX::XMMATRIX transform_xmmatrix = {
        (float)transform_matrix[0][0], (float)transform_matrix[0][1], (float)transform_matrix[0][2], (float)transform_matrix[0][3],
        (float)transform_matrix[1][0], (float)transform_matrix[1][1], (float)transform_matrix[1][2], (float)transform_matrix[1][3],
        (float)transform_matrix[2][0], (float)transform_matrix[2][1], (float)transform_matrix[2][2], (float)transform_matrix[2][3],
        (float)transform_matrix[3][0], (float)transform_matrix[3][1], (float)transform_matrix[3][2], (float)transform_matrix[3][3],
    };

    DirectX::XMMATRIX rotate_xmmatrix = {
        (float)rotate_matrix[0][0], (float)rotate_matrix[0][1], (float)rotate_matrix[0][2], (float)rotate_matrix[0][3],
        (float)rotate_matrix[1][0], (float)rotate_matrix[1][1], (float)rotate_matrix[1][2], (float)rotate_matrix[1][3],
        (float)rotate_matrix[2][0], (float)rotate_matrix[2][1], (float)rotate_matrix[2][2], (float)rotate_matrix[2][3],
        (float)rotate_matrix[3][0], (float)rotate_matrix[3][1], (float)rotate_matrix[3][2], (float)rotate_matrix[3][3],
    };

    //
    FbxMesh* mesh = node->GetMesh();

    UINT triangle_count = mesh->GetPolygonCount();
    UINT vertex_count = triangle_count * 3;

    UINT current_vertex_count = (UINT)mesh_data.vertices.size();

    for (UINT i = 0; i < vertex_count; ++i) {
        std::uint32_t control_point_index = mesh->GetPolygonVertex(i / 3, i % 3);

        mesh_data.indices_32.emplace_back(i + current_vertex_count);

        VertexData vertex_data;
        vertex_data.position = Get_Pos(mesh, i / 3, i % 3);
        vertex_data.normal = Get_Norm(mesh, control_point_index, i);
        vertex_data.tangent = Get_Tan(mesh, control_point_index, i);
        vertex_data.uv = Get_UV(mesh, control_point_index, i);

        vertex_data.position = MathHelper::Multiply(vertex_data.position, transform_xmmatrix);
        vertex_data.normal = MathHelper::Multiply(vertex_data.normal, rotate_xmmatrix);
        vertex_data.tangent = MathHelper::Multiply(vertex_data.tangent, rotate_xmmatrix);
        //vertex_data.uv = MathHelper::Multiply(vertex_data.uv, transform_xmmatrix);

        mesh_data.vertices.emplace_back(vertex_data);

        //
        mesh_data.minimum_x = MathHelper::Min(mesh_data.minimum_x, vertex_data.position.x);
        mesh_data.minimum_y = MathHelper::Min(mesh_data.minimum_y, vertex_data.position.y);
        mesh_data.minimum_z = MathHelper::Min(mesh_data.minimum_z, vertex_data.position.z);

        mesh_data.maximum_x = MathHelper::Max(mesh_data.maximum_x, vertex_data.position.x);
        mesh_data.maximum_y = MathHelper::Max(mesh_data.maximum_y, vertex_data.position.y);
        mesh_data.maximum_z = MathHelper::Max(mesh_data.maximum_z, vertex_data.position.z);
    }
}

DirectX::XMFLOAT3 FBXManager::Get_Pos(FbxMesh* mesh, UINT triangle_count, UINT index_count) {
    int index = mesh->GetPolygonVertex(triangle_count, index_count);
    FbxVector4 position_value = mesh->GetControlPointAt(index);

    DirectX::XMFLOAT3 position;
    position.x = (float)position_value[0];
    position.y = (float)position_value[1];
    position.z = (float)position_value[2];

    return position;
}

DirectX::XMFLOAT3 FBXManager::Get_Norm(FbxMesh* mesh, UINT control_point_index, UINT vertex_count) {
    if (mesh->GetElementNormalCount() == 0) {
        OutputDebugString(L"Normal Info Not Found\n");

        return DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
    }

    FbxGeometryElementNormal* normal = mesh->GetElementNormal(0);

    switch (normal->GetMappingMode()) {
    case FbxGeometryElement::eByControlPoint:
        switch (normal->GetReferenceMode()) {
        case FbxGeometryElement::eDirect:
        {
            FbxDouble* normal_value = normal->GetDirectArray().GetAt(control_point_index).mData;

            DirectX::XMFLOAT3 normal_vector;
            normal_vector.x = (float)normal_value[0];
            normal_vector.y = (float)normal_value[1];
            normal_vector.z = (float)normal_value[2];

            return normal_vector;
        }
        case FbxGeometryElement::eIndexToDirect:
        {
            int index = normal->GetIndexArray().GetAt(control_point_index);
            FbxDouble* normal_value = normal->GetDirectArray().GetAt(index).mData;

            DirectX::XMFLOAT3 normal_vector;
            normal_vector.x = (float)normal_value[0];
            normal_vector.y = (float)normal_value[1];
            normal_vector.z = (float)normal_value[2];

            return normal_vector;
        }
        default:
            OutputDebugString(L"Unexpected Reference Mode\n");
            break;
        }
        break;
    case FbxGeometryElement::eByPolygonVertex:
        switch (normal->GetReferenceMode()) {
        case FbxGeometryElement::eDirect:
        {
            FbxDouble* normal_value = normal->GetDirectArray().GetAt(vertex_count).mData;

            DirectX::XMFLOAT3 normal_vector;
            normal_vector.x = (float)normal_value[0];
            normal_vector.y = (float)normal_value[1];
            normal_vector.z = (float)normal_value[2];

            return normal_vector;
        }
        case FbxGeometryElement::eIndexToDirect:
        {
            int index = normal->GetIndexArray().GetAt(vertex_count);
            FbxDouble* normal_value = normal->GetDirectArray().GetAt(index).mData;

            DirectX::XMFLOAT3 normal_vector;
            normal_vector.x = (float)normal_value[0];
            normal_vector.y = (float)normal_value[1];
            normal_vector.z = (float)normal_value[2];

            return normal_vector;
        }
        default:
            OutputDebugString(L"Unexpected Reference Mode\n");
            break;
        }
        break;
    default:
        OutputDebugString(L"Unexpected Mapping Mode\n");
        break;
    }

    return DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
}

DirectX::XMFLOAT3 FBXManager::Get_Tan(FbxMesh* mesh, UINT control_point_index, UINT vertex_count) {
    if (mesh->GetElementTangentCount() == 0) {
        //OutputDebugString(L"Tangent Info Not Found\n");

        return DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
    }

    FbxGeometryElementTangent* tangent = mesh->GetElementTangent(0);

    switch (tangent->GetMappingMode()) {
    case FbxGeometryElement::eByControlPoint:
        switch (tangent->GetReferenceMode()) {
        case FbxGeometryElement::eDirect:
        {
            FbxDouble* tangent_value = tangent->GetDirectArray().GetAt(control_point_index).mData;

            DirectX::XMFLOAT3 tangent_vector;
            tangent_vector.x = (float)tangent_value[0];
            tangent_vector.y = (float)tangent_value[1];
            tangent_vector.z = (float)tangent_value[2];

            return tangent_vector;
        }
        case FbxGeometryElement::eIndexToDirect:
        {
            int index = tangent->GetIndexArray().GetAt(control_point_index);
            FbxDouble* tangent_value = tangent->GetDirectArray().GetAt(index).mData;

            DirectX::XMFLOAT3 tangent_vector;
            tangent_vector.x = (float)tangent_value[0];
            tangent_vector.y = (float)tangent_value[1];
            tangent_vector.z = (float)tangent_value[2];

            return tangent_vector;
        }
        default:
            OutputDebugString(L"Unexpected Reference Mode\n");
            break;
        }
        break;
    case FbxGeometryElement::eByPolygonVertex:
        switch (tangent->GetReferenceMode()) {
        case FbxGeometryElement::eDirect:
        {
            FbxDouble* tangent_value = tangent->GetDirectArray().GetAt(vertex_count).mData;

            DirectX::XMFLOAT3 tangent_vector;
            tangent_vector.x = (float)tangent_value[0];
            tangent_vector.y = (float)tangent_value[1];
            tangent_vector.z = (float)tangent_value[2];

            return tangent_vector;
        }
        case FbxGeometryElement::eIndexToDirect:
        {
            int index = tangent->GetIndexArray().GetAt(vertex_count);
            FbxDouble* tangent_value = tangent->GetDirectArray().GetAt(index).mData;

            DirectX::XMFLOAT3 tangent_vector;
            tangent_vector.x = (float)tangent_value[0];
            tangent_vector.y = (float)tangent_value[1];
            tangent_vector.z = (float)tangent_value[2];

            return tangent_vector;
        }
        default:
            OutputDebugString(L"Unexpected Reference Mode\n");
            break;
        }
        break;
    default:
        OutputDebugString(L"Unexpected Mapping Mode\n");
        break;
    }

    return DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
}

DirectX::XMFLOAT2 FBXManager::Get_UV(FbxMesh* mesh, UINT control_point_index, UINT vertex_count) {
    if (mesh->GetElementUVCount() == 0) {
        OutputDebugString(L"UV Info Not Found\n");

        return DirectX::XMFLOAT2(0.0f, 0.0f);
    }

    FbxGeometryElementUV* uv = mesh->GetElementUV(0);

    switch (uv->GetMappingMode()) {
    case FbxGeometryElement::eByControlPoint:
        switch (uv->GetReferenceMode()) {
        case FbxGeometryElement::eDirect:
        {
            FbxDouble* uv_value = uv->GetDirectArray().GetAt(control_point_index).mData;

            DirectX::XMFLOAT2 uv_vector;
            uv_vector.x = (float)uv_value[0];
            uv_vector.y = (float)uv_value[1];

            return uv_vector;
        }
        case FbxGeometryElement::eIndexToDirect:
        {
            int index = uv->GetIndexArray().GetAt(control_point_index);
            FbxDouble* uv_value = uv->GetDirectArray().GetAt(index).mData;

            DirectX::XMFLOAT2 uv_vector;
            uv_vector.x = (float)uv_value[0];
            uv_vector.y = (float)uv_value[1];

            return uv_vector;
        }
        default:
            OutputDebugString(L"Unexpected Reference Mode\n");
            break;
        }
        break;
    case FbxGeometryElement::eByPolygonVertex:
        switch (uv->GetReferenceMode()) {
        case FbxGeometryElement::eDirect:
        {
            FbxDouble* uv_value = uv->GetDirectArray().GetAt(vertex_count).mData;

            DirectX::XMFLOAT2 uv_vector;
            uv_vector.x = (float)uv_value[0];
            uv_vector.y = (float)uv_value[1];

            return uv_vector;
        }
        case FbxGeometryElement::eIndexToDirect:
        {
            int index = uv->GetIndexArray().GetAt(vertex_count);
            FbxDouble* uv_value = uv->GetDirectArray().GetAt(index).mData;

            DirectX::XMFLOAT2 uv_vector;
            uv_vector.x = (float)uv_value[0];
            uv_vector.y = (float)uv_value[1];

            return uv_vector;
        }
        default:
            OutputDebugString(L"Unexpected Reference Mode\n");
            break;
        }
        break;
    default:
        OutputDebugString(L"Unexpected Mapping Mode\n");
        break;
    }

    return DirectX::XMFLOAT2(0.0f, 0.0f);
}

void FBXManager::Ipt_From_File(ObjectManager* object_maanger, std::wstring file_name, bool merge_mesh, bool add_object, bool merge_object, BYTE info_flag, std::wstring skeleton_name) {
    FbxManager* manager = FbxManager::Create();
    FbxScene* scene = Ipt_Scene(manager, file_name);

    // Prcs Nodes
    std::vector<FbxNode*> bone_node_array;
    std::vector<Bone_Info> bone_array;
    std::unordered_map<std::wstring, UINT> bone_index_map;

    Mesh_Info mesh_info;
    std::vector<Mesh> mesh_array;

    // process skeleton info first
    if (info_flag & SKELETON_INFO || info_flag & ANIMATION_INFO) {
        Prcs_Node(file_name, scene->GetRootNode(), object_maanger, bone_node_array,
            bone_array, bone_index_map,
            mesh_info, mesh_array,
            merge_mesh, add_object, merge_object, SKELETON_INFO);

        if (info_flag & SKELETON_INFO) {
            object_maanger->Get_Skeleton_Manager().Add_Skeleton(file_name, bone_array);
        }
    }

    // process mesh info
    Prcs_Node(file_name, scene->GetRootNode(), object_maanger, bone_node_array,
        bone_array, bone_index_map,
        mesh_info, mesh_array,
        merge_mesh, add_object, merge_object, info_flag);

    // Prcs Animation Data
    std::map<float, Keyframe_Info> keyframe_map;

    if (info_flag & ANIMATION_INFO) {
        Skeleton_Info* skeleton_info;

        if (info_flag & SKELETON_INFO) {
            skeleton_info = object_maanger->Get_Skeleton_Manager().Get_Skeleton(file_name);
        }
        else {
            skeleton_info = object_maanger->Get_Skeleton_Manager().Get_Skeleton(skeleton_name);
        }

        Prcs_Animation(file_name, scene, object_maanger, skeleton_info, bone_node_array, keyframe_map);
    }

    // Add object
    if (info_flag & MESH_INFO &&  merge_mesh) {
        object_maanger->Get_Mesh_Manager().Add_Mesh(file_name, mesh_info.vertices, mesh_info.indices_32);
    }

    if (add_object) {
        if (merge_mesh) {
            object_maanger->Add_Obj(file_name, file_name);

            if (info_flag & SKELETON_INFO) {
                object_maanger->Set_Sklt_2_Obj(file_name, file_name);
            }
        }
        else {
            if (merge_object) {
                object_maanger->Add_Obj(file_name, mesh_array);

                if (info_flag & SKELETON_INFO) {
                    object_maanger->Set_Sklt_2_Obj(file_name, file_name);
                }
            }
        }
    }

    //
    manager->Destroy();
}

FbxScene* FBXManager::Ipt_Scene(FbxManager* manager, std::wstring file_name) {
    FbxIOSettings* ios = FbxIOSettings::Create(manager, IOSROOT);
    manager->SetIOSettings(ios);

    FbxImporter* importer = FbxImporter::Create(manager, "");
    bool result = importer->Initialize(WStr_2_Str(file_name).c_str(), -1, manager->GetIOSettings());

    if (!result) {
        OutputDebugString(L"Importer Initialize Failed");
        exit(-1);
    }

    FbxScene* scene = FbxScene::Create(manager, "scene");

    importer->Import(scene);
    importer->Destroy();

    FbxAxisSystem axis_system = FbxAxisSystem::OpenGL;
    //FbxAxisSystem axis_system = FbxAxisSystem::DirectX;
    axis_system.ConvertScene(scene);

    FbxGeometryConverter converter(manager);
    converter.Triangulate(scene, true);

    return scene;
}

void FBXManager::Prcs_Node(
    std::wstring file_name, FbxNode* node, ObjectManager* object_maanger, std::vector<FbxNode*>& bone_node_array,
    std::vector<Bone_Info>& bone_array, std::unordered_map<std::wstring, UINT>& bone_index_map,
    Mesh_Info& mesh_info, std::vector<Mesh>& mesh_array,
    bool merge_mesh, bool add_object, bool merge_object, BYTE info_flag
) {
    FbxNodeAttribute* node_attribute = node->GetNodeAttribute();

    if (node_attribute) {
        switch (node_attribute->GetAttributeType()) {
        case FbxNodeAttribute::eMesh:
            if (info_flag & MESH_INFO) {
                Prcs_Mesh_Node(file_name, node, object_maanger,
                    bone_index_map,
                    mesh_info, mesh_array,
                    merge_mesh, add_object, merge_object, info_flag);
            }
            break;
        case FbxNodeAttribute::eNull:   // dummy
        case FbxNodeAttribute::eSkeleton:   // bone
            if (info_flag == SKELETON_INFO) {
                Prcs_Skeleton_Node(
                    file_name, node, bone_node_array,
                    bone_array, bone_index_map);
            }
            break;
        default:
            break;
        }
    }

    UINT child_count = node->GetChildCount();

    for (UINT i = 0; i < child_count; ++i) {
        Prcs_Node(file_name, node->GetChild(i), object_maanger, bone_node_array,
            bone_array, bone_index_map,
            mesh_info, mesh_array,
            merge_mesh, add_object, merge_object, info_flag);
    }
}

void FBXManager::Prcs_Mesh_Node(
    std::wstring file_name, FbxNode* node, ObjectManager* object_maanger,
    std::unordered_map<std::wstring, UINT>& bone_index_map,
    Mesh_Info& mesh_info, std::vector<Mesh>& mesh_array,
    bool merge_mesh, bool add_object, bool merge_object, BYTE info_flag
) {
    if (!(info_flag & MESH_INFO)) {
        return;
    }

    std::wstring node_name = Str_2_WStr(node->GetName());
    FbxMesh* mesh = node->GetMesh();

    //
    std::unordered_map<UINT, Weight_Info> vertex_weight_info_map;

    if (info_flag & SKELETON_INFO) {
        int deformer_count = mesh->GetDeformerCount(FbxDeformer::eSkin);

        for (int i = 0; i < deformer_count; ++i) {
            FbxSkin* skin = (FbxSkin*)mesh->GetDeformer(i, FbxDeformer::eSkin);

            int cluster_count = skin->GetClusterCount();

            for (int j = 0; j < cluster_count; ++j) {
                FbxCluster* cluster = skin->GetCluster(j);
                FbxNode* bone_node = cluster->GetLink();

                if (bone_node) {
                    int* indices = cluster->GetControlPointIndices();
                    double* weights = cluster->GetControlPointWeights();
                    int index_count = cluster->GetControlPointIndicesCount();

                    for (int k = 0; k < index_count; ++k) {
                        Weight_Info& weight_info = vertex_weight_info_map[indices[k]];
                        weight_info.bone_indices[weight_info.bone_count] = bone_index_map[Str_2_WStr(bone_node->GetName())];
                        weight_info.bone_weights[weight_info.bone_count] = (float)weights[k];
                        weight_info.bone_count++;
                    }
                }
            }
        }

        //// normalize
        //for (auto& i : vertex_weight_info_map) {
        //    Weight_Info& weight_info = i.second;

        //    float sum = 0.0f;
        //    for (UINT j = 0; j < weight_info.bone_count; ++j) {
        //        sum += weight_info.bone_weights[j];
        //    }

        //    if (sum != 1.0f) {
        //        for (UINT j = 0; j < weight_info.bone_count; ++j) {
        //            weight_info.bone_weights[j] /= sum;
        //        }
        //    }
        //}
    }

    //
    std::vector<Vertex_Info> vertex_array;
    std::vector<std::uint32_t> index_array;

    FbxAMatrix global_transform_matrix = node->EvaluateGlobalTransform();
    FbxAMatrix rotate_matrix;
    rotate_matrix.SetR(global_transform_matrix.GetR());

    DirectX::XMMATRIX global_transform_xmmatrix = FbxAMatrix_2_XMMATRIX(global_transform_matrix);
    DirectX::XMMATRIX rotate_xmmatrix = FbxAMatrix_2_XMMATRIX(rotate_matrix);

    //
    UINT triangle_count = mesh->GetPolygonCount();
    UINT vertex_count = triangle_count * 3;

    for (UINT i = 0; i < vertex_count; ++i) {
        std::uint32_t control_point_index = mesh->GetPolygonVertex(i / 3, i % 3);

        index_array.emplace_back(i);

        Vertex_Info vertex_info;
        vertex_info.position = Get_Pos(mesh, i / 3, i % 3);
        vertex_info.normal = Get_Norm(mesh, control_point_index, i);
        vertex_info.tangent = Get_Tan(mesh, control_point_index, i);
        vertex_info.uv = Get_UV(mesh, control_point_index, i);

        if (info_flag & SKELETON_INFO) {
            vertex_info.bone_count = vertex_weight_info_map[control_point_index].bone_count;

            for (UINT j = 0; j < vertex_info.bone_count; ++j) {
                vertex_info.bone_indices[j] = vertex_weight_info_map[control_point_index].bone_indices[j];
                vertex_info.bone_weights[j] = vertex_weight_info_map[control_point_index].bone_weights[j];
            }
        }

        if (merge_mesh) {
            vertex_info.position = MathHelper::Multiply(vertex_info.position, global_transform_xmmatrix);
            vertex_info.normal = MathHelper::Multiply(vertex_info.normal, rotate_xmmatrix);
            vertex_info.tangent = MathHelper::Multiply(vertex_info.tangent, rotate_xmmatrix);
        }

        vertex_array.emplace_back(vertex_info);
    }

    if (merge_mesh) {
        mesh_info.Add_Info(vertex_array, index_array);
    }
    else {
        Mesh_Info* added_mesh_info = object_maanger->Get_Mesh_Manager().Add_Mesh(node_name, vertex_array, index_array);

        if (add_object) {
            if (merge_object) {
                mesh_array.emplace_back(added_mesh_info, XMMATRIX_2_XMFLOAT4X4(global_transform_xmmatrix));
            }
            else {
                object_maanger->Add_Obj(node_name, node_name, L"Object", global_transform_xmmatrix);

                if (info_flag & SKELETON_INFO) {
                    object_maanger->Set_Sklt_2_Obj(node_name, file_name);
                }
            }
        }
    }
}

void FBXManager::Prcs_Skeleton_Node(
    std::wstring file_name, FbxNode* node, std::vector<FbxNode*>& bone_node_array,
    std::vector<Bone_Info>& bone_array, std::unordered_map<std::wstring, UINT>& bone_index_map
) {
    Bone_Info bone_info;
    bone_info.name = Str_2_WStr(node->GetName());

    FbxNode* parent_node = node->GetParent();
    if (parent_node->GetNodeAttribute() != NULL) {
        //DirectX::XMStoreFloat4x4(&bone_info.offset_matrix, FbxAMatrix_2_XMMATRIX(parent_node->EvaluateGlobalTransform().Inverse()));
        DirectX::XMStoreFloat4x4(&bone_info.offset_matrix, FbxAMatrix_2_XMMATRIX(node->EvaluateGlobalTransform().Inverse()));
        bone_info.parent_bone_index = bone_index_map[Str_2_WStr(parent_node->GetName())];
        bone_info.bone_index = (UINT)bone_array.size();
    }
    else {
        //bone_info.offset_matrix = MathHelper::Identity_4x4();
        DirectX::XMStoreFloat4x4(&bone_info.offset_matrix, FbxAMatrix_2_XMMATRIX(node->EvaluateGlobalTransform().Inverse()));
        bone_info.parent_bone_index = -1;
        bone_info.bone_index = 0;
    }

    bone_array.emplace_back(bone_info);
    bone_index_map[bone_info.name] = bone_info.bone_index;

    bone_node_array.emplace_back(node);
}

void FBXManager::Prcs_Animation(std::wstring file_name, FbxScene* scene, ObjectManager* object_maanger,
    Skeleton_Info* skeleton_info, std::vector<FbxNode*>& bone_node_array, std::map<float, Keyframe_Info>& keyframe_map
) {
    int animstack_count = scene->GetSrcObjectCount<FbxAnimStack>();

    if (animstack_count) {
        if (bone_node_array.size() == 0) {
            Get_Null_Bone(scene->GetRootNode(), bone_node_array);
        }

        FbxAnimStack* animstack = scene->GetSrcObject<FbxAnimStack>(0);
        FbxAnimLayer* animlayer = animstack->GetMember<FbxAnimLayer>();

        std::set<FbxTime> keyframe_times;
        for (auto& bn : bone_node_array) {
            Calc_Keyframe_Times(keyframe_times, bn, animlayer);
        }

        for (auto& t : keyframe_times) {
            Add_Keyframe(t, skeleton_info, bone_node_array, keyframe_map);
        }

        float total_time = keyframe_map.rbegin()->second.time;
        object_maanger->Get_Animation_Manager().Add_Animation(file_name, (UINT)bone_node_array.size(), total_time, keyframe_map);
    }
}

void FBXManager::Calc_Keyframe_Times(std::set<FbxTime>& keyframe_times, FbxNode* node, FbxAnimLayer* animlayer) {
    FbxAnimCurve* translationXCurve = node->LclTranslation.GetCurve(animlayer, FBXSDK_CURVENODE_COMPONENT_X);
    FbxAnimCurve* translationYCurve = node->LclTranslation.GetCurve(animlayer, FBXSDK_CURVENODE_COMPONENT_Y);
    FbxAnimCurve* translationZCurve = node->LclTranslation.GetCurve(animlayer, FBXSDK_CURVENODE_COMPONENT_Z);

    FbxAnimCurve* rotationXCurve = node->LclRotation.GetCurve(animlayer, FBXSDK_CURVENODE_COMPONENT_X);
    FbxAnimCurve* rotationYCurve = node->LclRotation.GetCurve(animlayer, FBXSDK_CURVENODE_COMPONENT_Y);
    FbxAnimCurve* rotationZCurve = node->LclRotation.GetCurve(animlayer, FBXSDK_CURVENODE_COMPONENT_Z);

    FbxAnimCurve* scaleXCurve = node->LclScaling.GetCurve(animlayer, FBXSDK_CURVENODE_COMPONENT_X);
    FbxAnimCurve* scaleYCurve = node->LclScaling.GetCurve(animlayer, FBXSDK_CURVENODE_COMPONENT_Y);
    FbxAnimCurve* scaleZCurve = node->LclScaling.GetCurve(animlayer, FBXSDK_CURVENODE_COMPONENT_Z);

    Get_Keyframe_Times_From_Curve(keyframe_times, translationXCurve);
    Get_Keyframe_Times_From_Curve(keyframe_times, translationYCurve);
    Get_Keyframe_Times_From_Curve(keyframe_times, translationZCurve);
    Get_Keyframe_Times_From_Curve(keyframe_times, rotationXCurve);
    Get_Keyframe_Times_From_Curve(keyframe_times, rotationYCurve);
    Get_Keyframe_Times_From_Curve(keyframe_times, rotationZCurve);
    Get_Keyframe_Times_From_Curve(keyframe_times, scaleXCurve);
    Get_Keyframe_Times_From_Curve(keyframe_times, scaleYCurve);
    Get_Keyframe_Times_From_Curve(keyframe_times, scaleZCurve);
}

void FBXManager::Get_Keyframe_Times_From_Curve(std::set<FbxTime>& keyframe_times, FbxAnimCurve* curve) {
    if (curve) {
        int keyframe_count = curve->KeyGetCount();

        for (int i = 0; i < keyframe_count; ++i) {
            keyframe_times.insert(curve->KeyGetTime(i));
        }
    }
}

void FBXManager::Add_Keyframe(FbxTime time, Skeleton_Info* skeleton_info, std::vector<FbxNode*>& bone_node_array, std::map<float, Keyframe_Info>& keyframe_map) {
    Keyframe_Info keyframe_info;
    keyframe_info.time = (float)time.GetSecondDouble();

    //std::array<DirectX::XMMATRIX, MAX_BONE_COUNT> global_transform_matrix_array;

    //global_transform_matrix_array[0] = FbxAMatrix_2_XMMATRIX(bone_node_array[0]->EvaluateLocalTransform(time));
    ////global_transform_matrix_array[0] = FbxAMatrix_2_XMMATRIX(Normalization_FbxMatrix(bone_node_array[0]->EvaluateLocalTransform(time)));

    //for (UINT i = 1; i < (UINT)bone_node_array.size(); ++ i) {
    //    auto& bone_node = bone_node_array[i];

    //    global_transform_matrix_array[i] = FbxAMatrix_2_XMMATRIX(bone_node->EvaluateLocalTransform(time))
    //        * global_transform_matrix_array[skeleton_info->bone_array[i].parent_bone_index];
    //}

    //for (UINT i = 0; i < bone_node_array.size(); ++i) {
    //    DirectX::XMMATRIX animation_fransform_matrix = DirectX::XMLoadFloat4x4(&skeleton_info->bone_offset_matrix_array[i])
    //        * global_transform_matrix_array[i];

    //    DirectX::XMVECTOR translate, rotate, scale;

    //    DirectX::XMMatrixDecompose(&scale, &rotate, &translate, animation_fransform_matrix);

    //    DirectX::XMStoreFloat3(&keyframe_info.animation_transform_array[i].trenslate, translate);
    //    DirectX::XMStoreFloat4(&keyframe_info.animation_transform_array[i].rotate, rotate);
    //    DirectX::XMStoreFloat3(&keyframe_info.animation_transform_array[i].scale, scale);
    //}

    UINT bone_count = 0;
    DirectX::XMMATRIX global_transform_matrix;

    for (auto& bn : bone_node_array) {
        global_transform_matrix = FbxAMatrix_2_XMMATRIX(bn->EvaluateGlobalTransform(time));
        DirectX::XMMATRIX animation_fransform_matrix = DirectX::XMLoadFloat4x4(&skeleton_info->bone_offset_matrix_array[bone_count]) * global_transform_matrix;

        DirectX::XMVECTOR translate, rotate, scale;

        DirectX::XMMatrixDecompose(&scale, &rotate, &translate, animation_fransform_matrix);

        DirectX::XMStoreFloat3(&keyframe_info.animation_transform_array[bone_count].trenslate, translate);
        DirectX::XMStoreFloat4(&keyframe_info.animation_transform_array[bone_count].rotate, rotate);
        DirectX::XMStoreFloat3(&keyframe_info.animation_transform_array[bone_count].scale, scale);

        //keyframe_info.animation_transform_array[bone_count].trenslate = FbxVector4_2_XMFLOAT3(global_transform_matrix.GetT());
        //keyframe_info.animation_transform_array[bone_count].rotate = FbxQuaternion_2_XMFLOAT4(global_transform_matrix.GetQ());
        //keyframe_info.animation_transform_array[bone_count].scale = FbxVector4_2_XMFLOAT3(global_transform_matrix.GetS());
        bone_count++;
    }

    keyframe_map[keyframe_info.time] = keyframe_info;
}

void FBXManager::Get_Null_Bone(FbxNode* node, std::vector<FbxNode*>& bone_node_array) {
    if (strcmp(node->GetName(), "RootNode") && !node->GetNodeAttribute()) {
        bone_node_array.emplace_back(node);
    }

    UINT child_count = node->GetChildCount();

    for (UINT i = 0; i < child_count; ++i) {
        Get_Null_Bone(node->GetChild(i), bone_node_array);
    }
}
