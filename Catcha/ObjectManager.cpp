#include "ObjectManager.h"
#include "InputManager.h"
#include "Camera.h"

Object* ObjectManager::Add_Obj(std::wstring object_name, MeshInfo* mesh_info, std::wstring mesh_name, MaterialInfo* material_info,
    D3D12_PRIMITIVE_TOPOLOGY primitive_topology, ObjectType object_type, bool physics, bool visiable, std::wstring set_name)
{
    //auto object = std::make_unique<Object>(object_name, mesh_info, mesh_name, material_info, m_object_count++, primitive_topology, physics);

    std::unique_ptr<Object> object;

    switch (object_type) {
    case ObjectType::OPAQUE_OBJECT:
    case ObjectType::TRANSPARENT_OBJECT:
        object = std::make_unique<Object>(object_name, mesh_info, mesh_name, material_info, m_object_count++, primitive_topology, physics, visiable);
        break;
    case ObjectType::CAMERA_OBJECT:
        object = std::make_unique<Camera>();
        break;
    default:
        break;
    }

    m_object_map[object_name] = std::move(object);

    Object* object_pointer = m_object_map[object_name].get();
    
    m_objects.emplace_back(object_pointer);

    switch (object_type) {
    case ObjectType::OPAQUE_OBJECT:
        m_opaque_objects.emplace_back(object_pointer);
        break;
    case ObjectType::TRANSPARENT_OBJECT:
        m_transparent_objects.emplace_back(object_pointer);
        break;
    case ObjectType::CAMERA_OBJECT:
        m_camera_objects.emplace_back(object_pointer);
        break;
    default:
        break;
    }

    m_object_set_map[set_name].emplace_back(object_pointer);

    return m_object_map[object_name].get();
}

Object* ObjectManager::Get_Obj(std::wstring object_name) {
    return m_object_map[object_name].get();
}

Object* ObjectManager::Get_Obj(UINT object_number) {
    return m_objects[object_number];
}

// 플레이어 추가로 인한 getter
Object* ObjectManager::Get_Obj_Character(UINT object_number)
{
    return m_characters[object_number];
}

Object* ObjectManager::Get_Opaque_Obj(UINT object_number) {
    return m_opaque_objects[object_number];
}

Object* ObjectManager::Get_Transparent_Obj(UINT object_number) {
    return m_transparent_objects[object_number];
}

void ObjectManager::Move(std::wstring object_name, Action action) {
    Object* object = Get_Obj(object_name);

    switch (action) {
    case Action::MOVE_FORWARD:
        object->Move_Forward();
        break;
    case Action::MOVE_BACK:
        object->Move_Back();
        break;
    case Action::MOVE_LEFT:
        object->Move_Left();
        break;
    case Action::MOVE_RIGHT:
        object->Move_Right();
        break;
    case Action::MOVE_UP:
        object->Move_Up();
        break;
    case Action::MOVE_DOWN:
        object->Move_Down();
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

void ObjectManager::Rotate(std::wstring object_name, Action action, POINTF degree) {
    Object* object = Get_Obj(object_name);

    switch (action) {
    case Action::ROTATE:
        object->Rotate(degree.y, degree.x, 0.0f);
        break;
    case Action::ROTATE_ROLL:
        object->Rotate_Roll(degree.x);
        break;
    case Action::ROTATE_PITCH:
        object->Rotate_Pitch(degree.x);
        break;
    case Action::ROTATE_YAW:
        object->Rotate_Yaw(degree.x);
        break;
    default:
        break;
    }
}

void ObjectManager::Update(float elapsed_time) {
    for (auto& o : m_objects) {
        o->Calc_Delta(elapsed_time);
    }
    // player에 대한 calc_delta 따로 실시
    for (auto& o : m_characters)
    {
        o->Calc_Delta_Characters(elapsed_time);
        o->Rotate_Character(elapsed_time);
    }

    Solve_Collision();

    for (auto& o : m_objects) {
        o->Update();
    }

    for (auto& o : m_characters)
    {
        o->Update();
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

void ObjectManager::Bind_Cam_2_Obj(std::wstring camera_name, std::wstring object_name, float distance) {
    Camera* camera = (Camera*)Get_Obj(camera_name);
    Object* object = Get_Obj(object_name);

    object->Bind_Camera(camera);
    camera->Bind_Obj(object, distance);
}

void ObjectManager::Ipt_From_FBX(std::wstring file_name, bool merge_mesh, bool add_object, bool merge_object, BYTE info_flag) {
    FBXManager* fbx_manager = FBXManager::Get_Inst();

    fbx_manager->Ipt_From_File(this, file_name, merge_mesh, add_object, merge_mesh, info_flag);
}

Object* ObjectManager::Add_Obj(std::wstring object_name, std::wstring mesh_name, std::wstring set_name,
    DirectX::XMMATRIX world_matrix,
    D3D12_PRIMITIVE_TOPOLOGY primitive_topology, ObjectType object_type,
    bool physics, bool visiable)
{
    std::unique_ptr<Object> object;
    object = std::make_unique<Object>(object_name, m_mesh_manager.Get_Mesh(mesh_name), world_matrix, m_object_count++, primitive_topology, physics, visiable);

    m_object_map[object_name] = std::move(object);

    Object* object_pointer = m_object_map[object_name].get();

    // [CS] 서버 캐릭터 구분
    switch (object_type)
    {
    case ObjectType::CHARACTER_OBJECT:
        m_characters.emplace_back(object_pointer);
        break;
    }

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
    object = std::make_unique<Object>(object_name, mesh_array, m_object_count++, primitive_topology, physics, visiable);

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