#pragma once

// NetworkManager�� Scene�� ���� ����ϱ� ���� Observer Pattern
class NetworkObserver {
public:
    virtual ~NetworkObserver() = default;

    // ĳ���Ͱ� ����Ǿ��� �� ȣ��� �Լ�
    virtual void CharacterChange(bool is_cat, const std::wstring& key1, const std::wstring& key2) = 0;
};