#pragma once

// NetworkManager와 Scene이 서로 통신하기 위한 Observer Pattern
class NetworkObserver {
public:
    virtual ~NetworkObserver() = default;

    // 접속시 카메라 값 초기화
    virtual void InitCamera(DirectX::XMFLOAT4 rotate_quat = DirectX::XMFLOAT4{ 0.0f, 0.0f, 0.0f, 1.0f }) = 0;

    // 캐릭터가 변경되었을 때 호출될 함수
    virtual void CharacterChange(bool is_cat, const std::wstring& key1, const std::wstring& key2) = 0;

    // 문열림 이벤트 발생시 호출될 함수
    virtual void OpenDoorEvent() = 0;

    // 카메라 래깅 시간차 설정을 위한 함수 (카메라 오브젝트 바인딩시 보간되어 이동하도록)
    virtual void SetCameraLagging(float target_lagging, float time) = 0;

    // 탈출시 키 재바인딩 및 카메라 설정 
    virtual void SetEscape() = 0;

    // 죽거나 탈출 후 관전 모드
    virtual void ObservingMode() = 0;
};