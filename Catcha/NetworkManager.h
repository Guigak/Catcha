#pragma once
#include "common.h"
#include "Object.h"
#include "NetworkObserver.h"



// AI�� ���� �� ����
constexpr float FLOOR_Y = -61.6f;

struct Client
{
	int character_id = NUM_GHOST;		// *[ĳ���� ��ȣ]
	DirectX::XMFLOAT3 Location;			// ��ġ ����
	float pitch;						// ȸ�� ��ȭ ����
};

class Object;
class VoxelCheese;
class ParticleObject;
class TextUIObject;

class NetworkManager {
private:
	// socket
	SOCKET m_server_socket;
	SOCKET m_client_socket;
	SOCKET m_udp_socket;
	WSADATA m_wsa_data;
	sockaddr_in m_server_addr;
	sockaddr_in m_udp_addr;

	WSAOVERLAPPED m_overlapped;

	std::string m_name;
	std::string m_password;

	bool is_login = false;

	std::vector<Object*> m_objects;						// *ĳ���� ������Ʈ[character_id]
	std::vector<VoxelCheese*> m_cheeses;
	ParticleObject* m_particle_object;					// *��ƼŬ ������Ʈ
	float* m_total_time = 0;							// ��ƼŬ �ð�

	TextUIObject* m_time_object;						// �ð� ������Ʈ
	short m_game_time = 0;								// ���� �ð�

	// ī�޶� ���ε��� ���� ������Ʈ �Ŵ���
	std::vector<NetworkObserver*> m_observers;

	NetworkManager() {	}
	NetworkManager(const NetworkManager&) = delete;
	NetworkManager& operator=(const NetworkManager&) = delete;

public:
	int m_myid;											// *�ڽ��� ���� ���̵�(�÷��̾� ����) ��ȣ
	std::unordered_map<int, Client> characters;			// [key] *���� ���̵� ��ȣ / [value]ĳ���� ����


	bool Choose = false;								// ĳ���� ���� ���� Ȯ��

	static NetworkManager& GetInstance() 
	{
		static NetworkManager instance;
		return instance;
	}

	// ������� Initialize
	void InitSocket();

	void DoSend(void* packet);
	void SendInput(uint8_t& input_key);
	void SendRotate(float& pitch);
	void SendActionOne(const DirectX::XMFLOAT3& look);
	void ChooseCharacter(bool IsCat);
	void DoSendUDP(void* packet);
	void DoRecv();
	void ProcessData(char* net_buf, size_t io_byte);
	void ProcessPacket(char* packet);

	void ChangeOwnCharacter(int character_id, int new_number);

	void InitialzeCharacters();

	void AddCharacter(Object& object) {
		m_objects.emplace_back(&object);
	}

	void AddCheese(VoxelCheese& cheese) {
		m_cheeses.emplace_back(&cheese);
	}

	void AddParticleObject(ParticleObject& particle_object) {
		m_particle_object = &particle_object;
	}

	void SetTotalTime(float& total_time) {
		m_total_time = &total_time;
	}

	void SetTimeObject(TextUIObject& time) {
		m_time_object = &time;
	}

	// ������ ��� �Լ�
	void RegisterObserver(NetworkObserver* observer) {
		m_observers.push_back(observer);
	}

	// ������ ���� �Լ�
	void UnregisterObserver(NetworkObserver* observer) {
		auto it = std::find(m_observers.begin(), m_observers.end(), observer);
		if (it != m_observers.end())
		{
			m_observers.erase(it);
		}
	}
};
