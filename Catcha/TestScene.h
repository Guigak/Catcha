#pragma once
#include "common.h"
#include "Scene.h"

//
enum class Scene_State {
	MAIN_STATE,
	MATCHING_STATE,
	PLAY_STATE,
	END_STATE
};

class TestScene : public Scene {
public:
	TestScene(std::wstring name, Scene* back_scene = nullptr) : Scene(name, back_scene) {}

	virtual void Enter(D3DManager* d3d_manager);
	virtual void Exit(D3DManager* d3d_manager);

	virtual void Update(D3DManager* d3d_manager, float elapsed_time);
	virtual void Resize(D3DManager* d3d_manager);
	virtual void Draw(D3DManager* d3d_manager, ID3D12CommandList** command_lists);

	virtual void Prcs_Input_Msg(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);	// Process Message

	virtual void Load_Texture(ID3D12Device* device, ID3D12GraphicsCommandList* command_list);

	virtual void Build_RS(ID3D12Device* device);	// Build Root Signature
	virtual void Build_S_N_L();	// Build Shaders And Layouts
	virtual void Build_Mesh(ID3D12Device* device, ID3D12GraphicsCommandList* command_list);	// Build Meshs
	virtual void Build_Material();	// Build Materials
	virtual void Build_O();	// Build Objects
	virtual void Build_C(D3DManager* d3d_manager);	// Build Cameras
	virtual void Build_FR(ID3D12Device* device);	// Build FrameResources
	virtual void Build_DH(ID3D12Device* device);	// Build Descriptor Heaps
	virtual void Build_CBV(D3DManager* d3d_manager);	// Build Constant Buffer Views
	virtual void Build_PSO(D3DManager* d3d_manager);	// Build Pipeline State Objects

	virtual void Binding_Key();
	virtual void Pairing_Collision_Set();

	//
	virtual void Custom_Function_One();
	virtual void Custom_Function_Two();
	virtual void Custom_Function_Three();

	//
	void Crt_Voxel(DirectX::XMFLOAT3 position, float scale, UINT detail_level);
	void Crt_Voxel_Cheese(DirectX::XMFLOAT3 position, float scale, UINT detail_level);

	//
	void Del_Voxel(int cheese_index, int voxel_index);

	//
	void Chg_Scene_State(Scene_State scene_state);

	//
	virtual void Picking(POINTF screen_position);

	void CharacterChange(bool is_cat, const std::wstring& key1, const std::wstring& key2) override;		// 옵저버를 이용한 캐릭터 변경
	void InitCamera(DirectX::XMFLOAT4 rotate_quat = DirectX::XMFLOAT4{ 0.0f, 0.0f, 0.0f, 1.0f }) override;	// 접속시 카메라 값 초기화
	void OpenDoorEvent() override;	// 옵저버를 이용한 문 여는 이벤트 실행
	// 카메라 래깅 시간차 설정을 위한 함수 (카메라 오브젝트 바인딩시 보간되어 이동하도록)
	void SetCameraLagging(float target_lagging, float time) override;
	// 탈출시 키 재바인딩 및 카메라 설정 
	void SetEscape() override;
	// 죽거나 탈출 후 관전 모드
	void ObservingMode() override;
	// 게임 결과창 띄우기
	void ShowingResultScene(bool is_cat_winner) override;
};

