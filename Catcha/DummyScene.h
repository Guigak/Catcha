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
	// ���ӽ� ī�޶� �� �ʱ�ȭ
	void InitCamera(DirectX::XMFLOAT4 rotate_quat = DirectX::XMFLOAT4{ 0.0f, 0.0f, 0.0f, 1.0f }) {}

	void OpenDoorEvent() {};
	// ī�޶� ���� �ð��� ������ ���� �Լ� (ī�޶� ������Ʈ ���ε��� �����Ǿ� �̵��ϵ���)
	void SetCameraLagging(float target_lagging, float time) {}

	// Ż��� Ű ����ε� �� ī�޶� ���� 
	void SetEscape() {}

	// �װų� Ż�� �� ���� ���
	void ObservingMode() {}

	// ���� ���â ����
	void ShowingResultScene(bool is_cat_winner) {}

	// ȯ�� ui �̺�Ʈ
	void RebornUICount() {}

	// Ÿ�ݽ� ȿ��
	void AttackedUI() {}

	// ����� ���Ҹ� Ÿ�̸�
	void ActiveRingingBell() {}
};

