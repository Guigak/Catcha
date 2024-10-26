#pragma once
#include "common.h"
#include "Object.h"
#include "NetworkObserver.h"


enum class ANIM_TYPE { IDLE };

struct Client
{
	ANIM_TYPE Type;
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

	std::vector<Object*> m_characters;
	// 카메라 바인딩을 위한 오브젝트 매니져
	std::vector<NetworkObserver*> m_observers;

	NetworkManager() {	}
	NetworkManager(const NetworkManager&) = delete;
	NetworkManager& operator=(const NetworkManager&) = delete;

public:
	int m_myid;
	std::unordered_map<int, Client> characters;

	static NetworkManager& GetInstance() 
	{
		static NetworkManager instance;
		return instance;
	}

	// 구성요소 Initialize
	void InitSocket();

	void DoSend(void* packet);
	void SendInput(uint8_t& input_key);
	void SendRotate(float& pitch);
	void ChooseCharacter(bool IsCat);
	void DoSendUDP(void* packet);
	void DoRecv();
	void ProcessData(char* net_buf, size_t io_byte);
	void ProcessPacket(char* packet);

	void AddCharacter(Object& object) {
		m_characters.emplace_back(&object);
	}

	// 옵저버 등록 함수
	void RegisterObserver(NetworkObserver* observer) {
		m_observers.push_back(observer);
	}

	// 옵저버 제거 함수
	void UnregisterObserver(NetworkObserver* observer) {
		auto it = std::find(m_observers.begin(), m_observers.end(), observer);
		if (it != m_observers.end())
		{
			m_observers.erase(it);
		}
	}
};
