#pragma once

// NetworkManager�� Scene�� ���� ����ϱ� ���� Observer Pattern
class NetworkObserver {
public:
    virtual ~NetworkObserver() = default;

    // ���ӽ� ī�޶� �� �ʱ�ȭ
    virtual void InitCamera(DirectX::XMFLOAT4 rotate_quat = DirectX::XMFLOAT4{ 0.0f, 0.0f, 0.0f, 1.0f }) = 0;

    // ĳ���Ͱ� ����Ǿ��� �� ȣ��� �Լ�
    virtual void CharacterChange(bool is_cat, const std::wstring& key1, const std::wstring& key2) = 0;

    // ������ �̺�Ʈ �߻��� ȣ��� �Լ�
    virtual void OpenDoorEvent() = 0;

    // ī�޶� ���� �ð��� ������ ���� �Լ� (ī�޶� ������Ʈ ���ε��� �����Ǿ� �̵��ϵ���)
    virtual void SetCameraLagging(float target_lagging, float time) = 0;

    // Ż��� Ű ����ε� �� ī�޶� ���� 
    virtual void SetEscape() = 0;

    // �װų� Ż�� �� ���� ���
    virtual void ObservingMode() = 0;
};