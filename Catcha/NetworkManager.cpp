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
	if (true != res)
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
}


void NetworkManager::DoSend(void* packet)
{
	CHAR* p = reinterpret_cast<CHAR*>(packet);
	
	WSABUF wsabuf;
	wsabuf.buf = p;
	wsabuf.len = p[0];
	ZeroMemory(&m_overlapped, sizeof(m_overlapped));

	int res = WSASend(m_server_socket, &wsabuf, 1, nullptr, 0, &m_overlapped, nullptr);
	if (true != res)
	{
		print_error("WSASend", WSAGetLastError());
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

	int res = WSARecv(m_server_socket, &wsabuf, 1, &wsabuf.len, &recv_flags, nullptr, nullptr);
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
}

void NetworkManager::ProcessData(char* net_buf, size_t io_byte)
{
	OVERLAPPED over;
	char* p = net_buf;
	int total_data = io_byte;
}
