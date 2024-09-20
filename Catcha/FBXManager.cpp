#include "FBXManager.h"
#include "MeshCreater.h"

std::unique_ptr<FBXManager> FBXManager::fbx_manager = nullptr;

//FBXManager::~FBXManager() {
//    if (fbx_manager != nullptr) {
//        delete[] fbx_manager;
//    }
//}
//
//FBXManager* FBXManager::Get_Inst() {
//    if (fbx_manager == nullptr) {
//        fbx_manager = new FBXManager;
//    }
//
//    return fbx_manager;
//}

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

    //FbxAxisSystem axis_system(FbxAxisSystem::eYAxis, FbxAxisSystem::eParityOdd, FbxAxisSystem::eLeftHanded);
    FbxAxisSystem axis_system = FbxAxisSystem::DirectX;
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

    FbxAMatrix pivot_transform_matrix = FbxAMatrix(
        node->GetGeometricTranslation(FbxNode::eSourcePivot),
        node->GetGeometricRotation(FbxNode::eSourcePivot),
        node->GetGeometricScaling(FbxNode::eSourcePivot)
    );

    FbxAMatrix local_transform_matrix = node->EvaluateLocalTransform();
    //FbxAMatrix global_transform_matrix = node->GetParent()->EvaluateLocalTransform();

    FbxAMatrix transform_matrix = local_transform_matrix * pivot_transform_matrix;
    //FbxAMatrix transform_matrix = global_transform_matrix * local_transform_matrix * pivot_transform_matrix;

    DirectX::XMMATRIX transform_xmmatrix = {
        (float)transform_matrix[0][0], (float)transform_matrix[1][0], (float)transform_matrix[2][0], (float)transform_matrix[3][0],
        (float)transform_matrix[0][1], (float)transform_matrix[1][1], (float)transform_matrix[2][1], (float)transform_matrix[3][1],
        (float)transform_matrix[0][2], (float)transform_matrix[1][2], (float)transform_matrix[2][2], (float)transform_matrix[3][2],
        (float)transform_matrix[0][3], (float)transform_matrix[1][3], (float)transform_matrix[2][3], (float)transform_matrix[3][3],
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
        vertex_data.normal = MathHelper::Multiply(vertex_data.normal, transform_xmmatrix);
        vertex_data.tangent = MathHelper::Multiply(vertex_data.tangent, transform_xmmatrix);
        vertex_data.uv = MathHelper::Multiply(vertex_data.uv, transform_xmmatrix);

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
