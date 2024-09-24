#pragma once
#include "common.h"


class NetworkManager {
private:
	// socket
	SOCKET m_server_socket;
	SOCKET m_client_socket;
	SOCKET m_udp_socket;
	WSADATA m_wsa_data;
	sockaddr_in m_server_addr;
	sockaddr_in m_client_addr;

	WSAOVERLAPPED m_overlapped;

	std::string m_name;

public:
	// 구성요소 Initialize
	void InitSocket();

	void DoSend(void* packet);
	void DoRecv();
	void ProcessData(char* net_buf, size_t io_byte);
};
