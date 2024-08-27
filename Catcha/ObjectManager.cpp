#include "ObjectManager.h"
#include "InputManager.h"

void ObjectManager::Add_Obj(std::wstring object_name, MeshInfo* mesh_info, std::wstring mesh_name, MaterialInfo* material_info,
    D3D12_PRIMITIVE_TOPOLOGY primitive_topology, bool opaque_object, bool physics)
{
    auto object = std::make_unique<Object>(object_name, mesh_info, mesh_name, material_info, m_object_count++, primitive_topology, physics);

    m_object_map[object_name] = std::move(object);
    m_objects.emplace_back(m_object_map[object_name].get());

    if (opaque_object) {
        m_opaque_objects.emplace_back(m_object_map[object_name].get());
    }
    else {
        m_transparent_objects.emplace_back(m_object_map[object_name].get());
    }
}

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

void ObjectManager::Teleport(std::wstring object_name, Action action) {
    Object* object = Get_Obj(object_name);

    switch (action) {
    case Action::TELEPORT_FORWARD:
        object->TP_Forward();
        break;
    case Action::TELEPORT_BACK:
        object->TP_Back();
        break;
    case Action::TELEPORT_LEFT:
        object->TP_Left();
        break;
    case Action::TELEPORT_RIGHT:
        object->TP_Right();
        break;
    case Action::TELEPORT_UP:
        object->TP_Up();
        break;
    case Action::TELEPORT_DOWN:
        object->TP_Down();
        break;
    default:
        break;
    }
}

void ObjectManager::Update(float elapsed_time) {
    for (auto& o : m_objects) {
        o->Calc_Delta(elapsed_time);
    }

    Solve_Collision();

    for (auto& o : m_objects) {
        o->Udt_WM();
    }
}

void ObjectManager::Solve_Collision() {
}
