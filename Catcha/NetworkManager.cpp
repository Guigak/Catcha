#include "NetworkManager.h"

void print_error(const char* msg, int err_no)
{
	WCHAR* msg_buf;
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, err_no,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		reinterpret_cast<LPWSTR>(&msg_buf), 0, NULL);
	//std::cout << msg;
	OutputDebugStringA(msg);
	std::wcout << L" : ���� : " << msg_buf;
	while (true);
	LocalFree(msg_buf);
}

#define NAGLEOFF
#define ENTERIP
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

// TODO : ���� �����Ƿ� ����
#ifdef ENTERIP
	char SERVER_ADDR[BUFSIZE];
	std::ifstream ip_file("networkip.txt");  // ���� ����

	if (ip_file.is_open()) 
	{
		ip_file.getline(SERVER_ADDR, BUFSIZE);  // ���Ͽ��� IP �ּ� �б�
		ip_file.close();  // ���� �ݱ�
	}
#else
	char SERVER_ADDR[BUFSIZE] = "127.0.0.1";
#endif


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
	setsockopt(m_server_socket, DelayZero, TCP_NODELAY, (const char*)&DelayZero, sizeof(DelayZero));
#endif

	// TCP NonBlocking���� ����
	u_long noblock = 1;
	res = ioctlsocket(m_server_socket, FIONBIO, &noblock);

	// TODO : �α��� ��Ŷ Ư�� ��Ȳ�� �������� �ű��
	CS_LOGIN_PACKET p;
	p.size = sizeof(p);
	p.type = CS_LOGIN;
	strcpy_s(p.name, m_name.c_str());
	DoSend(&p);

	// UDP ���� Initialize
	m_udp_socket = WSASocket(AF_INET, SOCK_DGRAM, IPPROTO_UDP, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (m_udp_socket == INVALID_SOCKET)
	{
		print_error("UDP socket creation failed", WSAGetLastError());
		WSACleanup();
		exit(0);
	}

	// UDP ���� �񵿱� ��� ����
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
	p.keyinput = input_key;
	DoSend(&p);
}

void NetworkManager::SendRotate(float& pitch)
{
	CS_ROTATE_PACKET p;
	p.size = sizeof(p);
	p.type = CS_ROTATE;
	p.player_pitch = pitch;
	DoSend(&p);
}

void NetworkManager::ChooseCharacter(bool IsCat)
{
	CS_CHOOSE_CHARACTER_PACKET p;
	p.size = sizeof(p);
	p.type = CS_CHOOSE_CHARACTER;
	p.is_cat = IsCat;
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
			// NON-Blocking ��Ŷ �ȹ���
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
	//		// NON-Blocking ��Ŷ �ȹ���
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
		float pitch = 0.f;
		characters[m_myid].pitch = pitch;

		break;
	}
	case SC_ADD_PLAYER:
	{
		SC_ADD_PLAYER_PACKET* p = reinterpret_cast<SC_ADD_PLAYER_PACKET*>(ptr);
		int id = p->id;
		int character_id = static_cast<int>(p->character_num);
		characters[id].character_id = character_id;

		DirectX::XMFLOAT3 coord = { static_cast<float>(p->x), static_cast<float>(p->y), static_cast<float>(p->z) };
		DirectX::XMFLOAT4 quat = { p->quat_x, p->quat_y, p->quat_z, p->quat_w };

		characters[id].Location = coord;

		m_objects[characters[id].character_id]->Set_Look(quat);
		m_objects[characters[id].character_id]->Set_Position(coord.x, coord.y, coord.z);

		if (id == m_myid)
		{
			// �ڱ� �ڽ�
		}
		else
		{
			// �ٸ� ���� �߰�
			m_objects[characters[id].character_id]->Set_Visiable(true);
		}
		break;
	}
	case SC_MOVE_PLAYER:
	{
		SC_MOVE_PLAYER_PACKET* p = reinterpret_cast<SC_MOVE_PLAYER_PACKET*>(ptr);
		int id = p->id;
		DirectX::XMFLOAT3 coord = { static_cast<float>(p->x), static_cast<float>(p->y), static_cast<float>(p->z) };
		characters[id].Location = coord;
		characters[id].pitch = p->player_pitch;
		Object_State state = static_cast<Object_State>(p->state >> 3);
		bool on_ground = (p->state & (1 << 2)) != 0;
		bool need_blending = (p->state & (1 << 1)) != 0;
		bool cat_attacked = (p->state & 1) != 0;

		// ���� ����ȭ
		m_objects[characters[id].character_id]->SetTargetPosition(coord);
		m_objects[characters[id].character_id]->Set_Grounded(on_ground);

		// �ִϸ��̼� ����ȭ ����
		if (true == need_blending)
		{
			m_objects[characters[id].character_id]->Set_Next_State(state);
		}
		else
		{
			m_objects[characters[id].character_id]->Set_Network_State(state);
		}

		// �� ĳ����
		if (id == m_myid)
		{
			// pass
		}
		// Ÿ ĳ����(�ڱ� �ڽ��� �� �� ȸ���� ���� �ʿ䰡 ���..)
		else
		{
			// Ÿ�ݽ� �� ��ȭ ����
			if (true == cat_attacked)
			{
				m_objects[characters[id].character_id]->Set_Color_Mul(1.0f, 0.0f, 0.0f);
			}
			else
			{
				m_objects[characters[id].character_id]->Set_Color_Mul(1.0f, 1.0f, 1.0f);
			}

			// �ٸ� ĳ������ ���� ȸ����
			m_objects[characters[id].character_id]->SetTargetPitch(p->player_pitch);			
		}
		break;
	}
	case SC_SYNC_PLAYER:
	{
		SC_SYNC_PLAYER_PACKET* p = reinterpret_cast<SC_SYNC_PLAYER_PACKET*>(ptr);
		int id = p->id;
		DirectX::XMFLOAT3 coord = { static_cast<float>(p->x), static_cast<float>(p->y), static_cast<float>(p->z) };
		DirectX::XMFLOAT4 quat = { p->quat_x, p->quat_y, p->quat_z, p->quat_w };

		if (id == m_myid)
		{
			// �ڽ��� ���� �����Ӱ� look
			
			characters[m_myid].Location = coord;
			m_objects[characters[m_myid].character_id]->SetTargetPosition(coord);
			m_objects[characters[m_myid].character_id]->Set_Look(quat);

			// �ٽ� ������ ����ȭ ��Ŷ ����
			CS_SYNC_PLAYER_PACKET sync;
			sync.x = m_objects[characters[m_myid].character_id]->Get_Position_3f().x;
			sync.y = m_objects[characters[m_myid].character_id]->Get_Position_3f().y;
			sync.z = m_objects[characters[m_myid].character_id]->Get_Position_3f().z;
			sync.quat_x = m_objects[characters[m_myid].character_id]->Get_Rotate_RPY_4f().x;
			sync.quat_y = m_objects[characters[m_myid].character_id]->Get_Rotate_RPY_4f().y;
			sync.quat_z = m_objects[characters[m_myid].character_id]->Get_Rotate_RPY_4f().z;
			sync.quat_w = m_objects[characters[m_myid].character_id]->Get_Rotate_RPY_4f().w;

			sync.size = sizeof(sync);
			sync.type = CS_SYNC_PLAYER;
			sync.id = m_myid;
			DoSend(&sync);
		}
		else
		{
			// �ٸ� ĳ������ ���� ������
			characters[id].Location = coord;
			m_objects[characters[id].character_id]->SetTargetPosition(coord);
			m_objects[characters[id].character_id]->Set_Look(quat);
		}
		break;
	}
	case SC_CHANGE_CHARACTER:
	{
		SC_CHANGE_CHARACTER_PACKET* p = reinterpret_cast<SC_CHANGE_CHARACTER_PACKET*>(ptr);
		int id = p->id;
		int character_id = characters[p->id].character_id;

		int prev_character_num = static_cast<int>(p->prev_character_num);
		int new_character_num = static_cast<int>(p->new_character_num);

		// �ڽ��� ĳ���� ����
		if (id == m_myid)
		{
			ChangeOwnCharacter(character_id, new_character_num);
			characters[m_myid].character_id = new_character_num;
			for (auto& observer : m_observers)
			{
				observer->InitCamera();
			}
		}
		else
		{
			// �ٸ� ĳ������ ĳ���� ����
			characters[id].character_id = new_character_num;
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

void NetworkManager::ChangeOwnCharacter(int character_id, int new_number)
{
	for (auto& character : m_objects) 
	{
		if (new_number == character->Get_Character_Number()) 
		{
			// ������ ���������
			bool is_cat = (NUM_CAT == new_number);

			// ��ü �̸� �� ������ swap
			// enum�� ������ �����ϱ� ���� ������Ʈ ������ enum ������ �����
			m_objects[character_id]->Set_Name(L"free_mode");

			// ������ �⺻ ĳ���� �Ⱥ��̰� ����
			if (m_objects[character_id]->Get_Character_Number() == -1)
			{
				m_objects[character_id]->Set_Visiable(false);
				m_objects[character_id]->Set_Position(0.0f, -999.0f, 0.0f);
			}

			character->Set_Name(L"player");

			// �� 1��Ī�̶� �ڱ� ĳ���� �ȱ׸��� ����
			if (!is_cat) 
			{
				character->Set_Visiable(false);
			}

			std::wstring new_name = is_cat ? L"cat" : L"mouse" + std::to_wstring(new_number);

			// Observer���� �÷��̾�� ĳ���� ���� �˸�
			for (auto& observer : m_observers) 
			{
				observer->CharacterChange(is_cat, L"player", new_name);
			}
			break;
		}
	}
}
