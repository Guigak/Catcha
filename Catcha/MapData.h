#pragma once
#include "common.h"

struct ObjectOBB
{
    std::wstring name;
    DirectX::BoundingOrientedBox obb;
    bool is_spring_arm;
};

extern std::unordered_map<std::wstring, ObjectOBB> g_obbData;
extern std::vector<DirectX::BoundingOrientedBox> g_springArmObj;

class MapData {
public:
    // �� ������ �ε�
    bool LoadMapData(const std::string& filePath);

    // ���� Vector3 �Ľ�
    void ParseVector3(const std::string& values, DirectX::XMFLOAT3& vector);
    // ���� Vector4 �Ľ�
    void ParseVector4(const std::string& values, DirectX::XMFLOAT4& vector);
    // ���� ���� �Լ�
    std::string Trim(const std::string& str);
};

