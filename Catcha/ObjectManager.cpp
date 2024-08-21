#include "ObjectManager.h"
#include "InputManager.h"

void ObjectManager::Add_Obj(std::wstring object_name, MeshInfo* mesh_info, std::wstring mesh_name, MaterialInfo* material_info, D3D12_PRIMITIVE_TOPOLOGY primitive_topology, bool opaque_object) {
    auto object = std::make_unique<Object>(object_name, mesh_info, mesh_name, material_info, m_object_count++, primitive_topology);

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

void ObjectManager::Update() {
    for (auto& o : m_objects) {
        o->Update();
    }
}
