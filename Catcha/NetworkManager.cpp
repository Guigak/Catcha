#include "NetworkManager.h"

void print_error(const char* msg, int err_no)
{
	WCHAR* msg_buf;
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, err_no,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		reinterpret_cast<LPWSTR>(&msg_buf), 0, NULL);
	std::cout << msg;
	std::wcout << L" : 에러 : " << msg_buf;
	while (true);
	LocalFree(msg_buf);
}

void NetworkManager::InitSocket()
{
	std::wcout.imbue(std::locale("korean"));
	WSADATA WSAData;
	int res = WSAStartup(MAKEWORD(2, 2), &WSAData);
	if (0 != res)
	{
		print_error("WSAStartup", WSAGetLastError());
	}
	m_server_socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, nullptr, 0, WSA_FLAG_OVERLAPPED);
	m_server_addr.sin_family = AF_INET;
	m_server_addr.sin_port = htons(PORT);

	// TODO : 서버 아이피로 변경
#ifdef ENTERIP
	char SERVER_ADDR[BUFSIZE];
	std::cout << "\nEnter IP Address : ";
	std::cin.getline(SERVER_ADDR, BUFSIZE);
#endif
	char SERVER_ADDR[BUFSIZE] = "127.0.0.1";


	std::cout << "Enter User Name : ";
	//std::cin >> m_name;
	m_name = "Temp01";

	inet_pton(AF_INET, SERVER_ADDR, &m_server_addr.sin_addr);
	res = connect(m_server_socket, reinterpret_cast<sockaddr*>(&m_server_addr), sizeof(m_server_addr));
	if (SOCKET_ERROR == res)
	{
		print_error("Connect", WSAGetLastError());
		WSACleanup();
		exit(0);
	}

#ifdef NAGLEOFF
	int DelayZero = 1;
	setsockopt(m_server_socket, SQL_SOCKET, TCP_NODELAY, (const char*)&DelayZero, sizeof(DelayZero));
#endif

	// TCP NonBlocking으로 설정
	u_long noblock = 1;
	res = ioctlsocket(m_server_socket, FIONBIO, &noblock);

	// TODO : 로그인 패킷 특정 상황에 보내도록 옮기기
	CS_LOGIN_PACKET p;
	p.size = sizeof(p);
	p.type = CS_LOGIN;
	strcpy_s(p.name, m_name.c_str());
	DoSend(&p);

	// UDP 소켓 Initialize
	m_udp_socket = WSASocket(AF_INET, SOCK_DGRAM, IPPROTO_UDP, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (m_udp_socket == INVALID_SOCKET)
	{
		print_error("UDP socket creation failed", WSAGetLastError());
		WSACleanup();
		exit(0);
	}

	// UDP 소켓 비동기 모드 설정
	u_long mode = 1;
	ioctlsocket(m_udp_socket, FIONBIO, &mode);

	m_udp_addr.sin_family = AF_INET;
	inet_pton(AF_INET, SERVER_ADDR, &m_udp_addr.sin_addr);
	m_udp_addr.sin_port = htons(UDPPORT);

	CS_TIME_PACKET time;
	time.size = sizeof(time);
	time.type = CS_TIME;
	time.time = 0;
	//DoSendUDP(&time);
}


void NetworkManager::DoSend(void* packet)
{
	CHAR* p = reinterpret_cast<CHAR*>(packet);
	
	WSABUF wsabuf;
	wsabuf.buf = p;
	wsabuf.len = p[0];
	ZeroMemory(&m_overlapped, sizeof(m_overlapped));

	int res = WSASend(m_server_socket, &wsabuf, 1, nullptr, 0, &m_overlapped, nullptr);
	if (0 != res)
	{
		print_error("WSASend", WSAGetLastError());
	}
}

void NetworkManager::SendInput(uint8_t& input_key)
{
	CS_MOVE_PACKET p;
	p.size = sizeof(p);
	p.type = CS_MOVE;
	//p.camera_yaw = static_cast<short>(camera_->GetYaw());
	p.camera_yaw = 0;
	//p.player_yaw = static_cast<short>(rotation_component_->yaw());
	p.player_yaw = 0;
	p.keyinput = input_key;
	DoSend(&p);
}

void NetworkManager::DoSendUDP(void* packet)
{
	DWORD BytesSent = 0;
	DWORD Flags = 0;
	CHAR* p = reinterpret_cast<CHAR*>(packet);

	WSABUF wsabuf;
	wsabuf.buf = p;
	wsabuf.len = p[0];
	ZeroMemory(&m_overlapped, sizeof(m_overlapped));

	int res = WSASendTo(m_udp_socket, &wsabuf, 1, &BytesSent, Flags, 
		reinterpret_cast<SOCKADDR*>(&m_udp_addr), sizeof(m_udp_addr), &m_overlapped, nullptr);
	if (0 != res)
	{
		print_error("WSASendTo", WSAGetLastError());
	}
}

void NetworkManager::DoRecv()
{
	char buffer[BUFSIZE];
	WSABUF wsabuf;
	wsabuf.buf = buffer;
	wsabuf.len = BUFSIZE;

	DWORD recv_flags = 0;
	DWORD bytes_received = 0;

	// TCP
	int res = WSARecv(m_server_socket, &wsabuf, 1, &bytes_received, &recv_flags, nullptr, nullptr);
	if (res == SOCKET_ERROR)
	{
		int err = WSAGetLastError();
		if (err == WSA_IO_PENDING || err == WSAEWOULDBLOCK)
		{
			// NON-Blocking 패킷 안받음
		}
		else if (err == WSAECONNRESET || err == WSAENOTCONN)
		{
			// Disconnect
			closesocket(m_server_socket);
			exit(0);
		}
		else
		{
			print_error("WSARecv", err);
			exit(0);
		}
	}
	else
	{
		ProcessData(wsabuf.buf, bytes_received);
	}

	// UDP
	//sockaddr_in sender_addr;
	//int sender_addr_size = sizeof(sender_addr);

	//res = WSARecvFrom(
	//	m_udp_socket, &wsabuf, 1, &bytes_received, &recv_flags, 
	//	(SOCKADDR*)&sender_addr, &sender_addr_size, nullptr, nullptr);
	//if (res == SOCKET_ERROR)
	//{
	//	int err = WSAGetLastError();
	//	if (err == WSA_IO_PENDING || err == WSAEWOULDBLOCK)
	//	{
	//		// NON-Blocking 패킷 안받음
	//	}
	//	else if (err == WSAECONNRESET || err == WSAENOTCONN)
	//	{
	//		// Disconnect
	//		closesocket(m_server_socket);
	//		exit(0);
	//	}
	//	else
	//	{
	//		print_error("WSARecv", err);
	//		exit(0);
	//	}
	//}
	//else
	//{
	//	ProcessData(wsabuf.buf, bytes_received);
	//}
}

void NetworkManager::ProcessData(char* net_buf, size_t io_byte)
{
	char* ptr = net_buf;
	static size_t in_packet_size = 0;
	static size_t saved_packet_size = 0;
	static char packet_buffer[BUF_SIZE];

	while (0 != io_byte)
	{
		if (0 == in_packet_size) in_packet_size = ptr[0];

		if (io_byte + saved_packet_size >= in_packet_size)
		{
			memcpy(packet_buffer + saved_packet_size, ptr, in_packet_size - saved_packet_size);
			ProcessPacket(packet_buffer);
			ptr += in_packet_size - saved_packet_size;
			io_byte -= in_packet_size - saved_packet_size;
			in_packet_size = 0;
			saved_packet_size = 0;
		}
		else
		{
			memcpy(packet_buffer + saved_packet_size, ptr, io_byte);
			saved_packet_size += io_byte;
			io_byte = 0;
		}
	}
}

void NetworkManager::ProcessPacket(char* ptr)
{
	switch (ptr[1])
	{
	case SC_LOGIN_INFO:
	{
		SC_LOGIN_INFO_PACKET* p = reinterpret_cast<SC_LOGIN_INFO_PACKET*>(ptr);
		m_myid = p->id;
		DirectX::XMFLOAT3 coord = { static_cast<float>(p->x), static_cast<float>(p->y), static_cast<float>(p->z) };
		characters[m_myid].Location = coord;
		float yaw = 0;
		characters[m_myid].yaw = yaw;

		break;
	}
	case SC_ADD_PLAYER:
	{
		SC_ADD_PLAYER_PACKET* p = reinterpret_cast<SC_ADD_PLAYER_PACKET*>(ptr);
		int id = p->id;

		if (id == m_myid)
		{
			// 자신일 경우 자신의 움직임
			//g_objects[id].move(my_packet->x, my_packet->y);
		}
		else
		{
			DirectX::XMFLOAT3 coord = { static_cast<float>(p->x), static_cast<float>(p->y), static_cast<float>(p->z) };
			characters[id].Location = coord;
			float yaw = 0;
			characters[id].yaw = yaw;
		}
		break;
	}
	case SC_MOVE_PLAYER:
	{
		SC_MOVE_PLAYER_PACKET* p = reinterpret_cast<SC_MOVE_PLAYER_PACKET*>(ptr);
		int other_id = p->id;
		if (other_id == m_myid)
		{
			// 자신의 받은 움직임과 look
			DirectX::XMFLOAT3 coord = { static_cast<float>(p->x), static_cast<float>(p->y), static_cast<float>(p->z) };
			characters[m_myid].Location = coord;
			float yaw = p->yaw;
			characters[m_myid].yaw = yaw;
		}
		else
		{
			// 다른 캐릭터의 받은 움직임
			DirectX::XMFLOAT3 coord = { static_cast<float>(p->x), static_cast<float>(p->y), static_cast<float>(p->z) };
			characters[other_id].Location = coord;
			float yaw = p->yaw;
			characters[other_id].yaw = yaw;
		}
		break;
	}
	case SC_TIME:
	{
		SC_TIME_PACKET* time = reinterpret_cast<SC_TIME_PACKET*>(ptr);
		unsigned short game_time = time->time;
		DoSend(time);
		break;
	}
	default:
	{
		printf("Unknown PACKET type [%d]\n", ptr[1]);
		break;
	}
	}
}

