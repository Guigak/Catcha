#pragma once
#include "common.h"

struct Channel_Info {
	FMOD::Channel* channel_pointer = nullptr;
	bool linked = false;
	DirectX::XMFLOAT3* channel_position = nullptr;
	DirectX::XMFLOAT3* channel_velocity = nullptr;
};

class SoundManager {
private:
	static std::unique_ptr<SoundManager> sound_manager;

	FMOD::System* m_fmod_system = nullptr;

	std::unordered_map<std::wstring, FMOD::Sound*> m_sound_map;
	std::unordered_map<std::wstring, Channel_Info> m_named_channel_info_array;
	std::vector<Channel_Info> m_channel_info_array;

	bool m_listener_linked = false;

	DirectX::XMFLOAT3* m_listener_position = nullptr;
	DirectX::XMFLOAT3* m_listener_velocity = nullptr;
	DirectX::XMFLOAT3* m_listener_look = nullptr;
	DirectX::XMFLOAT3* m_listener_up = nullptr;

public:
	SoundManager() {}
	~SoundManager() {}

	static SoundManager* Get_Inst(int max_channel = 64, float doppler_scale = 1.0f, float distance_factor = 1.0f, float roll_off_scale = 1.0f);

	void Initialize(int max_channel, float doppler_scale, float distance_factor, float roll_off_scale);

	void Update();

	void Add_Sound(std::wstring file_name, FMOD_MODE mode);

	void Set_Master_Volume(float volume);
	void Set_Channel_Volume(std::wstring channel_name, float volume);

	void Set_Listener(DirectX::XMFLOAT3 position, DirectX::XMFLOAT3 look,
		DirectX::XMFLOAT3 up = { 0.0f,1.0f,0.0f }, DirectX::XMFLOAT3 velocity = { 0.0f, 0.0f, 0.0f });
	void Set_Listener(DirectX::XMFLOAT3* position, DirectX::XMFLOAT3* look, DirectX::XMFLOAT3* up, DirectX::XMFLOAT3* velocity);

	void Play_Sound(std::wstring channel_name, std::wstring file_name, DirectX::XMFLOAT3 position = { 0.0f, 0.0f, 0.0f }, DirectX::XMFLOAT3 velocity = { 0.0f, 0.0f, 0.0f });
	void Play_Sound(std::wstring channel_name, std::wstring file_name, DirectX::XMFLOAT3* position, DirectX::XMFLOAT3* velocity);
};

