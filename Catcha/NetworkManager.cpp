#include "NetworkManager.h"
#include "VoxelCheese.h"
#include "ParticleObject.h"
#include "TextUIObject.h"
#include "UIObject.h"
#include "SoundManager.h"

constexpr DirectX::XMFLOAT3 CHARACTER_POS[9] =
{
	DirectX::XMFLOAT3(10.0f, FLOOR_Y, 10.0f),					// Mouse1
	DirectX::XMFLOAT3(-10.0f, FLOOR_Y, 10.0f),					// Mouse2
	DirectX::XMFLOAT3(10.0f, FLOOR_Y, -10.0f),					// Mouse3
	DirectX::XMFLOAT3(-10.0f, FLOOR_Y, -10.0f),					// Mouse4
	DirectX::XMFLOAT3(0.0f, FLOOR_Y, 0.0f),						// AI1
	DirectX::XMFLOAT3(0.0f, FLOOR_Y, 0.0f),						// AI2
	DirectX::XMFLOAT3(0.0f, FLOOR_Y, 0.0f),						// AI3
	DirectX::XMFLOAT3(0.0f, FLOOR_Y, 0.0f),						// AI4
	DirectX::XMFLOAT3(437.225f, FLOOR_Y, -117.493f)				// CAT
};

constexpr DirectX::XMFLOAT4 CHARACTER_ROTATION[9] =
{
	DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f),					// Mouse1
	DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f),					// Mouse2
	DirectX::XMFLOAT4(0.0f, 1.0f, 0.0f, 0.0f),					// Mouse3
	DirectX::XMFLOAT4(0.0f, 1.0f, 0.0f, 0.0f),					// Mouse4
	DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f),					// AI1
	DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f),					// AI2
	DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f),					// AI3
	DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f),					// AI4
	DirectX::XMFLOAT4(0.0f, -0.7071f, 0.0f, 0.7071f)			// CAT
};


void print_error(const char* msg, int err_no)
{
	WCHAR* msg_buf;
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, err_no,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		reinterpret_cast<LPWSTR>(&msg_buf), 0, NULL);
	//std::cout << msg;
	OutputDebugStringA(msg);
	std::wcout << L" : 에러 : " << msg_buf;
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

// TODO : 서버 아이피로 변경
#ifdef ENTERIP
	char SERVER_ADDR[BUFSIZE];
	std::ifstream ip_file("networkip.txt");  // 파일 열기

	if (ip_file.is_open()) 
	{
		ip_file.getline(SERVER_ADDR, BUFSIZE);  // 파일에서 IP 주소 읽기
		ip_file.close();  // 파일 닫기
	}
#else
	char SERVER_ADDR[BUFSIZE] = "127.0.0.1";
#endif


	std::cout << "Enter User Name : ";
	//std::cin >> m_name;
	m_name = "Temp01";
	m_password = "1234";

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

	// TCP NonBlocking으로 설정
	u_long noblock = 1;
	res = ioctlsocket(m_server_socket, FIONBIO, &noblock);

	// TODO : 로그인 패킷 특정 상황에 보내도록 옮기기
	CS_LOGIN_PACKET p;
	p.size = sizeof(p);
	p.type = CS_LOGIN;
	strcpy_s(p.name, m_name.c_str());
	strcpy_s(p.password, m_password.c_str());
	DoSend(&p);


	m_sound_manager = SoundManager::Get_Inst();
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

void NetworkManager::SendActionOne(const DirectX::XMFLOAT3& look)
{
	CS_VOXEL_LOOK_PACKET p;
	p.size = sizeof(p);
	p.type = CS_VOXEL_LOOK;
	p.look_x = look.x;
	p.look_y = look.y;
	p.look_z = look.z;
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
		is_login = p->result;
		break;
	}
	case SC_SET_MY_ID:
	{
		SC_SET_MY_ID_PACKET* p = reinterpret_cast<SC_SET_MY_ID_PACKET*>(ptr);
		m_myid = p->my_id;
		break;
	}
	case SC_ADD_PLAYER:
	{
		SC_ADD_PLAYER_PACKET* p = reinterpret_cast<SC_ADD_PLAYER_PACKET*>(ptr);
		int id = p->id;
		int character_id = static_cast<int>(p->character_num);
		// 캐릭터 축가
		characters[id].character_id = character_id;

		DirectX::XMFLOAT3 coord = { static_cast<float>(p->x), static_cast<float>(p->y), static_cast<float>(p->z) };
		DirectX::XMFLOAT4 quat = { p->quat_x, p->quat_y, p->quat_z, p->quat_w };

		characters[id].Location = coord;

		m_objects[characters[id].character_id]->Set_Look(quat);
		m_objects[characters[id].character_id]->Set_Position(coord);
		m_objects[characters[id].character_id]->SetTargetPosition(coord);
		m_objects[characters[id].character_id]->Set_Shade(true);

		if (id == m_myid)
		{
			// 자기 자신
		}
		else
		{
			// 다른 유저 추가
			m_objects[characters[id].character_id]->Set_Visible(true);
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
		Object_State state = static_cast<Object_State>(p->state >> 1);
		bool cat_attacked = (p->state & 1) != 0;
		int curr_hp = static_cast<int>(p->curr_hp) / 5;

		// 공통 동기화
		m_objects[characters[id].character_id]->SetTargetPosition(coord);
		
		// 고양이 방울 소리 재생
		if (characters[id].character_id == NUM_CAT)
		{
			DirectX::XMVECTOR pos = DirectX::XMLoadFloat3(&coord);
			float distance = DirectX::XMVectorGetX(DirectX::XMVector3Length(
				DirectX::XMVectorSubtract(
					m_objects[characters[id].character_id]->Get_Position_V(), pos)));
			if (distance > 10.0f)
			{
				for (auto& observer : m_observers)
				{
					observer->ActiveRingingBell();
				}
			}
		}
		else
		{
			if (id == m_myid)
			{
				for (int i = 0; i < curr_hp; ++i)
				{
					m_gauge_ui_objects[i]->Set_Visible(true);
				}
				for (int i = curr_hp; i < 20; ++i)
				{
					m_gauge_ui_objects[i]->Set_Visible(false);
				}
			}
		}


		// 애니메이션 동기화 정보
		if (m_objects[characters[id].character_id]->Get_State() != state)
		{
			m_objects[characters[id].character_id]->Set_Next_State(state);
			if (state == Object_State::STATE_DEAD)
			{
				m_mouse_ui_objects[characters[id].character_id]->Set_Color_Mul(0.1f, 0.1f, 0.1f, 1.0f);
			}
		}

		// 타격시 색 변화 정보
		if (true == cat_attacked)
		{
			m_objects[characters[id].character_id]->Set_Color_Mul(1.0f, 0.0f, 0.0f, 1.0f);
			// 타격당한 캐릭터 소리 재생
			m_sound_manager->Play_Sound(L"hit_sound", L"hit_sound.mp3", 
				m_objects[characters[id].character_id]->Get_Position_Addr(), nullptr, false);
			// 고양이 타격 소리 재생
			m_sound_manager->Play_Sound(L"swing_sound", L"swing_sound.wav",
				m_objects[NUM_CAT]->Get_Position_Addr(), nullptr, false);

			m_particle_object->Add_Particle(
				m_objects[characters[id].character_id]->Get_Position_3f(),
				DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f),
				DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f),
				20,
				*m_total_time
			);

			// 내 플레이어 타격시, 타격 효과 재생
			if (id == m_myid)
			{
				for (auto& observer : m_observers)
				{
					observer->AttackedUI();
				}
			}
		}
		else
		{
			m_objects[characters[id].character_id]->Set_Color_Mul(1.0f, 1.0f, 1.0f, 1.0f);
		}

		// 다른 캐릭터의 받은 회전값
		m_objects[characters[id].character_id]->SetTargetPitch(p->player_pitch);			
		
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
			// 자신의 받은 움직임과 look
			
			characters[m_myid].Location = coord;
			m_objects[characters[m_myid].character_id]->SetTargetPosition(coord);
			m_objects[characters[m_myid].character_id]->Set_Rotate(quat);
			for (auto& observer : m_observers)
			{
				observer->InitCamera(quat);
			}

			// 다시 서버로 동기화 패킷 전송
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
			// 다른 캐릭터의 받은 움직임
			characters[id].Location = coord;
			m_objects[characters[id].character_id]->SetTargetPosition(coord);
			m_objects[characters[id].character_id]->Set_Look(quat);
		}
		break;
	}
	case SC_CHANGE_CHARACTER:
	{
		SC_CHANGE_CHARACTER_PACKET* p = reinterpret_cast<SC_CHANGE_CHARACTER_PACKET*>(ptr);
		int id = p->player_num;
		int character_id = characters[p->player_num].character_id;

		int prev_character_num = static_cast<int>(p->prev_character_num);
		int new_character_num = static_cast<int>(p->new_character_num);

		// 자신의 캐릭터 변경
		if (id == m_myid)
		{
			ChangeOwnCharacter(character_id, new_character_num);
			characters[m_myid].character_id = new_character_num;
			for (auto& observer : m_observers)
			{
				observer->InitCamera();
			}
			m_particle_object->Add_Particle(
				m_objects[characters[id].character_id]->Get_Position_3f(),
				DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f),
				DirectX::XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f),
				20,
				*m_total_time
			);
		}
		else
		{
			// 다른 캐릭터의 캐릭터 변경
			characters[id].character_id = new_character_num;
			m_objects[characters[id].character_id]->SetLerpDegree(4.0f);
			m_objects[characters[id].character_id]->SetTargetQuat(DirectX::XMFLOAT4{ 0.0f, 0.0f, 0.0f, 1.0f });
			m_particle_object->Add_Particle(
				m_objects[characters[id].character_id]->Get_Position_3f(),
				DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f),
				DirectX::XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f),
				20,
				*m_total_time
			);
		}
		break;
	}
	case SC_TIME:
	{
		SC_TIME_PACKET* p = reinterpret_cast<SC_TIME_PACKET*>(ptr);
		m_game_time = p->time;
		

		if (m_game_time != 0)
		{
			m_time_object->Set_Position(-0.05f, 0.9f, 0.0f);
			m_time_object->Set_Scale(0.1f, 0.1f, 0.0f);
			m_time_object->Set_Text(std::to_wstring((int)m_game_time % 300));
		}
		else
		{
			m_time_object->Set_Position(-0.15f, 0.9f, 0.0f);
			m_time_object->Set_Scale(0.06f, 0.08f, 0.0f);
			m_time_object->Set_Text(L"게임 종료!");
		}
		break;
	}
	case SC_RANDOM_VOXEL_SEED:
	{
		SC_RANDOM_VOXEL_SEED_PACKET* p = reinterpret_cast<SC_RANDOM_VOXEL_SEED_PACKET*>(ptr);
		int i = 0;
		for(auto& cheese : m_cheeses)
		{
			int random_seed = p->random_seeds[i++];
			cheese->Remove_Random_Voxel(random_seed);
		}
		break;
	}
	case SC_REMOVE_VOXEL_SPHERE:
	{
		SC_REMOVE_VOXEL_SPHERE_PACKET* p = reinterpret_cast<SC_REMOVE_VOXEL_SPHERE_PACKET*>(ptr);
		int cheese_num = static_cast<int>(p->cheese_num >> 1);
		bool is_removed_all = (p->cheese_num & 1) != 0;
		DirectX::XMFLOAT3 sphere_center {p->center_x, p->center_y, p->center_z};
		float radius = 10.0f;

		m_sound_manager->Play_Sound(L"eating_sound", L"eating_sound.wav",
			&sphere_center, nullptr, false);

		// 치즈 전부 삭제되었을시
		if (true == is_removed_all)
		{
			m_cheeses[cheese_num]->Remove_All_Voexl();
			m_cheese_ui_objects[cheese_num]->Set_Color_Mul(0.1f, 0.1f, 0.1f, 1.0f);
		}
		// 치즈 일부 삭제시
		else
		{
			m_cheeses[cheese_num]->Remove_Sphere_Voxel(sphere_center, radius);
		}
		m_particle_object->Add_Particle(
			sphere_center,
			DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f),
			DirectX::XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f), 
			20, 
			*m_total_time
		);
		break;
	}
	case SC_AI_MOVE:
	{
		SC_AI_MOVE_PACKET* p = reinterpret_cast<SC_AI_MOVE_PACKET*>(ptr);
		int AI_id = p->id;
		DirectX::XMFLOAT3 coord = { static_cast<float>(p->x), FLOOR_Y, static_cast<float>(p->z) };
		m_objects[AI_id]->SetTargetPosition(coord);
		
		DirectX::XMVECTOR prev_pos = m_objects[AI_id]->Get_Position_V();
		DirectX::XMVECTOR target_pos = DirectX::XMLoadFloat3(&coord);
		DirectX::XMVECTOR dir = DirectX::XMVectorSubtract(target_pos, prev_pos);
		dir = DirectX::XMVectorSetY(dir, 0.0f);
		dir = DirectX::XMVector3Normalize(dir);

		// 기준 전방 벡터
		DirectX::XMVECTOR forward = DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);

		// 회전축 계산 (전방 벡터와 dir 벡터 사이)
		DirectX::XMVECTOR axis = DirectX::XMVector3Cross(forward, dir);

		// 내적을 이용해 회전각 계산
		float dot = DirectX::XMVectorGetX(DirectX::XMVector3Dot(forward, dir));
		float angle = std::acos(std::clamp(dot, -1.0f, 1.0f));

		DirectX::XMVECTOR cross = DirectX::XMVector3Cross(forward, dir);
		float cross_y = DirectX::XMVectorGetY(cross);

		if (cross_y < 0.0f)
		{
			angle = -angle; // 반대 방향 회전 적용
		}

		// Y축 회전 쿼터니언 생성
		DirectX::XMVECTOR quaternion = DirectX::XMQuaternionRotationAxis(DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f), angle);

		// 쿼터니언을 XMFLOAT4로 변환 후 적용
		DirectX::XMFLOAT4 quat;
		DirectX::XMStoreFloat4(&quat, quaternion);
		m_objects[AI_id]->SetTargetQuat(quat);

		break;
	}
	case SC_GAME_START:
	{
		InitialzeCharacters();
		break;
	}
	case SC_GAME_OPEN_DOOR:
	{
		for (auto& observer : m_observers)
		{
			observer->OpenDoorEvent();
		}
		
		break;
	}
	case SC_GAME_WIN_CAT:
	{
		EndSceneInitCharacters();
		for (auto& observer : m_observers)
		{
			observer->ShowingResultScene(true);
		}
		break;
	}
	case SC_GAME_WIN_MOUSE:
	{
		EndSceneInitCharacters();
		for (auto& observer : m_observers)
		{
			observer->ShowingResultScene(false);
		}
		break;
	}
	case SC_PLAYER_ESCAPE:
	{
		SC_PLAYER_STATE_PACKET* p = reinterpret_cast<SC_PLAYER_STATE_PACKET*>(ptr);
		int id = p->id;

		// 안보이게
		m_objects[characters[id].character_id]->Set_Visible(false);
		m_objects[characters[id].character_id]->Set_Shade(false);
		m_mouse_ui_objects[characters[id].character_id]->Set_Color_Mul(0.1f, 0.1f, 0.1f, 1.0f);

		m_particle_object->Add_Particle(
			m_door_camera->Get_Position_3f(),
			DirectX::XMFLOAT3(3.0f, 3.0f, 3.0f),
			DirectX::XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f),
			1000,
			*m_total_time
		);
		

		// 자신이 탈출한 경우 카메라 문으로 보이게
		if (m_myid == id)
		{
			for (auto& observer : m_observers)
			{
				observer->SetEscape();
			}
		}

		break;
	}
	case SC_PLAYER_REBORN:
	{
		for (auto& observer : m_observers)
		{
			observer->RebornUICount();
		}
		break;
	}
	case SC_PLAYER_DEAD:
	{
		SC_PLAYER_STATE_PACKET* p = reinterpret_cast<SC_PLAYER_STATE_PACKET*>(ptr);
		int id = p->id;

		if (m_myid == id)
		{
			for (auto& observer : m_observers)
			{
				observer->ObservingMode();
			}
		}
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
			// 쥐인지 고양이인지
			bool is_cat = (NUM_CAT == new_number);

			// 객체 이름 및 포인터 swap
			// enum의 순서를 보장하기 위해 오브젝트 순서를 enum 순서에 맞춘다
			if (character_id == NUM_GHOST)
			{
				m_objects[character_id]->Set_Name(L"free_mode");
			}

			// 접속전 기본 캐릭터 안보이게 설정
			if (m_objects[character_id]->Get_Character_Number() == NUM_GHOST)
			{
				m_objects[character_id]->Set_Visible(false);
				m_objects[character_id]->Set_Position(-999.0f, -999.0f, -999.0f);
			}

			character->Set_Name(L"player");

			// 쥐 1인칭이라 자기 캐릭터 안그리게 설정
			if (false == is_cat) 
			{
				character->Set_Visible(false);
				character->SetLerpDegree(4.0f);
				character->SetTargetQuat(DirectX::XMFLOAT4{ 0.0f, 0.0f, 0.0f, 1.0f });
				is_player_cat = false;
			}
			else
			{
				character->Set_Visible(true);
				character->Set_Shade(true);
				is_player_cat = true;
			}

			std::wstring new_name = is_cat ? L"cat" : L"mouse" + std::to_wstring(new_number);

			// Observer에게 플레이어블 캐릭터 변경 알림
			for (auto& observer : m_observers) 
			{
				observer->CharacterChange(is_cat, L"player", new_name);
			}
			break;
		}
	}
}


void NetworkManager::InitialzeCharacters()
{
	for(int i = NUM_MOUSE1; i <= NUM_CAT; ++i)
	{
		m_objects[i]->SetTargetPosition(CHARACTER_POS[i]);
		m_objects[i]->Set_Position(CHARACTER_POS[i]);
		m_objects[i]->Rst_Rotate();
		m_objects[i]->SetTargetQuat(CHARACTER_ROTATION[i]);
		m_objects[i]->Set_Rotate(CHARACTER_ROTATION[i]);
		m_objects[i]->Update(0.0f);
	}

	for (auto& observer : m_observers)
	{
		observer->InitCamera(CHARACTER_ROTATION[characters[m_myid].character_id]);
	}

}

void NetworkManager::EndSceneInitCharacters()
{
	for (int i = NUM_MOUSE1; i <= NUM_GHOST; ++i)
	{
		m_objects[i]->SetTargetPosition(DirectX::XMFLOAT3(0.0f, 999.0f, 0.0f));
		m_objects[i]->Set_Position(DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f));
		m_objects[i]->SetTargetQuat(DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));
		m_objects[i]->Rst_Rotate();
		m_objects[i]->TP_Down(999.0f);
		m_objects[i]->Set_Visible(true);
		m_objects[i]->Set_Shade(true);
		m_objects[i]->Set_State(Object_State::STATE_IDLE);
		m_objects[i]->Set_Next_State(Object_State::STATE_IDLE);
		m_objects[i]->Set_Camera_Need_Send(false);
		m_objects[i]->SetLerpDegree(4.0f);
		m_objects[i]->Set_Color_Mul(1.0f, 1.0f, 1.0f, 1.0f);
		m_objects[i]->Set_Character_Number(i);

		if (i < NUM_CAT)
		{
			m_objects[i]->Set_Name(L"mouse" + std::to_wstring(i));
		}
		else if (i == NUM_CAT)
		{
			m_objects[i]->Set_Name(L"cat");
		}
		else
		{
			m_objects[i]->Set_Name(L"player");
		}
	}

	for (int i = NUM_AI1; i <= NUM_AI4; ++i)
	{
		m_objects[i]->TP_Up(999.0f + FLOOR_Y - 10.0f);
		m_objects[i]->SetTargetPosition(DirectX::XMFLOAT3(0.0f, FLOOR_Y - 10.0f, 0.0f));
		m_objects[i]->SetLerpDegree(50.0f);
	}
}

void NetworkManager::SetObjectsVisible(bool visible)
{
	for (auto& obj : m_objects)
	{
		obj->Set_Visible(visible);
		obj->Set_Shade(visible);
	}
}
