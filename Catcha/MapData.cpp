#include "MapData.h"


bool MapData::LoadMapData(const std::string& filePath)
{
    std::ifstream file(filePath);
    if (!file.is_open())
    {
        std::cout << "Failed to open file: " << filePath << std::endl;
        return false;
    }

    std::string line;
    ObjectOBB currentOBB;
    DirectX::XMFLOAT3 position = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
    DirectX::XMFLOAT3 extents = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
    DirectX::XMFLOAT4 rotation = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);

    while (std::getline(file, line))
    {
        if (line.empty())
        {
            currentOBB.obb = DirectX::BoundingOrientedBox(position, extents, rotation);
            // 카메라가 충돌해야 하는 오브젝트 분리 저장
            if (true == currentOBB.is_spring_arm)
            {
                g_springArmObj.emplace_back(DirectX::BoundingOrientedBox(position, extents, rotation));
            }
            g_obbData.emplace(currentOBB.name, std::move(currentOBB));
            currentOBB = ObjectOBB();  // 다음 객체를 위해 초기화
            continue;
        }

        // "Object Name" 파싱
        if (line.find("Object Name:") != std::string::npos)
        {
            size_t pos = line.find(':');
            std::string name = line.substr(pos + 1);
            name.erase(0, name.find_first_not_of(" \t"));
            currentOBB.name = std::wstring(name.begin(), name.end());
        }
        // "Position" 파싱
        else if (line.find("Position:") != std::string::npos)
        {
            size_t pos = line.find(':');
            std::string values = line.substr(pos + 1);
            ParseVector3(values, position);
        }
        // "Rotation" 파싱
        else if (line.find("Rotation:") != std::string::npos)
        {
            size_t pos = line.find(':');
            std::string values = line.substr(pos + 1);
            ParseVector4(values, rotation);
        }
        // "Extents" 파싱
        else if (line.find("Extents:") != std::string::npos)
        {
            size_t pos = line.find(':');
            std::string values = line.substr(pos + 1);
            ParseVector3(values, extents);
        }
        // "Tag" 파싱
        else if (line.find("Tag:") != std::string::npos)
        {
            size_t pos = line.find(':');
            std::string tag = line.substr(pos + 1);
            tag.erase(0, tag.find_first_not_of(" \t"));
            // springArm 이면 true
            currentOBB.is_spring_arm = (tag.find("SpringArm") != std::string::npos) ? true : false;
        }
    }

    // 마지막 객체 추가
    if (!currentOBB.name.empty())
    {
        currentOBB.obb = DirectX::BoundingOrientedBox(position, extents, rotation);
        // 카메라가 충돌해야 하는 오브젝트 분리 저장
        if (true == currentOBB.is_spring_arm)
        {
            g_springArmObj.emplace_back(DirectX::BoundingOrientedBox(position, extents, rotation));
        }
        g_obbData.emplace(currentOBB.name, std::move(currentOBB));
    }

    file.close();
    return true;
}

// 라인 Vector3 파싱
void MapData::ParseVector3(const std::string& values, DirectX::XMFLOAT3& vector)
{
    std::istringstream iss(values);
    std::string token;

    std::getline(iss, token, ',');
    vector.x = std::stof(Trim(token));

    std::getline(iss, token, ',');
    vector.y = std::stof(Trim(token));

    std::getline(iss, token, ',');
    vector.z = std::stof(Trim(token));
}
// 라인 Vector4 파싱
void MapData::ParseVector4(const std::string& values, DirectX::XMFLOAT4& vector)
{
    std::istringstream iss(values);
    std::string token;

    std::getline(iss, token, ',');
    vector.x = std::stof(Trim(token));

    std::getline(iss, token, ',');
    vector.y = std::stof(Trim(token));

    std::getline(iss, token, ',');
    vector.z = std::stof(Trim(token));

    std::getline(iss, token, ',');
    vector.w = std::stof(Trim(token));
}

// 공백 제거 함수
std::string MapData::Trim(const std::string& str)
{
    const char* whitespace = " \t";
    size_t start = str.find_first_not_of(whitespace);
    size_t end = str.find_last_not_of(whitespace);
    return start == std::string::npos ? "" : str.substr(start, end - start + 1);
}
