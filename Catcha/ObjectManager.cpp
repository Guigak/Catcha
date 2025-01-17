#include "ObjectManager.h"
#include "InputManager.h"
#include "Camera.h"
#include "InstanceObject.h"
#include "VoxelCheese.h"
#include "TextUIObject.h"

Object* ObjectManager::Get_Obj(std::wstring object_name) {
    return m_object_map[object_name].get();
}

Object* ObjectManager::Get_Obj(UINT object_number) {
    return m_objects[object_number];
}

Object* ObjectManager::Get_Opaque_Obj(UINT object_number) {
    return m_opaque_objects[object_number];
}

Object* ObjectManager::Get_Transparent_Obj(UINT object_number) {
    return m_transparent_objects[object_number];
}

void ObjectManager::Move(std::wstring object_name, Action action, BYTE flag) {
    Object* object = Get_Obj(object_name);

    if (object->Get_Movable() == false) {
        return;
    }

    switch (action) {
    case Action::MOVE_FORWARD:
        object->Move_Forward(flag);
        break;
    case Action::MOVE_BACK:
        object->Move_Back(flag);
        break;
    case Action::MOVE_LEFT:
        object->Move_Left(flag);
        break;
    case Action::MOVE_RIGHT:
        object->Move_Right(flag);
        break;
    case Action::MOVE_UP:
        object->Move_Up(flag);
        break;
    case Action::MOVE_DOWN:
        object->Move_Down(flag);
        break;
    default:
        break;
    }
}

void ObjectManager::Teleport(std::wstring object_name, Action action, float distance) {
    Object* object = Get_Obj(object_name);

    switch (action) {
    case Action::TELEPORT_FORWARD:
        object->TP_Forward(distance);
        break;
    case Action::TELEPORT_BACK:
        object->TP_Back(distance);
        break;
    case Action::TELEPORT_LEFT:
        object->TP_Left(distance);
        break;
    case Action::TELEPORT_RIGHT:
        object->TP_Right(distance);
        break;
    case Action::TELEPORT_UP:
        object->TP_Up(distance);
        break;
    case Action::TELEPORT_DOWN:
        object->TP_Down(distance);
        break;
    default:
        break;
    }
}

void ObjectManager::Rotate(std::wstring object_name, Action action, float degree) {
    Object* object = Get_Obj(object_name);

    switch (action) {
    case Action::ROTATE:
        break;
    case Action::ROTATE_ROLL:
        object->Rotate_Roll(degree);
        break;
    case Action::ROTATE_PITCH:
        object->Rotate_Pitch(degree);
        break;
    case Action::ROTATE_YAW:
        object->Rotate_Yaw(degree);
        break;
    case Action::ROTATE_RIGHT:
        object->Rotate_Right(degree);
        break;
    case Action::ROTATE_LOOK:
        object->Rotate_Look(degree);
        break;
    default:
        break;
    }
}

void ObjectManager::Actions(std::wstring object_name, Action action) {
    Object* object = Get_Obj(object_name);

    switch (action) {
    case Action::ACTION_JUMP:
        object->Jump();
        break;
    case Action::ACTION_ONE:
        object->Act_One();
        break;
    case Action::ACTION_TWO:
        object->Act_Two();
        break;
    case Action::ACTION_THREE:
        object->Act_Three();
        break;
    default:
        break;
    }
}

void ObjectManager::Update(float elapsed_time) {
    m_material_manager.Update();

    for (auto& o : m_opaque_objects) {
        o->Calc_Delta(elapsed_time);
    }

    Solve_Collision();

    for (auto& o : m_opaque_objects) {
        o->Update(elapsed_time);
    }

    for (auto& o : m_camera_objects) {
        o->Update(elapsed_time);
    }

    for (auto& o : m_instance_objects) {
        o->Update(elapsed_time);
    }

    for (auto& o : m_text_UI_objects) {
        o->Update(elapsed_time);
    }

    //
    for (auto& o : m_collision_obb_objects) {
        o->Update(elapsed_time);
    }
}

void ObjectManager::Solve_Collision() {
    for (auto& m : m_collision_pair_map) {
        for (auto& s1 : m_object_set_map[m.first]) {
            for (auto& s2 : m_object_set_map[m.second]) {
                DirectX::BoundingOrientedBox OBB_a = s1->Get_Calcd_OBB();
                DirectX::BoundingOrientedBox OBB_b = s2->Get_Calcd_OBB();


            }
        }
    }
}

void ObjectManager::Add_Collision_Pair(std::wstring set_name_a, std::wstring set_name_b) {
    m_collision_pair_map[set_name_a] = set_name_b;
}

bool ObjectManager::Chk_Collision(const DirectX::BoundingOrientedBox& OBB_a, const DirectX::BoundingOrientedBox& OBB_b, DirectX::XMFLOAT3& normal, float& depth) {
    return false;
}

bool ObjectManager::Overlaped(DirectX::XMVECTOR corners_a[], DirectX::XMVECTOR corners_b[], const DirectX::XMVECTOR& axis, float& overlap) {
    return false;
}

void ObjectManager::Bind_Cam_2_Obj(std::wstring camera_name, std::wstring object_name,
	float offset_look, float offset_up, float offset_right, float distance, BYTE flag) {
    Camera* camera = (Camera*)Get_Obj(camera_name);
    Object* object = Get_Obj(object_name);

    object->Bind_Camera(camera);
    camera->Bind_Obj(object, offset_look, offset_up, offset_right, distance);

    object->Set_Cam_Rotate_Flag(flag);
}

void ObjectManager::Ipt_From_FBX(std::wstring file_name, bool merge_mesh, bool add_object, bool merge_object, BYTE info_flag, std::wstring skeleton_name) {
    FBXManager* fbx_manager = FBXManager::Get_Inst();

    fbx_manager->Ipt_From_File(this, file_name, merge_mesh, add_object, merge_mesh, info_flag, skeleton_name);
}

Object* ObjectManager::Add_Obj(std::wstring object_name, std::wstring mesh_name, std::wstring set_name,
    DirectX::XMMATRIX world_matrix,
    D3D12_PRIMITIVE_TOPOLOGY primitive_topology, ObjectType object_type,
    bool physics, bool visiable)
{
    std::unique_ptr<Object> object;
    object = std::make_unique<Object>(this, object_name, m_mesh_manager.Get_Mesh(mesh_name), world_matrix, m_object_count++, primitive_topology, physics, visiable);

    m_object_map[object_name] = std::move(object);

    Object* object_pointer = m_object_map[object_name].get();
    m_objects.emplace_back(object_pointer);

    m_opaque_objects.emplace_back(object_pointer);

    m_object_set_map[set_name].emplace_back(object_pointer);

    return m_object_map[object_name].get();

}

Object* ObjectManager::Add_Obj(std::wstring object_name, std::vector<Mesh>& mesh_array, std::wstring set_name,
    D3D12_PRIMITIVE_TOPOLOGY primitive_topology, ObjectType object_type,
    bool physics, bool visiable)
{
    std::unique_ptr<Object> object;
    object = std::make_unique<Object>(this, object_name, mesh_array, m_object_count++, primitive_topology, physics, visiable);

    m_object_map[object_name] = std::move(object);

    Object* object_pointer = m_object_map[object_name].get();
    m_objects.emplace_back(object_pointer);

    m_opaque_objects.emplace_back(object_pointer);

    m_object_set_map[set_name].emplace_back(object_pointer);

    return m_object_map[object_name].get();
}

void ObjectManager::Build_BV(ID3D12Device* device, ID3D12GraphicsCommandList* command_list) {
    m_mesh_manager.Crt_BV(device, command_list);
}

void ObjectManager::Set_Sklt_2_Obj(std::wstring object_name, std::wstring skeleton_name) {
    Get_Obj(object_name)->Set_Skeleton(m_skeleton_manager.Get_Skeleton(skeleton_name));
}

Object* ObjectManager::Add_Cam(std::wstring camera_name, std::wstring set_name, std::wstring bind_object_name,
    float offset_look, float offset_up, float offset_right, float distance, BYTE flag) {
    std::unique_ptr<Object> object;
    object = std::make_unique<Camera>();

    object->Set_CB_Index(m_object_count++);
    object->Add_Mesh(m_mesh_manager.Get_Mesh(L"default_box"));

    m_object_map[camera_name] = std::move(object);

    Object* object_pointer = m_object_map[camera_name].get();
    m_objects.emplace_back(object_pointer);
    m_camera_objects.emplace_back(object_pointer);

    m_object_set_map[set_name].emplace_back(object_pointer);

    if (bind_object_name != L"") {
        Bind_Cam_2_Obj(camera_name, bind_object_name, offset_look, offset_up, offset_right, distance, flag);
    }

    return object_pointer;
}

Object* ObjectManager::Add_Col_OBB_Obj(std::wstring obb_object_name, DirectX::BoundingOrientedBox obb, std::wstring object_name) {
    std::unique_ptr<Object> object;
    object = std::make_unique<Object>(this, obb_object_name, m_mesh_manager.Get_Mesh(L"boundingbox"),
        DirectX::XMMatrixIdentity(), m_object_count++, D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST, false, true);

    object->Set_OBB(obb);

    m_object_map[obb_object_name] = std::move(object);

    Object* object_pointer = m_object_map[obb_object_name].get();
    m_objects.emplace_back(object_pointer);

    m_collision_obb_objects.emplace_back(object_pointer);

    m_object_set_map[L"BoundingBox"].emplace_back(object_pointer);

    return m_object_map[obb_object_name].get();
}

Object* ObjectManager::Add_Voxel_Cheese(std::wstring object_name, DirectX::XMFLOAT3 object_position, float scale, UINT detail_level) {
    std::unique_ptr<Object> object;
    object = std::make_unique<VoxelCheese>(object_position.x, object_position.y, object_position.z, scale, detail_level);

    object->Set_Name(object_name);
    object->Add_Mesh(m_mesh_manager.Get_Mesh(L"cheese"));
    object->Set_PT(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    object->Set_CB_Index(m_object_count++);
    
    ((InstanceObject*)object.get())->Set_Instance_Index((UINT)m_instance_objects.size());

    m_object_map[object_name] = std::move(object);

    Object* object_pointer = m_object_map[object_name].get();
    m_objects.emplace_back(object_pointer);
    m_instance_objects.emplace_back(object_pointer);
    m_voxel_cheese_objects.emplace_back(object_pointer);

    m_object_set_map[L"Object"].emplace_back(object_pointer);

    return m_object_map[object_name].get();
}

UINT ObjectManager::Get_Max_Instc_Count() {
    UINT max_count = 0;

    for (auto& o : m_instance_objects) {
        max_count = MathHelper::Max(max_count, ((InstanceObject*)o)->Get_Instance_Max_Count());
    }

    return max_count;
}

Object* ObjectManager::Add_Text_UI_Obj(std::wstring object_name, float position_x, float position_y, float scale_x, float scale_y) {
    std::unique_ptr<Object> object;
    object = std::make_unique<TextUIObject>(position_x, position_y, scale_x, scale_y);

    object->Set_Name(object_name);
    object->Add_Mesh(m_mesh_manager.Get_Mesh(L"text"));
    object->Set_PT(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    object->Set_CB_Index(m_object_count++);

    ((InstanceObject*)object.get())->Set_Instance_Index((UINT)(m_instance_objects.size()));

    m_object_map[object_name] = std::move(object);

    Object* object_pointer = m_object_map[object_name].get();
    m_objects.emplace_back(object_pointer);
    m_instance_objects.emplace_back(object_pointer);
    m_text_UI_objects.emplace_back(object_pointer);

    m_object_set_map[L"UI"].emplace_back(object_pointer);

    return m_object_map[object_name].get();

}