#pragma once
#include "Scene.h"

class DummyScene : public Scene {
public:
	DummyScene(std::wstring name, Scene* back_scene = nullptr);

	virtual void Enter(D3DManager* d3d_manager);
	virtual void Pause();
	virtual void Resume();
	virtual void Exit(D3DManager* d3d_manager);

	virtual void Update(D3DManager* d3d_manager);
	virtual void Draw(D3DManager* d3d_manager, ID3D12CommandList** command_lists);

	virtual void Prcs_Input(UINT message, WPARAM wparam, LPARAM lparam);	// Process Message

	void CharacterChange(bool is_cat, const std::wstring& key1, const std::wstring& key2) {}
	// 접속시 카메라 값 초기화
	void InitCamera(DirectX::XMFLOAT4 rotate_quat = DirectX::XMFLOAT4{ 0.0f, 0.0f, 0.0f, 1.0f }) {}

	void OpenDoorEvent() {};
	// 카메라 래깅 시간차 설정을 위한 함수 (카메라 오브젝트 바인딩시 보간되어 이동하도록)
	void SetCameraLagging(float target_lagging, float time) {}

	// 탈출시 키 재바인딩 및 카메라 설정 
	void SetEscape() {}

	// 죽거나 탈출 후 관전 모드
	void ObservingMode() {}

	// 게임 결과창 띄우기
	void ShowingResultScene(bool is_cat_winner) {}

	// 환생 ui 이벤트
	void RebornUICount() {}

	// 타격시 효과
	void AttackedUI() {}

	// 고양이 종소리 타이머
	void ActiveRingingBell() {}
};

