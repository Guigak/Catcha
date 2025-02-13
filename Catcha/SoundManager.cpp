#include "SoundManager.h"

std::unique_ptr<SoundManager> SoundManager::sound_manager = nullptr;

SoundManager* SoundManager::Get_Inst(int max_channel, float doppler_scale, float distance_factor, float roll_off_scale) {
	if (!sound_manager) {
		sound_manager = std::make_unique<SoundManager>();
		sound_manager->Initialize(max_channel, doppler_scale, distance_factor, roll_off_scale);
	}

	return sound_manager.get();
}

void SoundManager::Initialize(int max_channel, float doppler_scale, float distance_factor, float roll_off_scale) {
	FMOD::System_Create(&m_fmod_system);
	m_fmod_system->init(max_channel, FMOD_INIT_NORMAL, nullptr);
	m_fmod_system->set3DSettings(doppler_scale, distance_factor, roll_off_scale);
}

void SoundManager::Update() {
	//
	if (m_listener_linked) {
		FMOD_VECTOR listener_position = { 0.0f, 0.0f, 0.0f };
		FMOD_VECTOR listener_velocity = { 0.0f, 0.0f, 0.0f };
		FMOD_VECTOR listener_look = { 0.0f, 0.0f, 0.0f };
		FMOD_VECTOR listener_up = { 0.0f, 0.0f, 0.0f };

		if (m_listener_position != nullptr) {
			listener_position = { m_listener_position->x, m_listener_position->y, m_listener_position->z };
		}
		if (m_listener_velocity != nullptr) {
			listener_velocity = { m_listener_velocity->x, m_listener_velocity->y, m_listener_velocity->z };
		}
		if (m_listener_look != nullptr) {
			listener_look = { m_listener_look->x, m_listener_look->y, m_listener_look->z };
		}
		if (m_listener_up != nullptr) {
			listener_up = { m_listener_up->x, m_listener_up->y, m_listener_up->z };
		}

		m_fmod_system->set3DListenerAttributes(0, &listener_position, &listener_velocity, &listener_look, &listener_up);
	}

	//
	auto target_iterator = std::remove_if(m_channel_info_array.begin(), m_channel_info_array.end(),
		[](const Channel_Info& data) {
			bool playing = false;
			data.channel_pointer->isPlaying(&playing);

			return playing == false;
		});

	m_channel_info_array.erase(target_iterator, m_channel_info_array.end());

	//
	for (auto& c : m_named_channel_info_array) {
		if (c.second.linked) {
			FMOD_VECTOR sound_position = { 0.0f, 0.0f, 0.0f };
			FMOD_VECTOR sound_velocity = { 0.0f, 0.0f, 0.0f };

			if (c.second.channel_position != nullptr) {
				sound_position = { c.second.channel_position->x, c.second.channel_position->y, c.second.channel_position->z };
			}
			if (c.second.channel_velocity != nullptr) {
				sound_velocity = { c.second.channel_velocity->x, c.second.channel_velocity->y, c.second.channel_velocity->z };
			}

			c.second.channel_pointer->set3DAttributes(&sound_position, &sound_velocity);
		}
	}

	for (auto& c : m_channel_info_array) {
		if (c.linked) {
			FMOD_VECTOR sound_position = { 0.0f, 0.0f, 0.0f };
			FMOD_VECTOR sound_velocity = { 0.0f, 0.0f, 0.0f };

			if (c.channel_position != nullptr) {
				sound_position = { c.channel_position->x, c.channel_position->y, c.channel_position->z };
			}
			if (c.channel_velocity != nullptr) {
				sound_velocity = { c.channel_velocity->x, c.channel_velocity->y, c.channel_velocity->z };
			}

			c.channel_pointer->set3DAttributes(&sound_position, &sound_velocity);
		}
	}

	m_fmod_system->update();
}

void SoundManager::Add_Sound(std::wstring file_name, FMOD_MODE mode) {
	FMOD::Sound* sound = nullptr;
	m_fmod_system->createSound(WStr_2_Str(file_name).c_str(), mode, nullptr, &sound);

	m_sound_map[file_name] = sound;
}

void SoundManager::Set_Master_Volume(float volume) {
	FMOD::ChannelGroup* master_group = nullptr;
	m_fmod_system->getMasterChannelGroup(&master_group);

	master_group->setVolume(volume);
}

void SoundManager::Set_Channel_Volume(std::wstring channel_name, float volume) {
	m_named_channel_info_array[channel_name].channel_pointer->setVolume(volume);
}

void SoundManager::Set_Channel_Paused(std::wstring channel_name, bool paused) {
	m_named_channel_info_array[channel_name].channel_pointer->setPaused(paused);
}

void SoundManager::Restart_Channel(std::wstring channel_name) {
	m_named_channel_info_array[channel_name].channel_pointer->setPosition(0, FMOD_TIMEUNIT_MS);
	m_named_channel_info_array[channel_name].channel_pointer->setPaused(false);
}

void SoundManager::Set_Listener(DirectX::XMFLOAT3 position, DirectX::XMFLOAT3 look, DirectX::XMFLOAT3 up, DirectX::XMFLOAT3 velocity) {
	m_listener_linked = false;

	FMOD_VECTOR listener_position = { position.x, position.y, position.z };
	FMOD_VECTOR listener_velocity = { velocity.x, velocity.y, velocity.z };
	FMOD_VECTOR listener_look = { look.x, look.y, look.z };
	FMOD_VECTOR listener_up = { up.x, up.y, up.z };

	m_fmod_system->set3DListenerAttributes(0, &listener_position, &listener_velocity, &listener_look, &listener_up);
}

void SoundManager::Set_Listener(DirectX::XMFLOAT3* position, DirectX::XMFLOAT3* look, DirectX::XMFLOAT3* up, DirectX::XMFLOAT3* velocity) {
	m_listener_linked = true;

	m_listener_position = position;
	m_listener_velocity = velocity;
	m_listener_look = look;
	m_listener_up = up;

	FMOD_VECTOR listener_position = { 0.0f, 0.0f, 0.0f };
	FMOD_VECTOR listener_velocity = { 0.0f, 0.0f, 0.0f };
	FMOD_VECTOR listener_look = { 0.0f, 0.0f, 0.0f };
	FMOD_VECTOR listener_up = { 0.0f, 0.0f, 0.0f };

	if (m_listener_position != nullptr) {
		listener_position = { m_listener_position->x, m_listener_position->y, m_listener_position->z };
	}
	if (m_listener_velocity != nullptr) {
		listener_velocity = { m_listener_velocity->x, m_listener_velocity->y, m_listener_velocity->z };
	}
	if (m_listener_look != nullptr) {
		listener_look = { m_listener_look->x, m_listener_look->y, m_listener_look->z };
	}
	if (m_listener_up != nullptr) {
		listener_up = { m_listener_up->x, m_listener_up->y, m_listener_up->z };
	}

	m_fmod_system->set3DListenerAttributes(0, &listener_position, &listener_velocity, &listener_look, &listener_up);
}

void SoundManager::Play_Sound(std::wstring channel_name, std::wstring file_name, DirectX::XMFLOAT3 position, DirectX::XMFLOAT3 velocity, bool sound_override) {
	FMOD_VECTOR sound_position = { position.x, position.y, position.z };
	FMOD_VECTOR sound_velocity = { velocity.x, velocity.y, velocity.z };

	FMOD::Channel* channel = nullptr;

	if (channel_name == L"") {
		m_fmod_system->playSound(m_sound_map[file_name], nullptr, false, &channel);
		channel->set3DAttributes(&sound_position, &sound_velocity);

		m_channel_info_array.emplace_back(channel, false, nullptr, nullptr);
	}
	else {
		bool playing = false;
		FMOD::Channel* channel_pointer = nullptr;

		if (m_named_channel_info_array.count(channel_name)) {
			channel_pointer = m_named_channel_info_array[channel_name].channel_pointer;
			channel_pointer->isPlaying(&playing);
		}

		if (playing) {
			Channel_Info channel_info = m_named_channel_info_array[channel_name];
			channel_info.channel_position = nullptr;
			channel_info.channel_velocity = nullptr;
			channel_info.linked = false;

			if (sound_override) {
				Restart_Channel(channel_name);
			}
			else {
				return;
			}
		}
		else {
			m_fmod_system->playSound(m_sound_map[file_name], nullptr, false, &channel);
			channel->set3DAttributes(&sound_position, &sound_velocity);

			m_named_channel_info_array[channel_name] = Channel_Info(channel, false, nullptr, nullptr);
		}
	}
}

void SoundManager::Play_Sound(std::wstring channel_name, std::wstring file_name, DirectX::XMFLOAT3* position, DirectX::XMFLOAT3* velocity, bool sound_override) {
	FMOD_VECTOR sound_position = { 0.0f, 0.0f, 0.0f };
	FMOD_VECTOR sound_velocity = { 0.0f, 0.0f, 0.0f };

	if (position != nullptr) {
		sound_position = { position->x, position->y, position->z };
	}
	if (velocity != nullptr) {
		sound_velocity = { velocity->x, velocity->y, velocity->z };
	}

	FMOD::Channel* channel = nullptr;

	if (channel_name == L"") {
		m_fmod_system->playSound(m_sound_map[file_name], nullptr, false, &channel);
		channel->set3DAttributes(&sound_position, &sound_velocity);

		m_channel_info_array.emplace_back(channel, true, position, velocity);
	}
	else {
		bool playing = false;
		FMOD::Channel* channel_pointer = nullptr;

		if (m_named_channel_info_array.count(channel_name)) {
			channel_pointer = m_named_channel_info_array[channel_name].channel_pointer;
			channel_pointer->isPlaying(&playing);
		}

		if (playing) {
			Channel_Info channel_info = m_named_channel_info_array[channel_name];
			channel_info.channel_position = position;
			channel_info.channel_velocity = velocity;
			channel_info.linked = true;

			if (sound_override) {
				Restart_Channel(channel_name);
			}
			else {
				return;
			}
		}
		else {
			m_fmod_system->playSound(m_sound_map[file_name], nullptr, false, &channel);
			channel->set3DAttributes(&sound_position, &sound_velocity);

			m_named_channel_info_array[channel_name] = Channel_Info(channel, true, position, velocity);
		}
	}
}
