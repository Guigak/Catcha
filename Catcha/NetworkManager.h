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

	void AddCharacter(Object& object) {
		m_objects.emplace_back(&object);
	}

	void AddCheese(VoxelCheese& cheese) {
		m_cheeses.emplace_back(&cheese);
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
