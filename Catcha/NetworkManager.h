#pragma once
#include "common.h"
#include "Object.h"
#include "NetworkObserver.h"


enum class ANIM_TYPE { IDLE };

struct Client
{
	ANIM_TYPE Type;
	int character_id = NUM_GHOST;		// ĳ���� ��ȣ
	DirectX::XMFLOAT3 Location;
	float pitch;
};

class Object;

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

	std::vector<Object*> m_objects;


	// ī�޶� ���ε��� ���� ������Ʈ �Ŵ���
	std::vector<NetworkObserver*> m_observers;

	NetworkManager() {	}
	NetworkManager(const NetworkManager&) = delete;
	NetworkManager& operator=(const NetworkManager&) = delete;

public:
	int m_myid;											// �ڽ��� ���� ���̵� ��ȣ
	std::unordered_map<int, Client> characters;			// [key] ���� ���̵� ��ȣ / [value]ĳ���� ����

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
	void ChooseCharacter(bool IsCat);
	void DoSendUDP(void* packet);
	void DoRecv();
	void ProcessData(char* net_buf, size_t io_byte);
	void ProcessPacket(char* packet);

	void ChangeOwnCharacter(int character_id, int new_number);

	void AddCharacter(Object& object) {
		m_objects.emplace_back(&object);
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
