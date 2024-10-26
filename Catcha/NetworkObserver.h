#pragma once

// NetworkManager와 Scene이 서로 통신하기 위한 Observer Pattern
class NetworkObserver {
public:
    virtual ~NetworkObserver() = default;

    // 캐릭터가 변경되었을 때 호출될 함수
    virtual void CharacterChange(bool is_cat, const std::wstring& key1, const std::wstring& key2) = 0;
};