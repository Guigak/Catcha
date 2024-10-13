#pragma once
#include "common.h"


enum class ANIM_TYPE { IDLE };

struct Client
{
	ANIM_TYPE Type;
	DirectX::XMFLOAT3 Location;
	float pitch;
};


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
	void SendRotate(short& pitch);
	void DoSendUDP(void* packet);
	void DoRecv();
	void ProcessData(char* net_buf, size_t io_byte);
	void ProcessPacket(char* packet);
};
