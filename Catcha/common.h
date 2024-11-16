#pragma once

// disable error
#pragma warning(disable : 4017)

// about memory leak
#if defined(DEBUG) || defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

// NetworkManager
#include <WS2tcpip.h>
#include <MSWSock.h>
#include <stdint.h>
#include <chrono>
#pragma comment(lib, "WS2_32.lib")
#pragma comment(lib, "MSWSock.lib")

#include "protocol.h"

constexpr short PORT = 4000;
constexpr short UDPPORT = 8000;
constexpr int BUFSIZE = 256;
constexpr int MAX_USER = 4;
constexpr int MAX_NPC = 4;

extern void print_error(const char* msg, int err_no);


// windows
#include <windows.h>
#include <wrl.h>
#include <comdef.h>

#pragma comment(lib, "winmm.lib")

// directX
#include <dxgi1_4.h>
#include <d3d12.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include <DirectXColors.h>
#include <DirectXCollision.h>

#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "D3D12.lib")
#pragma comment(lib, "dxgi.lib")

// standard
#include <iostream>
#include <string>
#include <memory>
#include <algorithm>
#include <vector>
#include <array>
#include <set>
#include <map>
#include <stack>
#include <unordered_set>
#include <unordered_map>
#include <cstdint>
#include <fstream>
#include <sstream>
#include <cassert>

#include <variant>

// fbx sdk
#include "fbxsdk.h"

// constant
constexpr int CLIENT_WIDTH = 1200;
constexpr int CLIENT_HEIGHT = 1024;

constexpr int FRAME_RESOURCES_NUMBER = 3;

constexpr int MAX_BONE_COUNT = 64;
constexpr int MAX_WEIGHT_BONE_COUNT = 4;

constexpr int MAX_MATERIAL_COUNT = 32;

constexpr float RIGHT_ANGLE_RADIAN = DirectX::XMConvertToRadians(90.0f);

constexpr BYTE MOVE_ALL_AXIS = 0b00;
constexpr BYTE MOVE_ONLY_XZ = 0b01;

constexpr BYTE ROTATE_SYNC_NONE = 0b00;
constexpr BYTE ROTATE_SYNC_ALL = 0b01;
constexpr BYTE ROTATE_SYNC_RPY = 0b10;	// ROTATE_ROLL_PITCH_YAW

constexpr bool LOOP_ANIMATION = true;
constexpr bool ONCE_ANIMATION = false;

constexpr bool MOVABLE = true;
constexpr bool NOT_MOVABLE = false;

// virtual key
#define VK_NUM0 0x30
#define VK_NUM1 0x31
#define VK_NUM2 0x32
#define VK_NUM3 0x33
#define VK_NUM4 0x34
#define VK_NUM5 0x35
#define VK_NUM6 0x36
#define VK_NUM7 0x37
#define VK_NUM8 0x38
#define VK_NUM9 0x39

#define VK_A 0x41
#define VK_B 0x42
#define VK_C 0x43
#define VK_D 0x44
#define VK_E 0x45
#define VK_F 0x46
#define VK_G 0x47
#define VK_H 0x48
#define VK_I 0x49
#define VK_J 0x4A
#define VK_K 0x4B
#define VK_L 0x4C
#define VK_M 0x4D
#define VK_N 0x4E
#define VK_O 0x4F
#define VK_P 0x50
#define VK_Q 0x51
#define VK_R 0x52
#define VK_S 0x53
#define VK_T 0x54
#define VK_U 0x55
#define VK_V 0x56
#define VK_W 0x57
#define VK_X 0x58
#define VK_Y 0x59
#define VK_Z 0x5A

#define MESH_INFO 0b0001
#define MATERIAL_INFO 0b0010
#define SKELETON_INFO 0b0100
#define ANIMATION_INFO 0b1000

// enum class
enum class Object_State {
	STATE_IDLE, STATE_MOVE,
	STATE_JUMP_START, STATE_JUMP_IDLE, STATE_JUMP_END,
	STATE_ACTION_ONE, STATE_ACTION_TWO, STATE_ACTION_THREE
};

// struct
struct D3D12_DEFAULT;
struct D3D12_RECT_EX;
struct D3D12_BOX_EX;
struct D3D12_HEAP_PROPERTIES_EX;
struct D3D12_HEAP_DESC_EX;
struct D3D12_RESOURCE_BARRIER_EX;
struct D3D12_TEXTURE_COPY_LOCATION_EX;
struct D3D12_ROOT_SIGNATURE_DESC_EX;
struct D3D12_CPU_DESCRIPTOR_HANDLE_EX;
struct D3D12_GPU_DESCRIPTOR_HANDLE_EX;
struct D3D12_RESOURCE_DESC_EX;
struct D3D12_DESCRIPTOR_RANGE_EX;
struct D3D12_ROOT_DESCRIPTOR_TABLE_EX;
struct D3D12_ROOT_CONSTANTS_EX;
struct D3D12_ROOT_DESCRIPTOR_EX;
struct D3D12_ROOT_PARAMETER_EX;

// Character Number
enum CHARACTER_NUMBER
{
	NUM_MOUSE1 = 0,
	NUM_MOUSE2,
	NUM_MOUSE3,
	NUM_MOUSE4,
	NUM_AI1,
	NUM_AI2,
	NUM_AI3,
	NUM_AI4,
	NUM_CAT,
	NUM_GHOST
};



// math
struct MathHelper {
	static float Infinity() { return FLT_MAX; }
	static float Pi() { return DirectX::XM_PI; }

	static float Rand_F() { return (float)rand() / (float)RAND_MAX; }
	static float Rand_F(float a, float b) { return a + Rand_F() * (b - a); }
	static int Rand_I() { return rand(); }
	static int Rand_I(int a, int b) { return a + rand() % ((b - a) + 1); }

	template<typename T>
	static T Min(const T& a, const T& b) { return a < b ? a : b; }

	template<typename T>
	static T Max(const T& a, const T& b) { return a > b ? a : b; }

	template<typename T>
	static T Lerp(const T& a, const T& b, float t) { return a + (b - a) * t; }

	template<typename T>
	static T Clamp(const T& x, const T& min, float max) { return x < min ? min : (x > max ? max : x); }

	static DirectX::XMMATRIX Inverse_Transpose(DirectX::XMMATRIX m) {
		m.r[3] = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);

		DirectX::XMVECTOR determinant = DirectX::XMMatrixDeterminant(m);
		
		return DirectX::XMMatrixTranspose(DirectX::XMMatrixInverse(&determinant, m));
	}

	static DirectX::XMMATRIX Inverse(DirectX::XMMATRIX m) {
		return  DirectX::XMMatrixInverse(nullptr, m);
	}

	static DirectX::XMFLOAT4X4 Identity_4x4() {
		static DirectX::XMFLOAT4X4 identity{
			1.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f
		};

		return identity;
	}

	static DirectX::XMMATRIX XMMATRIX_Translation(const DirectX::XMFLOAT3& xmfloat3) {
		return DirectX::XMMatrixTranslation(xmfloat3.x, xmfloat3.y, xmfloat3.z);
	}

	static DirectX::XMMATRIX XMMATRIX_Rotation(const DirectX::XMFLOAT4& xmfloat4) {
		return DirectX::XMMatrixRotationQuaternion(DirectX::XMLoadFloat4(&xmfloat4));
	}

	static DirectX::XMMATRIX XMMATRIX_Scaling(const DirectX::XMFLOAT3& xmfloat3) {
		return DirectX::XMMatrixScaling(xmfloat3.x, xmfloat3.y, xmfloat3.z);
	}

	static DirectX::XMFLOAT2 Multiply(const DirectX::XMFLOAT2& xmfloat2, const DirectX::XMMATRIX& matrix) {
		DirectX::XMVECTOR vector = DirectX::XMLoadFloat2(&xmfloat2);

		DirectX::XMFLOAT2 result;
		DirectX::XMStoreFloat2(&result, DirectX::XMVector2Transform(vector, matrix));

		return result;
	}

	static DirectX::XMFLOAT3 Multiply(const DirectX::XMFLOAT3& xmfloat3, const DirectX::XMMATRIX& matrix) {
		DirectX::XMVECTOR vector = DirectX::XMLoadFloat3(&xmfloat3);

		DirectX::XMFLOAT3 result;
		DirectX::XMStoreFloat3(&result, DirectX::XMVector3Transform(vector, matrix));

		return result;
	}

	static DirectX::XMFLOAT4X4 Multiply(const DirectX::XMFLOAT4X4& xmfloat4x4, const DirectX::XMMATRIX& matrix) {
		DirectX::XMFLOAT4X4 result;
		DirectX::XMStoreFloat4x4(&result, DirectX::XMMatrixMultiply(DirectX::XMLoadFloat4x4(&xmfloat4x4), matrix));

		return result;
	}

	//
	static DirectX::XMFLOAT3 Add(const DirectX::XMFLOAT3& xmfloat3_a, const DirectX::XMFLOAT3 xmfloat3_b) {
		DirectX::XMFLOAT3 result;
		DirectX::XMStoreFloat3(&result, DirectX::XMVectorAdd(DirectX::XMLoadFloat3(&xmfloat3_a), DirectX::XMLoadFloat3(&xmfloat3_b)));

		return result;
	}

	static DirectX::XMFLOAT3 Add(const DirectX::XMFLOAT3& xmfloat3_a, const DirectX::XMFLOAT3 xmfloat3_b, float scalar) {
		DirectX::XMFLOAT3 result;
		DirectX::XMStoreFloat3(&result, DirectX::XMVectorAdd(DirectX::XMLoadFloat3(&xmfloat3_a),
			DirectX::XMVectorScale(DirectX::XMLoadFloat3(&xmfloat3_b), scalar)));

		return result;
	}

	static DirectX::XMFLOAT3 Subtract(const DirectX::XMFLOAT3& xmfloat3_a, const DirectX::XMFLOAT3 xmfloat3_b) {
		DirectX::XMFLOAT3 result;
		DirectX::XMStoreFloat3(&result, DirectX::XMVectorSubtract(DirectX::XMLoadFloat3(&xmfloat3_a), DirectX::XMLoadFloat3(&xmfloat3_b)));

		return result;
	}

	static DirectX::XMFLOAT3 Multiply(const DirectX::XMFLOAT3& xmfloat3, float scalar) {
		DirectX::XMFLOAT3 result;
		DirectX::XMStoreFloat3(&result, DirectX::XMVectorScale(DirectX::XMLoadFloat3(&xmfloat3), scalar));

		return result;
	}

	static DirectX::XMFLOAT3 Normalize(const DirectX::XMFLOAT3& xmfloat3) {
		DirectX::XMFLOAT3 result;
		DirectX::XMStoreFloat3(&result, DirectX::XMVector3Normalize(DirectX::XMLoadFloat3(&xmfloat3)));

		return result;
	}

	static float Length(const DirectX::XMFLOAT3& xmfloat3) {
		return DirectX::XMVectorGetX(DirectX::XMVector3Length(DirectX::XMLoadFloat3(&xmfloat3)));
	}

	static DirectX::XMFLOAT3 Get_XZ(const DirectX::XMFLOAT3& xmfloat3) {
		DirectX::XMFLOAT3 result = DirectX::XMFLOAT3(xmfloat3.x, 0.0f, xmfloat3.z);

		return result;
	}

	static DirectX::XMFLOAT3 Get_XZ_Norm(const DirectX::XMFLOAT3& xmfloat3) {
		DirectX::XMFLOAT3 result;
		DirectX::XMStoreFloat3(&result, DirectX::XMVector3Normalize(DirectX::XMLoadFloat3(&Get_XZ(xmfloat3))));

		return result;
	}

	static float Length_XZ(const DirectX::XMFLOAT3& xmfloat3) {
		DirectX::XMFLOAT3 result = Get_XZ(xmfloat3);

		return DirectX::XMVectorGetX(DirectX::XMVector3Length(DirectX::XMLoadFloat3(&result)));
	}

	static DirectX::XMFLOAT3 Dot(const DirectX::XMFLOAT3& xmfloat3_a, const DirectX::XMFLOAT3 xmfloat3_b) {
		DirectX::XMFLOAT3 result;
		DirectX::XMStoreFloat3(&result, DirectX::XMVector3Dot(DirectX::XMLoadFloat3(&xmfloat3_a), DirectX::XMLoadFloat3(&xmfloat3_b)));

		return result;
	}

	static DirectX::XMFLOAT3 Cross(const DirectX::XMFLOAT3& xmfloat3_a, const DirectX::XMFLOAT3 xmfloat3_b) {
		DirectX::XMFLOAT3 result;
		DirectX::XMStoreFloat3(&result, DirectX::XMVector3Cross(DirectX::XMLoadFloat3(&xmfloat3_a), DirectX::XMLoadFloat3(&xmfloat3_b)));

		return result;
	}
};

// Math Helper XMFLOAT3 사용을 위한 오버로딩
// Overload + for XMFLOAT3
static DirectX::XMFLOAT3 operator+(const DirectX::XMFLOAT3& a, const DirectX::XMFLOAT3& b) {
	return MathHelper::Add(a, b);
}

// Overload - for XMFLOAT3
static DirectX::XMFLOAT3 operator-(const DirectX::XMFLOAT3& a, const DirectX::XMFLOAT3& b) {
	return MathHelper::Subtract(a, b);
}

// Overload * for scalar
static DirectX::XMFLOAT3 operator*(const DirectX::XMFLOAT3& a, float scalar) {
	return MathHelper::Multiply(a, scalar);
}

// Overload * for scalar
static DirectX::XMFLOAT3 operator*(float scalar, const DirectX::XMFLOAT3& a) {
	return MathHelper::Multiply(a, scalar);
}


// utility
class DXException {
public :
	DXException() {}
	DXException(HRESULT hresult, const std::wstring& function_name, std::wstring& file_name, int line_num)
		: m_error_code(hresult), m_function_name(function_name), m_file_name(file_name), m_line_num(line_num) {}

	std::wstring To_WStr() {
		_com_error error(m_error_code);
		std::wstring message = error.ErrorMessage();

		return m_function_name + L" failed in " + m_file_name + L"\nline " + std::to_wstring(m_line_num) + L"\nError : " + message;
	}

	HRESULT m_error_code = S_OK;
	std::wstring m_function_name;
	std::wstring m_file_name;
	int m_line_num = -1;
};

inline void D3D_Set_Debug_Name(IDXGIObject* object, char* name) {
	if (object) {
		object->SetPrivateData(WKPDID_D3DDebugObjectName, lstrlenA(name), name);
	}
}

inline void D3D_Set_Debug_Name(ID3D12Device* object, char* name) {
	if (object) {
		object->SetPrivateData(WKPDID_D3DDebugObjectName, lstrlenA(name), name);
	}
}

inline void D3D_Set_Debug_Name(ID3D12DeviceChild* object, char* name) {
	if (object) {
		object->SetPrivateData(WKPDID_D3DDebugObjectName, lstrlenA(name), name);
	}
}

inline std::string WStr_2_Str(const std::wstring& wstr) {
	char str[512];
	WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, str, 512, NULL, NULL);

	return std::string(str);
}

inline std::wstring Str_2_WStr(const std::string& str) {
	WCHAR wstr[512];
	MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, wstr, 512);

	return std::wstring(wstr);
}

inline DirectX::XMMATRIX FbxAMatrix_2_XMMATRIX(const FbxAMatrix& fbx_matrix) {
	DirectX::XMMATRIX xmmatrix = {
		(float)fbx_matrix[0][0], (float)fbx_matrix[0][1], (float)fbx_matrix[0][2], (float)fbx_matrix[0][3],
		(float)fbx_matrix[1][0], (float)fbx_matrix[1][1], (float)fbx_matrix[1][2], (float)fbx_matrix[1][3],
		(float)fbx_matrix[2][0], (float)fbx_matrix[2][1], (float)fbx_matrix[2][2], (float)fbx_matrix[2][3],
		(float)fbx_matrix[3][0], (float)fbx_matrix[3][1], (float)fbx_matrix[3][2], (float)fbx_matrix[3][3]
	};

	return xmmatrix;
}

inline DirectX::XMFLOAT4X4 XMMATRIX_2_XMFLOAT4X4(const DirectX::XMMATRIX& xmmatrix) {
	DirectX::XMFLOAT4X4 xmfloat4x4;
	DirectX::XMStoreFloat4x4(&xmfloat4x4, xmmatrix);

	return xmfloat4x4;
}

inline DirectX::XMFLOAT4 FbxVector4_2_XMFLOAT4(const FbxVector4& fbxvector4) {
	DirectX::XMFLOAT4 xmfloat4 = DirectX::XMFLOAT4(
		(float)fbxvector4[0], (float)fbxvector4[1], (float)fbxvector4[2], (float)fbxvector4[3]);

	return xmfloat4;
}

inline DirectX::XMFLOAT3 FbxVector4_2_XMFLOAT3(const FbxVector4& fbxvector4) {
	DirectX::XMFLOAT3 xmfloat3 = DirectX::XMFLOAT3(
		(float)fbxvector4[0], (float)fbxvector4[1], (float)fbxvector4[2]);

	return xmfloat3;
}

inline DirectX::XMFLOAT4 FbxQuaternion_2_XMFLOAT4(const FbxQuaternion& fbxquaternion) {
	DirectX::XMFLOAT4 xmfloat4 = DirectX::XMFLOAT4(
		(float)fbxquaternion[0], (float)fbxquaternion[1], (float)fbxquaternion[2], (float)fbxquaternion[3]);

	return xmfloat4;
}

#define Throw_If_Failed(x)															\
{																					\
	HRESULT hresult = (x);															\
	std::wstring file_name = Str_2_WStr(__FILE__);									\
	if (FAILED(hresult)) { throw DXException(hresult, L#x, file_name, __LINE__); }	\
}

#define Release_Com(x) { if (x) { x->Release(); x = 0; } }

// info
struct MaterialInfo {
	std::wstring name;

	UINT constant_buffer_index = -1;

	UINT diffuse_heap_index = -1;
	UINT normal_heap_index = -1;

	int dirty_frame_count = FRAME_RESOURCES_NUMBER;

	DirectX::XMFLOAT4 diffuse_albedo = { 1.0f, 1.0f, 1.0f, 1.0f };
	DirectX::XMFLOAT3 fresnel = { 0.01f, 0.01f, 0.01f };
	float roughness = 0.25f;
};

struct LightInfo {
	DirectX::XMFLOAT3 strength = { 0.5f, 0.5f, 0.5f };
	float falloff_start = 1.0f;
	DirectX::XMFLOAT3 direction = { 0.0f, -1.0f, 0.0f };
	float falloff_end = 10.0f;
	DirectX::XMFLOAT3 position = { 0.0f, 0.0f, 0.0f };
	float spot_power = 64.0f;
};

#define MAX_LIGHTS 16

// from d3dx12.h
struct D3D12_DEFAULT {};

struct D3D12_RECT_EX : public D3D12_RECT {
	D3D12_RECT_EX() {}
	~D3D12_RECT_EX() {}
	explicit D3D12_RECT_EX(const D3D12_RECT& object) : D3D12_RECT(object) {}

	explicit D3D12_RECT_EX(
		LONG left,
		LONG top,
		LONG right,
		LONG bottom
	) {
		left = left;
		top = top;
		right = right;
		bottom = bottom;
	}

	operator const D3D12_RECT&() const { return *this; }
};

struct D3D12_BOX_EX : public D3D12_BOX {
	D3D12_BOX_EX() {}
	~D3D12_BOX_EX() {}
	explicit D3D12_BOX_EX(const D3D12_BOX& object) : D3D12_BOX(object) {}

	explicit D3D12_BOX_EX(
		LONG left,
		LONG right
	) {
		left = left;
		top = 0;
		front = 0;
		right = right;
		bottom = 1;
		back = 1;
	}

	explicit D3D12_BOX_EX(
		LONG left,
		LONG top,
		LONG right,
		LONG bottom
	) {
		left = left;
		top = top;
		front = 0;
		right = right;
		bottom = bottom;
		back = 1;
	}

	explicit D3D12_BOX_EX(
		LONG left,
		LONG top,
		LONG front,
		LONG right,
		LONG bottom,
		LONG back
	) {
		left = left;
		top = top;
		front = front;
		right = right;
		bottom = bottom;
		back = back;
	}

	operator const D3D12_BOX&() const { return *this; }
};

inline bool operator==(const D3D12_BOX& lhs, const D3D12_BOX& rhs) {
	return lhs.left == rhs.left &&
		lhs.top == rhs.top &&
		lhs.front == rhs.front &&
		lhs.right == rhs.right &&
		lhs.bottom == rhs.bottom &&
		lhs.back == rhs.back;
}

inline bool operator!=(const D3D12_BOX& lhs, const D3D12_BOX& rhs) {
	return !(lhs == rhs);
}

struct D3D12_DEPTH_STENCIL_DESC_EX : public D3D12_DEPTH_STENCIL_DESC {
	D3D12_DEPTH_STENCIL_DESC_EX() {}
	explicit D3D12_DEPTH_STENCIL_DESC_EX(const D3D12_DEPTH_STENCIL_DESC& object) : D3D12_DEPTH_STENCIL_DESC(object) {}

	explicit D3D12_DEPTH_STENCIL_DESC_EX(D3D12_DEFAULT) {
		DepthEnable = TRUE;
		DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
		DepthFunc = D3D12_COMPARISON_FUNC_LESS;
		StencilEnable = FALSE;
		StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK;
		StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK;

		const D3D12_DEPTH_STENCILOP_DESC default_stencil_option = { D3D12_STENCIL_OP_KEEP, D3D12_STENCIL_OP_KEEP, D3D12_STENCIL_OP_KEEP, D3D12_COMPARISON_FUNC_ALWAYS };
		FrontFace = default_stencil_option;
		BackFace = default_stencil_option;
	}

	explicit D3D12_DEPTH_STENCIL_DESC_EX(
		BOOL depth_enable,
		D3D12_DEPTH_WRITE_MASK depth_write_mask,
		D3D12_COMPARISON_FUNC depth_function,
		BOOL stencil_enable,
		UINT8 stencil_read_mask,
		UINT8 stencil_write_mask,
		D3D12_STENCIL_OP front_stencil_fail_option,
		D3D12_STENCIL_OP front_stencil_depth_fail_option,
		D3D12_STENCIL_OP front_stencil_pass_option,
		D3D12_COMPARISON_FUNC front_stencil_function,
		D3D12_STENCIL_OP back_stencil_fail_option,
		D3D12_STENCIL_OP back_stencil_depth_fail_option,
		D3D12_STENCIL_OP back_stencil_pass_option,
		D3D12_COMPARISON_FUNC back_stencil_function
	) {
		DepthEnable = depth_enable;
		DepthWriteMask = depth_write_mask;
		DepthFunc = depth_function;
		StencilEnable = stencil_enable;
		StencilReadMask = stencil_read_mask;
		StencilWriteMask = stencil_write_mask;
		FrontFace.StencilFailOp = front_stencil_fail_option;
		FrontFace.StencilDepthFailOp = front_stencil_depth_fail_option;
		FrontFace.StencilPassOp = front_stencil_pass_option;
		FrontFace.StencilFunc = front_stencil_function;
		BackFace.StencilFailOp = back_stencil_fail_option;
		BackFace.StencilDepthFailOp = back_stencil_depth_fail_option;
		BackFace.StencilPassOp = back_stencil_pass_option;
		BackFace.StencilFunc = back_stencil_function;
	}

	operator const D3D12_DEPTH_STENCIL_DESC&() const { return *this; }
};

struct D3D12_BLEND_DESC_EX : public D3D12_BLEND_DESC {
	D3D12_BLEND_DESC_EX() {}
	explicit D3D12_BLEND_DESC_EX(const D3D12_BLEND_DESC& object) : D3D12_BLEND_DESC(object) {}

	explicit D3D12_BLEND_DESC_EX(D3D12_DEFAULT) {
		AlphaToCoverageEnable = FALSE;
		IndependentBlendEnable = FALSE;

		const D3D12_RENDER_TARGET_BLEND_DESC default_render_target_blend_desc = {
			FALSE,FALSE,
			D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
			D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
			D3D12_LOGIC_OP_NOOP,
			D3D12_COLOR_WRITE_ENABLE_ALL
		};

		for (UINT i = 0; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; ++i) {
			RenderTarget[i] = default_render_target_blend_desc;
		}
	}

	operator const D3D12_BLEND_DESC&() const { return *this; }
};

struct D3D12_RASTERIZER_DESC_EX : public D3D12_RASTERIZER_DESC {
	D3D12_RASTERIZER_DESC_EX() {}
	explicit D3D12_RASTERIZER_DESC_EX(const D3D12_RASTERIZER_DESC& object) : D3D12_RASTERIZER_DESC(object) {}

	explicit D3D12_RASTERIZER_DESC_EX(D3D12_DEFAULT) {
		FillMode = D3D12_FILL_MODE_SOLID;
		CullMode = D3D12_CULL_MODE_BACK;
		FrontCounterClockwise = FALSE;
		DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
		DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
		SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
		DepthClipEnable = TRUE;
		MultisampleEnable = FALSE;
		AntialiasedLineEnable = FALSE;
		ForcedSampleCount = 0;
		ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
	}

	explicit D3D12_RASTERIZER_DESC_EX(
		D3D12_FILL_MODE fill_mode,
		D3D12_CULL_MODE cull_mode,
		BOOL front_counter_clockwise,
		INT depth_bias,
		FLOAT depth_bias_clamp,
		FLOAT slope_scaled_depth_bias,
		BOOL depth_clip_enable,
		BOOL multisample_enable,
		BOOL antialiased_line_enable,
		UINT forced_sample_count,
		D3D12_CONSERVATIVE_RASTERIZATION_MODE conservative_raster
	) {
		FillMode = fill_mode;
		CullMode = cull_mode;
		FrontCounterClockwise = front_counter_clockwise;
		DepthBias = depth_bias;
		DepthBiasClamp = depth_bias_clamp;
		SlopeScaledDepthBias = slope_scaled_depth_bias;
		DepthClipEnable = depth_clip_enable;
		MultisampleEnable = multisample_enable;
		AntialiasedLineEnable = antialiased_line_enable;
		ForcedSampleCount = forced_sample_count;
		ConservativeRaster = conservative_raster;
	}

	operator const D3D12_RASTERIZER_DESC&() const { return *this; }
};

struct D3D12_HEAP_PROPERTIES_EX : public D3D12_HEAP_PROPERTIES {
	D3D12_HEAP_PROPERTIES_EX() {}
	explicit D3D12_HEAP_PROPERTIES_EX(const D3D12_HEAP_PROPERTIES& object) : D3D12_HEAP_PROPERTIES(object) {}

	D3D12_HEAP_PROPERTIES_EX(
		D3D12_CPU_PAGE_PROPERTY cpu_page_property,
		D3D12_MEMORY_POOL memory_pool,
		UINT creation_nodemask = 1,
		UINT nodemask = 1
	) {
		Type = D3D12_HEAP_TYPE_CUSTOM;
		CPUPageProperty = cpu_page_property;
		MemoryPoolPreference = memory_pool;
		CreationNodeMask = creation_nodemask;
		VisibleNodeMask = nodemask;
	}

	explicit D3D12_HEAP_PROPERTIES_EX(
		D3D12_HEAP_TYPE type,
		UINT creationnodemask = 1,
		UINT nodemask = 1
	) {
		Type = type;
		CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		CreationNodeMask = creationnodemask;
		VisibleNodeMask = nodemask;
	}

	operator const D3D12_HEAP_PROPERTIES& () const { return *this; }

	bool Is_CPU_Accessible() const {
		return Type == D3D12_HEAP_TYPE_UPLOAD || Type == D3D12_HEAP_TYPE_READBACK ||
			(Type == D3D12_HEAP_TYPE_CUSTOM && (CPUPageProperty == D3D12_CPU_PAGE_PROPERTY_WRITE_COMBINE || CPUPageProperty == D3D12_CPU_PAGE_PROPERTY_WRITE_BACK));
	}
};

inline bool operator==(const D3D12_HEAP_PROPERTIES& lhs, const D3D12_HEAP_PROPERTIES& rhs) {
	return lhs.Type == rhs.Type &&
		lhs.CPUPageProperty == rhs.CPUPageProperty &&
		lhs.MemoryPoolPreference == rhs.MemoryPoolPreference &&
		lhs.CreationNodeMask == rhs.CreationNodeMask &&
		lhs.VisibleNodeMask == rhs.VisibleNodeMask;
}

inline bool operator!=(const D3D12_HEAP_PROPERTIES& lhs, const D3D12_HEAP_PROPERTIES& rhs) {
	return !(lhs == rhs);
}

struct D3D12_HEAP_DESC_EX : public D3D12_HEAP_DESC {
	D3D12_HEAP_DESC_EX() {}
	explicit D3D12_HEAP_DESC_EX(const D3D12_HEAP_DESC& object) : D3D12_HEAP_DESC(object) {}

	D3D12_HEAP_DESC_EX(
		UINT64 size,
		D3D12_HEAP_PROPERTIES properties,
		UINT64 alignment = 0,
		D3D12_HEAP_FLAGS flags = D3D12_HEAP_FLAG_NONE
	) {
		SizeInBytes = size;
		Properties = properties;
		Alignment = alignment;
		Flags = flags;
	}

	D3D12_HEAP_DESC_EX(
		UINT64 size,
		D3D12_HEAP_TYPE type,
		UINT64 alignment = 0,
		D3D12_HEAP_FLAGS flags = D3D12_HEAP_FLAG_NONE
	) {
		SizeInBytes = size;
		Properties = D3D12_HEAP_PROPERTIES_EX(type);
		Alignment = alignment;
		Flags = flags;
	}

	D3D12_HEAP_DESC_EX(
		UINT64 size,
		D3D12_CPU_PAGE_PROPERTY cpu_page_property,
		D3D12_MEMORY_POOL memory_pool,
		UINT64 alignment = 0,
		D3D12_HEAP_FLAGS flags = D3D12_HEAP_FLAG_NONE
	) {
		SizeInBytes = size;
		Properties = D3D12_HEAP_PROPERTIES_EX(cpu_page_property, memory_pool);
		Alignment = alignment;
		Flags = flags;
	}

	D3D12_HEAP_DESC_EX(
		const D3D12_RESOURCE_ALLOCATION_INFO& resource_allocation_info,
		D3D12_HEAP_PROPERTIES properties,
		D3D12_HEAP_FLAGS flags = D3D12_HEAP_FLAG_NONE
	) {
		SizeInBytes = resource_allocation_info.SizeInBytes;
		Properties = properties;
		Alignment = resource_allocation_info.Alignment;
		Flags = flags;
	}

	D3D12_HEAP_DESC_EX(
		const D3D12_RESOURCE_ALLOCATION_INFO& resource_allocation_info,
		D3D12_HEAP_TYPE type,
		D3D12_HEAP_FLAGS flags = D3D12_HEAP_FLAG_NONE
	) {
		SizeInBytes = resource_allocation_info.SizeInBytes;
		Properties = D3D12_HEAP_PROPERTIES_EX(type);
		Alignment = resource_allocation_info.Alignment;
		Flags = flags;
	}

	D3D12_HEAP_DESC_EX(
		const D3D12_RESOURCE_ALLOCATION_INFO& resource_alloc_info,
		D3D12_CPU_PAGE_PROPERTY cpu_page_property,
		D3D12_MEMORY_POOL memory_pool,
		D3D12_HEAP_FLAGS flags = D3D12_HEAP_FLAG_NONE
	) {
		SizeInBytes = resource_alloc_info.SizeInBytes;
		Properties = D3D12_HEAP_PROPERTIES_EX(cpu_page_property, memory_pool);
		Alignment = resource_alloc_info.Alignment;
		Flags = flags;
	}

	operator const D3D12_HEAP_DESC& () const { return *this; }

	bool Is_CPU_Accessible() const {
		return ((const D3D12_HEAP_PROPERTIES_EX*)(&Properties))->Is_CPU_Accessible();
	}
};

inline bool operator==(const D3D12_HEAP_DESC& lhs, const D3D12_HEAP_DESC& rhs) {
	return lhs.SizeInBytes == rhs.SizeInBytes &&
		lhs.Properties == rhs.Properties &&
		lhs.Alignment == rhs.Alignment &&
		lhs.Flags == rhs.Flags;
}

inline bool operator!=(const D3D12_HEAP_DESC& lhs, const D3D12_HEAP_DESC& rhs) {
	return !(lhs == rhs);
}

struct D3D12_RESOURCE_BARRIER_EX : public D3D12_RESOURCE_BARRIER {
	D3D12_RESOURCE_BARRIER_EX() {}
	explicit D3D12_RESOURCE_BARRIER_EX(const D3D12_RESOURCE_BARRIER& object) : D3D12_RESOURCE_BARRIER(object) {}

	static inline D3D12_RESOURCE_BARRIER_EX Transition(
		_In_ ID3D12Resource* resource,
		D3D12_RESOURCE_STATES before_state,
		D3D12_RESOURCE_STATES after_state,
		UINT subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES,
		D3D12_RESOURCE_BARRIER_FLAGS flags = D3D12_RESOURCE_BARRIER_FLAG_NONE
	) {
		D3D12_RESOURCE_BARRIER_EX result;
		ZeroMemory(&result, sizeof(result));
		D3D12_RESOURCE_BARRIER& barrier = result;
		result.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		result.Flags = flags;
		barrier.Transition.pResource = resource;
		barrier.Transition.StateBefore = before_state;
		barrier.Transition.StateAfter = after_state;
		barrier.Transition.Subresource = subresource;

		return result;
	}

	static inline D3D12_RESOURCE_BARRIER_EX Aliasing(
		_In_ ID3D12Resource* before_resource,
		_In_ ID3D12Resource* after_resource
	) {
		D3D12_RESOURCE_BARRIER_EX result;
		ZeroMemory(&result, sizeof(result));
		D3D12_RESOURCE_BARRIER& barrier = result;
		result.Type = D3D12_RESOURCE_BARRIER_TYPE_ALIASING;
		barrier.Aliasing.pResourceBefore = before_resource;
		barrier.Aliasing.pResourceAfter = after_resource;

		return result;
	}

	static inline D3D12_RESOURCE_BARRIER_EX UAV(_In_ ID3D12Resource* resource) {
		D3D12_RESOURCE_BARRIER_EX result;
		ZeroMemory(&result, sizeof(result));
		D3D12_RESOURCE_BARRIER& barrier = result;
		result.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
		barrier.UAV.pResource = resource;

		return result;
	}

	operator const D3D12_RESOURCE_BARRIER_EX& () const { return *this; }
};

struct D3D12_TEXTURE_COPY_LOCATION_EX : public D3D12_TEXTURE_COPY_LOCATION {
	D3D12_TEXTURE_COPY_LOCATION_EX() {}
	explicit D3D12_TEXTURE_COPY_LOCATION_EX(const D3D12_TEXTURE_COPY_LOCATION& object) : D3D12_TEXTURE_COPY_LOCATION(object) {}

	D3D12_TEXTURE_COPY_LOCATION_EX(ID3D12Resource* res) { pResource = res; }

	D3D12_TEXTURE_COPY_LOCATION_EX(ID3D12Resource* res, D3D12_PLACED_SUBRESOURCE_FOOTPRINT const& footprint) {
		pResource = res;
		Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
		PlacedFootprint = footprint;
	}

	D3D12_TEXTURE_COPY_LOCATION_EX(ID3D12Resource* res, UINT subresource_index) {
		pResource = res;
		Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
		SubresourceIndex = subresource_index;
	}
};

struct D3D12_ROOT_SIGNATURE_DESC_EX : public D3D12_ROOT_SIGNATURE_DESC {
	D3D12_ROOT_SIGNATURE_DESC_EX() {}
	explicit D3D12_ROOT_SIGNATURE_DESC_EX(const D3D12_ROOT_SIGNATURE_DESC& object) : D3D12_ROOT_SIGNATURE_DESC(object) {}

	D3D12_ROOT_SIGNATURE_DESC_EX(D3D12_DEFAULT) {
		Init(0, NULL, 0, NULL, D3D12_ROOT_SIGNATURE_FLAG_NONE);
	}

	D3D12_ROOT_SIGNATURE_DESC_EX(
		UINT parameters_number,
		_In_reads_opt_(parameters_number) const D3D12_ROOT_PARAMETER* parameters,
		UINT static_samplers_number = 0,
		_In_reads_opt_(static_samplers_number) const D3D12_STATIC_SAMPLER_DESC* static_samplers = NULL,
		D3D12_ROOT_SIGNATURE_FLAGS flags = D3D12_ROOT_SIGNATURE_FLAG_NONE
	) {
		Init(parameters_number, parameters, static_samplers_number, static_samplers, flags);
	}

	inline void Init(
		UINT parameters_number,
		_In_reads_opt_(parameters_number) const D3D12_ROOT_PARAMETER* parameters,
		UINT static_samplers_number = 0,
		_In_reads_opt_(static_samplers_number) const D3D12_STATIC_SAMPLER_DESC* static_samplers = NULL,
		D3D12_ROOT_SIGNATURE_FLAGS flags = D3D12_ROOT_SIGNATURE_FLAG_NONE
	) {
		Init(*this, parameters_number, parameters, static_samplers_number, static_samplers, flags);
	}

	static inline void Init(
		_Out_ D3D12_ROOT_SIGNATURE_DESC& desc,
		UINT parameters_number,
		_In_reads_opt_(parameters_number) const D3D12_ROOT_PARAMETER* parameters,
		UINT static_samplers_number = 0,
		_In_reads_opt_(static_samplers_number) const D3D12_STATIC_SAMPLER_DESC* static_samplers = NULL,
		D3D12_ROOT_SIGNATURE_FLAGS flags = D3D12_ROOT_SIGNATURE_FLAG_NONE
	) {
		desc.NumParameters = parameters_number;
		desc.pParameters = parameters;
		desc.NumStaticSamplers = static_samplers_number;
		desc.pStaticSamplers = static_samplers;
		desc.Flags = flags;
	}
};

struct D3D12_CPU_DESCRIPTOR_HANDLE_EX : public D3D12_CPU_DESCRIPTOR_HANDLE {
	D3D12_CPU_DESCRIPTOR_HANDLE_EX() {}
	explicit D3D12_CPU_DESCRIPTOR_HANDLE_EX(const D3D12_CPU_DESCRIPTOR_HANDLE& object) : D3D12_CPU_DESCRIPTOR_HANDLE(object) {}

	D3D12_CPU_DESCRIPTOR_HANDLE_EX(D3D12_DEFAULT) { ptr = 0; }

	D3D12_CPU_DESCRIPTOR_HANDLE_EX(_In_ const D3D12_CPU_DESCRIPTOR_HANDLE& other, INT offset_scaled) {
		Set_Offset(other, offset_scaled);
	}

	D3D12_CPU_DESCRIPTOR_HANDLE_EX(_In_ const D3D12_CPU_DESCRIPTOR_HANDLE& other, INT offset, UINT increment_size) {
		Set_Offset(other, offset, increment_size);
	}

	D3D12_CPU_DESCRIPTOR_HANDLE_EX& Get_By_Offset(INT offset_scaled) {
		ptr += offset_scaled;

		return *this;
	}

	D3D12_CPU_DESCRIPTOR_HANDLE_EX& Get_By_Offset(INT offset, UINT increment_size) {
		ptr += offset * increment_size;

		return *this;
	}

	bool operator==(_In_ const D3D12_CPU_DESCRIPTOR_HANDLE& other) {
		return (ptr == other.ptr);
	}

	bool operator!=(_In_ const D3D12_CPU_DESCRIPTOR_HANDLE& other) {
		return (ptr != other.ptr);
	}

	D3D12_CPU_DESCRIPTOR_HANDLE_EX& operator=(const D3D12_CPU_DESCRIPTOR_HANDLE& other) {
		ptr = other.ptr;

		return *this;
	}

	inline void Set_Offset(_In_ const D3D12_CPU_DESCRIPTOR_HANDLE& base, INT offset_scaled) {
		Set_Offset(*this, base, offset_scaled);
	}

	inline void Set_Offset(_In_ const D3D12_CPU_DESCRIPTOR_HANDLE& base, INT offset, UINT increment_size) {
		Set_Offset(*this, base, offset, increment_size);
	}

	static inline void Set_Offset(_Out_ D3D12_CPU_DESCRIPTOR_HANDLE& handle, _In_ const D3D12_CPU_DESCRIPTOR_HANDLE& base, INT offset_scaled) {
		handle.ptr = base.ptr + offset_scaled;
	}

	static inline void Set_Offset(_Out_ D3D12_CPU_DESCRIPTOR_HANDLE& handle, _In_ const D3D12_CPU_DESCRIPTOR_HANDLE& base, INT offset, UINT increment_size) {
		handle.ptr = base.ptr + offset * increment_size;
	}
};

struct D3D12_GPU_DESCRIPTOR_HANDLE_EX : public D3D12_GPU_DESCRIPTOR_HANDLE {
	D3D12_GPU_DESCRIPTOR_HANDLE_EX() {}
	explicit D3D12_GPU_DESCRIPTOR_HANDLE_EX(const D3D12_GPU_DESCRIPTOR_HANDLE& object) : D3D12_GPU_DESCRIPTOR_HANDLE(object) {}

	D3D12_GPU_DESCRIPTOR_HANDLE_EX(D3D12_DEFAULT) { ptr = 0; }

	D3D12_GPU_DESCRIPTOR_HANDLE_EX(_In_ const D3D12_GPU_DESCRIPTOR_HANDLE& other, INT offset_scaled) {
		Set_Offset(other, offset_scaled);
	}

	D3D12_GPU_DESCRIPTOR_HANDLE_EX(_In_ const D3D12_GPU_DESCRIPTOR_HANDLE& other, INT offset, UINT increment_size) {
		Set_Offset(other, offset, increment_size);
	}

	D3D12_GPU_DESCRIPTOR_HANDLE_EX& Get_By_Offset(INT offset, UINT increment_size) {
		ptr += offset * increment_size;

		return *this;
	}

	D3D12_GPU_DESCRIPTOR_HANDLE_EX& Get_By_Offset(INT offset_scaled) {
		ptr += offset_scaled;

		return *this;
	}

	inline bool operator==(_In_ const D3D12_GPU_DESCRIPTOR_HANDLE& other) {
		return (ptr == other.ptr);
	}

	inline bool operator!=(_In_ const D3D12_GPU_DESCRIPTOR_HANDLE& other) {
		return (ptr != other.ptr);
	}

	D3D12_GPU_DESCRIPTOR_HANDLE_EX& operator=(const D3D12_GPU_DESCRIPTOR_HANDLE& other) {
		ptr = other.ptr;

		return *this;
	}

	inline void Set_Offset(_In_ const D3D12_GPU_DESCRIPTOR_HANDLE& base, INT offset_scaled) {
		Set_Offset(*this, base, offset_scaled);
	}

	inline void Set_Offset(_In_ const D3D12_GPU_DESCRIPTOR_HANDLE& base, INT offset, UINT increment_size) {
		Set_Offset(*this, base, offset, increment_size);
	}

	static inline void Set_Offset(_Out_ D3D12_GPU_DESCRIPTOR_HANDLE& handle, _In_ const D3D12_GPU_DESCRIPTOR_HANDLE& base, INT offset_scaled) {
		handle.ptr = base.ptr + offset_scaled;
	}

	static inline void Set_Offset(_Out_ D3D12_GPU_DESCRIPTOR_HANDLE& handle, _In_ const D3D12_GPU_DESCRIPTOR_HANDLE& base, INT offset, UINT increment_size) {
		handle.ptr = base.ptr + offset * increment_size;
	}
};

struct D3D12_RESOURCE_DESC_EX_HELPER {
	static inline UINT Calc_Subresource(UINT mip_slice, UINT array_slice, UINT plane_slice, UINT mip_levels, UINT array_size) {
		return mip_slice + array_slice * mip_levels + plane_slice * mip_levels * array_size;
	}

	static inline UINT8 Get_Format_Plane_Count(_In_ ID3D12Device* device, DXGI_FORMAT format) {
		D3D12_FEATURE_DATA_FORMAT_INFO format_info = { format };

		if (FAILED(device->CheckFeatureSupport(D3D12_FEATURE_FORMAT_INFO, &format_info, sizeof(format_info)))) {
			return 0;
		}

		return format_info.PlaneCount;
	}
};

struct D3D12_RESOURCE_DESC_EX : public D3D12_RESOURCE_DESC {
	D3D12_RESOURCE_DESC_EX() {}
	explicit D3D12_RESOURCE_DESC_EX(const D3D12_RESOURCE_DESC& object) : D3D12_RESOURCE_DESC(object) {}

	D3D12_RESOURCE_DESC_EX(
		D3D12_RESOURCE_DIMENSION dimension,
		UINT64 alignment,
		UINT64 width,
		UINT height,
		UINT16 depth_or_array_size,
		UINT16 mip_levels,
		DXGI_FORMAT format,
		UINT sample_count,
		UINT sample_quality,
		D3D12_TEXTURE_LAYOUT layout,
		D3D12_RESOURCE_FLAGS flags
	) {
		Dimension = dimension;
		Alignment = alignment;
		Width = width;
		Height = height;
		DepthOrArraySize = depth_or_array_size;
		MipLevels = mip_levels;
		Format = format;
		SampleDesc.Count = sample_count;
		SampleDesc.Quality = sample_quality;
		Layout = layout;
		Flags = flags;
	}

	static inline D3D12_RESOURCE_DESC_EX Buffer(const D3D12_RESOURCE_ALLOCATION_INFO& resource_alloc_info, D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE) {
		return D3D12_RESOURCE_DESC_EX(D3D12_RESOURCE_DIMENSION_BUFFER, resource_alloc_info.Alignment, resource_alloc_info.SizeInBytes,
			1, 1, 1, DXGI_FORMAT_UNKNOWN, 1, 0, D3D12_TEXTURE_LAYOUT_ROW_MAJOR, flags);
	}

	static inline D3D12_RESOURCE_DESC_EX Buffer(UINT64 width, D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE, UINT64 alignment = 0) {
		return D3D12_RESOURCE_DESC_EX(D3D12_RESOURCE_DIMENSION_BUFFER, alignment, width, 1, 1, 1, DXGI_FORMAT_UNKNOWN, 1, 0, D3D12_TEXTURE_LAYOUT_ROW_MAJOR, flags);
	}

	static inline D3D12_RESOURCE_DESC_EX Texture_1D(
		DXGI_FORMAT format,
		UINT64 width,
		UINT16 array_size = 1,
		UINT16 mip_levels = 0,
		D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE,
		D3D12_TEXTURE_LAYOUT layout = D3D12_TEXTURE_LAYOUT_UNKNOWN,
		UINT64 alignment = 0
	) {
		return D3D12_RESOURCE_DESC_EX(D3D12_RESOURCE_DIMENSION_TEXTURE1D, alignment, width, 1, array_size, mip_levels, format, 1, 0, layout, flags);
	}

	static inline D3D12_RESOURCE_DESC_EX Texture_2D(
		DXGI_FORMAT format,
		UINT64 width,
		UINT height,
		UINT16 array_size = 1,
		UINT16 mip_levels = 0,
		UINT sample_count = 1,
		UINT sample_quality = 0,
		D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE,
		D3D12_TEXTURE_LAYOUT layout = D3D12_TEXTURE_LAYOUT_UNKNOWN,
		UINT64 alignment = 0
	) {
		return D3D12_RESOURCE_DESC_EX(D3D12_RESOURCE_DIMENSION_TEXTURE2D, alignment, width, height, array_size, mip_levels, format, sample_count, sample_quality, layout, flags);
	}

	static inline D3D12_RESOURCE_DESC_EX Texture_3D(
		DXGI_FORMAT format,
		UINT64 width,
		UINT height,
		UINT16 depth,
		UINT16 mip_levels = 0,
		D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE,
		D3D12_TEXTURE_LAYOUT layout = D3D12_TEXTURE_LAYOUT_UNKNOWN,
		UINT64 alignment = 0
	) {
		return D3D12_RESOURCE_DESC_EX(D3D12_RESOURCE_DIMENSION_TEXTURE3D, alignment, width, height, depth, mip_levels, format, 1, 0, layout, flags);
	}

	inline UINT16 Get_Depth() const {
		return (Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE3D ? DepthOrArraySize : 1);
	}

	inline UINT16 Get_Array_Size() const {
		return (Dimension != D3D12_RESOURCE_DIMENSION_TEXTURE3D ? DepthOrArraySize : 1);
	}

	inline UINT8 Get_Plane_Count(_In_ ID3D12Device* device) const {
		return D3D12_RESOURCE_DESC_EX_HELPER::Get_Format_Plane_Count(device, Format);
	}

	inline UINT Subresources(_In_ ID3D12Device* device) const {
		return MipLevels * Get_Array_Size() * Get_Plane_Count(device);
	}

	inline UINT Calc_Subresource(UINT mip_slice, UINT array_slice, UINT plane_slice) {
		return D3D12_RESOURCE_DESC_EX_HELPER::Calc_Subresource(mip_slice, array_slice, plane_slice, MipLevels, Get_Array_Size());
	}

	operator const D3D12_RESOURCE_DESC& () const { return *this; }
};

inline bool operator==(const D3D12_RESOURCE_DESC& lhs, const D3D12_RESOURCE_DESC& rhs) {
	return lhs.Dimension == rhs.Dimension &&
		lhs.Alignment == rhs.Alignment &&
		lhs.Width == rhs.Width &&
		lhs.Height == rhs.Height &&
		lhs.DepthOrArraySize == rhs.DepthOrArraySize &&
		lhs.MipLevels == rhs.MipLevels &&
		lhs.Format == rhs.Format &&
		lhs.SampleDesc.Count == rhs.SampleDesc.Count &&
		lhs.SampleDesc.Quality == rhs.SampleDesc.Quality &&
		lhs.Layout == rhs.Layout &&
		lhs.Flags == rhs.Flags;
}

inline bool operator!=(const D3D12_RESOURCE_DESC& lhs, const D3D12_RESOURCE_DESC& rhs) {
	return !(lhs == rhs);
}

struct D3D12_DESCRIPTOR_RANGE_EX : public D3D12_DESCRIPTOR_RANGE {
	D3D12_DESCRIPTOR_RANGE_EX() {}
	explicit D3D12_DESCRIPTOR_RANGE_EX(const D3D12_DESCRIPTOR_RANGE& object) : D3D12_DESCRIPTOR_RANGE(object) {}

	D3D12_DESCRIPTOR_RANGE_EX(
		D3D12_DESCRIPTOR_RANGE_TYPE range_type,
		UINT descriptors_number,
		UINT base_shader_register,
		UINT register_space = 0,
		UINT offset = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND
	) {
		Init(range_type, descriptors_number, base_shader_register, register_space, offset);
	}

	inline void Init(
		D3D12_DESCRIPTOR_RANGE_TYPE range_type,
		UINT descriptors_number,
		UINT base_shader_register,
		UINT register_space = 0,
		UINT offset = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND
	) {
		Init(*this, range_type, descriptors_number, base_shader_register, register_space, offset);
	}

	static inline void Init(
		_Out_ D3D12_DESCRIPTOR_RANGE& range,
		D3D12_DESCRIPTOR_RANGE_TYPE range_type,
		UINT descriptors_number,
		UINT base_shader_register,
		UINT register_space = 0,
		UINT offset = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND
	) {
		range.RangeType = range_type;
		range.NumDescriptors = descriptors_number;
		range.BaseShaderRegister = base_shader_register;
		range.RegisterSpace = register_space;
		range.OffsetInDescriptorsFromTableStart = offset;
	}
};

struct D3D12_ROOT_DESCRIPTOR_TABLE_EX : public D3D12_ROOT_DESCRIPTOR_TABLE {
	D3D12_ROOT_DESCRIPTOR_TABLE_EX() {}
	explicit D3D12_ROOT_DESCRIPTOR_TABLE_EX(const D3D12_ROOT_DESCRIPTOR_TABLE& object) : D3D12_ROOT_DESCRIPTOR_TABLE(object) {}

	D3D12_ROOT_DESCRIPTOR_TABLE_EX(
		UINT descriptor_ranges_number,
		_In_reads_opt_(descriptor_ranges_number) const D3D12_DESCRIPTOR_RANGE* descriptor_ranges
	) {
		Init(descriptor_ranges_number, descriptor_ranges);
	}

	inline void Init(
		UINT descriptor_ranges_number,
		_In_reads_(descriptor_ranges_number) const D3D12_DESCRIPTOR_RANGE* descriptor_ranges
	) {
		Init(*this, descriptor_ranges_number, descriptor_ranges);
	}

	static inline void Init(
		_Out_ D3D12_ROOT_DESCRIPTOR_TABLE& root_descriptor_table,
		UINT descriptor_ranges_number,
		_In_reads_opt_(descriptor_ranges_number) const D3D12_DESCRIPTOR_RANGE* descriptor_ranges
	) {
		root_descriptor_table.NumDescriptorRanges = descriptor_ranges_number;
		root_descriptor_table.pDescriptorRanges = descriptor_ranges;
	}
};

struct D3D12_ROOT_CONSTANTS_EX : public D3D12_ROOT_CONSTANTS {
	D3D12_ROOT_CONSTANTS_EX() {}
	explicit D3D12_ROOT_CONSTANTS_EX(const D3D12_ROOT_CONSTANTS& object) : D3D12_ROOT_CONSTANTS(object) {}

	D3D12_ROOT_CONSTANTS_EX(
		UINT values_number,
		UINT shader_register,
		UINT register_space = 0
	) {
		Init(values_number, shader_register, register_space);
	}

	inline void Init(
		UINT values_number,
		UINT shader_register,
		UINT register_space = 0
	) {
		Init(*this, values_number, shader_register, register_space);
	}

	static inline void Init(
		_Out_ D3D12_ROOT_CONSTANTS& root_constants,
		UINT values_number,
		UINT shader_register,
		UINT register_space = 0
	) {
		root_constants.Num32BitValues = values_number;
		root_constants.ShaderRegister = shader_register;
		root_constants.RegisterSpace = register_space;
	}
};

struct D3D12_ROOT_DESCRIPTOR_EX : public D3D12_ROOT_DESCRIPTOR {
	D3D12_ROOT_DESCRIPTOR_EX() {}
	explicit D3D12_ROOT_DESCRIPTOR_EX(D3D12_ROOT_DESCRIPTOR& object) : D3D12_ROOT_DESCRIPTOR(object) {}

	D3D12_ROOT_DESCRIPTOR_EX(
		UINT shader_register,
		UINT register_space = 0
	) {
		Init(shader_register, register_space);
	}

	inline void Init(
		UINT shader_register,
		UINT register_space = 0
	) {
		Init(*this, shader_register, register_space);
	}

	static inline void Init(
		_Out_ D3D12_ROOT_DESCRIPTOR& table,
		UINT shader_register,
		UINT register_space = 0
	) {
		table.ShaderRegister = shader_register;
		table.RegisterSpace = register_space;
	}
};

struct D3D12_ROOT_PARAMETER_EX : public D3D12_ROOT_PARAMETER {
	D3D12_ROOT_PARAMETER_EX() {}
	explicit D3D12_ROOT_PARAMETER_EX(const D3D12_ROOT_PARAMETER& object) : D3D12_ROOT_PARAMETER(object) {}

	static inline void Init_As_DT(	// Initialize As Descriptor Table
		_Out_ D3D12_ROOT_PARAMETER& root_param,
		UINT descriptor_ranges_number,
		_In_reads_(descriptor_ranges_number) const D3D12_DESCRIPTOR_RANGE* descriptor_ranges,
		D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL
	) {
		root_param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		root_param.ShaderVisibility = visibility;
		D3D12_ROOT_DESCRIPTOR_TABLE_EX::Init(root_param.DescriptorTable, descriptor_ranges_number, descriptor_ranges);
	}

	static inline void Init_As_C(	// Initialize As Constants
		_Out_ D3D12_ROOT_PARAMETER& root_param,
		UINT values_number,
		UINT shader_register,
		UINT register_space = 0,
		D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL
	) {
		root_param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
		root_param.ShaderVisibility = visibility;
		D3D12_ROOT_CONSTANTS_EX::Init(root_param.Constants, values_number, shader_register, register_space);
	}

	static inline void Init_As_CBV(	// Initialize As Constant Buffer View
		_Out_ D3D12_ROOT_PARAMETER& root_param,
		UINT shader_register,
		UINT register_space = 0,
		D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL
	) {
		root_param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
		root_param.ShaderVisibility = visibility;
		D3D12_ROOT_DESCRIPTOR_EX::Init(root_param.Descriptor, shader_register, register_space);
	}

	static inline void Init_As_SRV(	// Initialize As Shader Resource View
		_Out_ D3D12_ROOT_PARAMETER& root_param,
		UINT shader_register,
		UINT register_space = 0,
		D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL
	) {
		root_param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
		root_param.ShaderVisibility = visibility;
		D3D12_ROOT_DESCRIPTOR_EX::Init(root_param.Descriptor, shader_register, register_space);
	}

	static inline void Init_As_UAV(	// Initialize As Unordered Access View
		_Out_ D3D12_ROOT_PARAMETER& root_param,
		UINT shader_register,
		UINT register_space = 0,
		D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL
	) {
		root_param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_UAV;
		root_param.ShaderVisibility = visibility;
		D3D12_ROOT_DESCRIPTOR_EX::Init(root_param.Descriptor, shader_register, register_space);
	}

	inline void Init_As_DT(	// Initialize As Descriptor Table
		UINT descriptor_ranges_number,
		_In_reads_(descriptor_ranges_number) const D3D12_DESCRIPTOR_RANGE* descriptor_ranges,
		D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL
	) {
		Init_As_DT(*this, descriptor_ranges_number, descriptor_ranges, visibility);
	}

	inline void Init_As_C(	// Initialize As Constants
		UINT values_number,
		UINT shader_register,
		UINT register_space = 0,
		D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL
	) {
		Init_As_C(*this, values_number, shader_register, register_space);
	}

	inline void Init_As_CBV(	// Initialize As Constant Buffer View
		UINT shader_register,
		UINT register_space = 0,
		D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL
	) {
		Init_As_CBV(*this, shader_register, register_space);
	}

	inline void Init_As_SRV(	// Initialize As Shader Resource View
		UINT shader_register,
		UINT register_space = 0,
		D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL
	) {
		Init_As_SRV(*this, shader_register, register_space);
	}

	inline void Init_As_UAV(	// Initialize As Unordered Access View
		UINT shader_register,
		UINT register_space = 0,
		D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL
	) {
		Init_As_UAV(*this, shader_register, register_space);
	}
};

//
inline void Memcpy_Subresource(
	_In_ const D3D12_MEMCPY_DEST* destination,
	_In_ const D3D12_SUBRESOURCE_DATA* source,
	SIZE_T row_size,
	UINT num_rows,
	UINT num_slices
) {
	for (UINT z = 0; z < num_slices; ++z) {
		BYTE* destination_slice = reinterpret_cast<BYTE*>(destination->pData) + destination->SlicePitch * z;
		const BYTE* source_slice = reinterpret_cast<const BYTE*>(source->pData) + source->SlicePitch * z;

		for (UINT y = 0; y < num_rows; ++y) {
			memcpy(destination_slice + destination->RowPitch * y, source_slice + source->RowPitch * y, row_size);
		}
	}
}

inline UINT64 Udt_Subresource(
	_In_ ID3D12GraphicsCommandList* command_list,
	_In_ ID3D12Resource* destination_resource,
	_In_ ID3D12Resource* intermediate,
	_In_range_(0, D3D12_REQ_SUBRESOURCES) UINT first_subresource,
	_In_range_(0, D3D12_REQ_SUBRESOURCES - FirstSubresource) UINT num_subresources,
	UINT64 required_size,
	_In_reads_(NumSubresources) const D3D12_PLACED_SUBRESOURCE_FOOTPRINT* layouts,
	_In_reads_(NumSubresources) const UINT* num_rows,
	_In_reads_(NumSubresources) const UINT64* row_sizes,
	_In_reads_(NumSubresources) const D3D12_SUBRESOURCE_DATA* source_data
) {
	// Minor validation
	D3D12_RESOURCE_DESC intermediate_desc = intermediate->GetDesc();
	D3D12_RESOURCE_DESC destination_desc = destination_resource->GetDesc();
	if (intermediate_desc.Dimension != D3D12_RESOURCE_DIMENSION_BUFFER ||
		intermediate_desc.Width < required_size + layouts[0].Offset ||
		required_size >(SIZE_T) - 1 ||
		(destination_desc.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER && (first_subresource != 0 || num_subresources != 1))
		) {
		return 0;
	}

	BYTE* data;
	HRESULT hresult = intermediate->Map(0, NULL, reinterpret_cast<void**>(&data));

	if (FAILED(hresult)) {
		return 0;
	}

	for (UINT i = 0; i < num_subresources; ++i) {
		if (row_sizes[i] > (SIZE_T)-1) return 0;

		D3D12_MEMCPY_DEST destination_data = { data + layouts[i].Offset, layouts[i].Footprint.RowPitch, layouts[i].Footprint.RowPitch * num_rows[i] };
		Memcpy_Subresource(&destination_data, &source_data[i], (SIZE_T)row_sizes[i], num_rows[i], layouts[i].Footprint.Depth);
	}

	intermediate->Unmap(0, NULL);

	if (destination_desc.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER) {
		D3D12_BOX_EX source_box(UINT(layouts[0].Offset), UINT(layouts[0].Offset + layouts[0].Footprint.Width));
		command_list->CopyBufferRegion( destination_resource, 0, intermediate, layouts[0].Offset, layouts[0].Footprint.Width);
	}
	else {
		for (UINT i = 0; i < num_subresources; ++i) {
			D3D12_TEXTURE_COPY_LOCATION_EX destination(destination_resource, i + first_subresource);
			D3D12_TEXTURE_COPY_LOCATION_EX source(intermediate, layouts[i]);
			command_list->CopyTextureRegion(&destination, 0, 0, 0, &source, nullptr);
		}
	}

	return required_size;
}

inline UINT64 Udt_Subresource(
	_In_ ID3D12GraphicsCommandList* command_list,
	_In_ ID3D12Resource* destination_resource,
	_In_ ID3D12Resource* intermediate,
	UINT64 intermediate_offset,
	_In_range_(0, D3D12_REQ_SUBRESOURCES) UINT first_subresource,
	_In_range_(0, D3D12_REQ_SUBRESOURCES - first_subresource) UINT num_subresources,
	_In_reads_(num_subresources) D3D12_SUBRESOURCE_DATA* source_data
) {
	UINT64 required_size = 0;
	UINT64 memory_to_allocate = static_cast<UINT64>(sizeof(D3D12_PLACED_SUBRESOURCE_FOOTPRINT) + sizeof(UINT) + sizeof(UINT64)) * num_subresources;

	if (memory_to_allocate > SIZE_MAX) {
		return 0;
	}

	void* memory = HeapAlloc(GetProcessHeap(), 0, static_cast<SIZE_T>(memory_to_allocate));

	if (memory == NULL) {
		return 0;
	}

	D3D12_PLACED_SUBRESOURCE_FOOTPRINT* layouts = reinterpret_cast<D3D12_PLACED_SUBRESOURCE_FOOTPRINT*>(memory);
	UINT64* row_sizes = reinterpret_cast<UINT64*>(layouts + num_subresources);
	UINT* num_rows = reinterpret_cast<UINT*>(row_sizes + num_subresources);

	D3D12_RESOURCE_DESC description = destination_resource->GetDesc();
	ID3D12Device* device;
	destination_resource->GetDevice(__uuidof(*device), reinterpret_cast<void**>(&device));
	device->GetCopyableFootprints(&description, first_subresource, num_subresources, intermediate_offset, layouts, num_rows, row_sizes, &required_size);
	device->Release();

	UINT64 Result = Udt_Subresource(command_list, destination_resource, intermediate, first_subresource, num_subresources, required_size, layouts, num_rows, row_sizes, source_data);
	HeapFree(GetProcessHeap(), 0, memory);
	return Result;
}

template <UINT max_subresources>
inline UINT64 Udt_Subresource(
	_In_ ID3D12GraphicsCommandList* command_lsit,
	_In_ ID3D12Resource* destination_resource,
	_In_ ID3D12Resource* intermediate,
	UINT64 intermediate_offset,
	_In_range_(0, max_subresources) UINT first_subresource,
	_In_range_(1, max_subresources - first_subresource) UINT num_subresources,
	_In_reads_(num_subresources) D3D12_SUBRESOURCE_DATA* source_data
) {
	UINT64 RequiredSize = 0;
	D3D12_PLACED_SUBRESOURCE_FOOTPRINT Layouts[max_subresources];
	UINT NumRows[max_subresources];
	UINT64 RowSizesInBytes[max_subresources];

	D3D12_RESOURCE_DESC Desc = destination_resource->GetDesc();
	ID3D12Device* pDevice;
	destination_resource->GetDevice(__uuidof(*pDevice), reinterpret_cast<void**>(&pDevice));
	pDevice->GetCopyableFootprints(&Desc, first_subresource, num_subresources, intermediate_offset, Layouts, NumRows, RowSizesInBytes, &RequiredSize);
	pDevice->Release();

	return Udt_Subresource(command_lsit, destination_resource, intermediate, first_subresource, num_subresources, RequiredSize, Layouts, NumRows, RowSizesInBytes, source_data);
}

// utility
inline UINT Calc_CB_Size(UINT size) {	// Calculate Constant Buffer Size
	return (size + 255) & ~255;
}

inline Microsoft::WRL::ComPtr<ID3D12Resource> Crt_DB(	// Create Default Buffer
	ID3D12Device* device,
	ID3D12GraphicsCommandList* command_list,
	void* data,
	UINT64 size,
	Microsoft::WRL::ComPtr<ID3D12Resource>& upload_buffer
) {
	Microsoft::WRL::ComPtr<ID3D12Resource> default_buffer;

	Throw_If_Failed(device->CreateCommittedResource(&D3D12_HEAP_PROPERTIES_EX(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE,
		&D3D12_RESOURCE_DESC_EX::Buffer(size), D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(default_buffer.GetAddressOf())));

	Throw_If_Failed(device->CreateCommittedResource(&D3D12_HEAP_PROPERTIES_EX(D3D12_HEAP_TYPE_UPLOAD), D3D12_HEAP_FLAG_NONE,
		&D3D12_RESOURCE_DESC_EX::Buffer(size), D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(upload_buffer.GetAddressOf())));

	D3D12_SUBRESOURCE_DATA subresource_data;
	subresource_data.pData = data;
	subresource_data.RowPitch = size;
	subresource_data.SlicePitch = subresource_data.RowPitch;

	command_list->ResourceBarrier(1, &D3D12_RESOURCE_BARRIER_EX::Transition(default_buffer.Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST));

	Udt_Subresource<1>(command_list, default_buffer.Get(), upload_buffer.Get(), 0, 0, 1, &subresource_data);

	command_list->ResourceBarrier(1, &D3D12_RESOURCE_BARRIER_EX::Transition(default_buffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ));

	return default_buffer;
}

inline Microsoft::WRL::ComPtr<ID3DBlob> Compile_Shader(
	const std::wstring& file_name,
	const D3D_SHADER_MACRO* defines,
	const std::string& entry_point,
	const std::string& target
) {
	UINT compiler_flags = 0;

#if defined(DEBUG) || defined(_DEBUG)
	compiler_flags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

	HRESULT hr = S_OK;

	Microsoft::WRL::ComPtr<ID3DBlob> code_blob = nullptr;
	Microsoft::WRL::ComPtr<ID3DBlob> error_blob = nullptr;

	hr = D3DCompileFromFile(file_name.c_str(), defines, D3D_COMPILE_STANDARD_FILE_INCLUDE, entry_point.c_str(), target.c_str(), compiler_flags, 0, &code_blob, &error_blob);

	if (error_blob != nullptr) {
		OutputDebugStringA((char*)error_blob->GetBufferPointer());
	}

	Throw_If_Failed(hr);

	return code_blob;
}

// _Info
struct Material_Factor {
	DirectX::XMFLOAT4 diffuse_albedo = DirectX::XMFLOAT4(DirectX::Colors::LightBlue);
	DirectX::XMFLOAT3 fresnel = { 0.1f, 0.1f, 0.1f };
	float shininess = 0.25f;
};

struct Material_Info {
	std::wstring name;

	int dirty_count = FRAME_RESOURCES_NUMBER;

	Material_Factor material_factor;

	Material_Info() {}
	Material_Info(std::wstring material_info_name, Material_Factor material_factor_in) {
		name = material_info_name;
		material_factor = material_factor_in;
	}

	void Rst_Dirty_Count() {
		dirty_count = FRAME_RESOURCES_NUMBER;
	}
};

struct Material {
	std::wstring name;

	UINT constant_buffer_index = -1;

	UINT diffuse_heap_index = -1;
	UINT normal_heap_index = -1;

	std::vector<Material_Info*> material_info_array;

	std::array<Material_Factor, MAX_MATERIAL_COUNT> material_factor_array;

	//
	Material() {}
	Material(UINT constant_buffer_index_in, std::wstring material_name, std::vector<Material_Info*>& material_info_array_in) {
		constant_buffer_index = constant_buffer_index_in;
		name = material_name;

		material_info_array.assign(material_info_array_in.begin(), material_info_array_in.end());
	}

	void Udt_Material_Factors() {
		for (size_t i = 0; i < material_info_array.size(); ++i) {
			material_factor_array[i] = material_info_array[i]->material_factor;
		}
	}

	std::array<Material_Factor, MAX_MATERIAL_COUNT>& Get_Material_Factors() {
		return material_factor_array;
	}
};

struct Vertex_Info {
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT3 normal;
	DirectX::XMFLOAT3 tangent;
	DirectX::XMFLOAT2 uv;
	UINT bone_count = 0;
	UINT bone_indices[MAX_WEIGHT_BONE_COUNT] = { (UINT)-1, (UINT)-1, (UINT)-1, (UINT)-1 };
	float bone_weights[MAX_WEIGHT_BONE_COUNT];
	UINT material_index = 0;
};

struct Mesh_Info {
	std::vector<Vertex_Info> vertices;

	std::vector<std::uint16_t> indices_16;
	std::vector<std::uint32_t> indices_32;
	UINT index_count = 0;

	Microsoft::WRL::ComPtr<ID3DBlob> vertex_buffer_cpu = nullptr;
	Microsoft::WRL::ComPtr<ID3DBlob> index_buffer_cpu = nullptr;

	Microsoft::WRL::ComPtr<ID3D12Resource> vertex_buffer_gpu = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> index_buffer_gpu = nullptr;

	Microsoft::WRL::ComPtr<ID3D12Resource> vertex_upload_buffer = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> index_upload_buffer = nullptr;

	UINT vertex_buffer_stride = 0;
	UINT vertex_buffer_size = 0;

	DXGI_FORMAT index_format = DXGI_FORMAT_R32_UINT;
	UINT index_buffer_size = 0;

	D3D12_PRIMITIVE_TOPOLOGY primitive_topology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	//
	Material* material = nullptr;

	//
	Mesh_Info() {}
	Mesh_Info(std::vector<Vertex_Info>& vertex_array, std::vector<std::uint32_t>& index_array_32) {
		vertices.assign(vertex_array.begin(), vertex_array.end());
		indices_32.assign(index_array_32.begin(), index_array_32.end());
	}

	void Add_Info(std::vector<Vertex_Info>& vertex_array, std::vector<std::uint32_t>& index_array_32) {
		size_t vertex_count = vertices.size();

		vertices.insert(vertices.end(), vertex_array.begin(), vertex_array.end());

		size_t index_count = indices_32.size();
		for (size_t i = 0; i < index_array_32.size(); ++i) {
			indices_32.emplace_back((std::uint32_t)(vertex_count + index_array_32[i]));
		}
	}

	D3D12_VERTEX_BUFFER_VIEW Get_VBV() const {	// Get Vertex Buffer View
		D3D12_VERTEX_BUFFER_VIEW vertex_buffer_view;
		vertex_buffer_view.BufferLocation = vertex_buffer_gpu->GetGPUVirtualAddress();
		vertex_buffer_view.StrideInBytes = vertex_buffer_stride;
		vertex_buffer_view.SizeInBytes = vertex_buffer_size;

		return vertex_buffer_view;
	}

	D3D12_INDEX_BUFFER_VIEW Get_IBV() const {	// Get Index Buffer View
		D3D12_INDEX_BUFFER_VIEW index_buffer_view;
		index_buffer_view.BufferLocation = index_buffer_gpu->GetGPUVirtualAddress();
		index_buffer_view.Format = index_format;
		index_buffer_view.SizeInBytes = index_buffer_size;

		return index_buffer_view;
	}

	void Crt_BV(ID3D12Device* device, ID3D12GraphicsCommandList* command_list) {	// Creat Buffer View
		size_t vertex_count = vertices.size();

		vertex_buffer_size = (UINT)vertices.size() * sizeof(Vertex_Info);

		if (vertex_count <= UINT16_MAX) {
			if (indices_16.empty()) {
				indices_16.resize(indices_32.size());

				for (size_t i = 0; i < indices_32.size(); ++i) {
					indices_16[i] = (std::uint16_t)indices_32[i];
				}
			}

			index_buffer_size = (UINT)indices_16.size() * sizeof(std::uint16_t);
		}
		else {
			index_buffer_size = (UINT)indices_32.size() * sizeof(std::uint32_t);
		}

		Throw_If_Failed(D3DCreateBlob(vertex_buffer_size, &vertex_buffer_cpu));
		CopyMemory(vertex_buffer_cpu->GetBufferPointer(), vertices.data(), vertex_buffer_size);

		Throw_If_Failed(D3DCreateBlob(index_buffer_size, &index_buffer_cpu));
		if (vertex_count <= UINT16_MAX) {
			CopyMemory(index_buffer_cpu->GetBufferPointer(), indices_16.data(), index_buffer_size);
		}
		else {
			CopyMemory(index_buffer_cpu->GetBufferPointer(), indices_32.data(), index_buffer_size);
		}

		vertex_buffer_gpu = Crt_DB(device, command_list, vertices.data(), vertex_buffer_size, vertex_upload_buffer);

		if (vertex_count <= UINT16_MAX) {
			index_buffer_gpu = Crt_DB(device, command_list, indices_16.data(), index_buffer_size, index_upload_buffer);
		}
		else {
			index_buffer_gpu = Crt_DB(device, command_list, indices_32.data(), index_buffer_size, index_upload_buffer);
		}

		vertex_buffer_stride = sizeof(Vertex_Info);

		if (vertex_count <= UINT16_MAX) {
			index_format = DXGI_FORMAT_R16_UINT;
		}
		else {
			index_format = DXGI_FORMAT_R32_UINT;
		}

		index_count = (UINT)indices_32.size();
	}

	void Draw(ID3D12GraphicsCommandList* command_list) {
		command_list->IASetVertexBuffers(0, 1, &Get_VBV());
		command_list->IASetIndexBuffer(&Get_IBV());
		command_list->IASetPrimitiveTopology(primitive_topology);

		command_list->DrawIndexedInstanced(index_count, 1, 0, 0, 0);
	}

	void Free_UB() {	// Free Upload Buffer
		vertex_upload_buffer = nullptr;
		index_upload_buffer = nullptr;
	}
};

struct Mesh {
	Mesh_Info* mesh_info = nullptr;
	DirectX::XMFLOAT4X4 local_transform_matrix = MathHelper::Identity_4x4();
};

struct Bone_Info {
	std::wstring name;
	DirectX::XMFLOAT4X4 offset_matrix;
	UINT parent_bone_index;
	UINT bone_index;
};

struct Skeleton_Info {
	std::wstring name;
	UINT bone_count = 0;
	std::vector<Bone_Info> bone_array;
	DirectX::XMFLOAT4X4 bone_offset_matrix_array[MAX_BONE_COUNT];

	Skeleton_Info() {}
	Skeleton_Info(std::wstring skeleton_name, std::vector<Bone_Info>& bone_array_in) {
		name = skeleton_name;

		bone_array.assign(bone_array_in.begin(), bone_array_in.end());
		bone_count = (UINT)bone_array.size();

		for (UINT i = 0; i < bone_count; ++i) {
			bone_offset_matrix_array[i] = bone_array[i].offset_matrix;
		}
	}
};

struct Transform_Info {
	DirectX::XMFLOAT3 translate = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
	DirectX::XMFLOAT4 rotate = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	DirectX::XMFLOAT3 scale = DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f);
};

inline Transform_Info Interp_Trans_Info(const Transform_Info& transform_info_a, const Transform_Info& transform_info_b, float interpolation_factor) {
	DirectX::XMVECTOR translate_a = DirectX::XMLoadFloat3(&transform_info_a.translate);
	DirectX::XMVECTOR rotate_a = DirectX::XMLoadFloat4(&transform_info_a.rotate);
	DirectX::XMVECTOR scale_a = DirectX::XMLoadFloat3(&transform_info_a.scale);

	DirectX::XMVECTOR translate_b = DirectX::XMLoadFloat3(&transform_info_b.translate);
	DirectX::XMVECTOR rotate_b = DirectX::XMLoadFloat4(&transform_info_b.rotate);
	DirectX::XMVECTOR scale_b = DirectX::XMLoadFloat3(&transform_info_b.scale);

	Transform_Info result;
	DirectX::XMStoreFloat3(&result.translate, DirectX::XMVectorLerp(translate_a, translate_b, interpolation_factor));
	DirectX::XMStoreFloat4(&result.rotate, DirectX::XMQuaternionSlerp(rotate_a, rotate_b, interpolation_factor));
	DirectX::XMStoreFloat3(&result.scale, DirectX::XMVectorLerp(scale_a, scale_b, interpolation_factor));

	return result;
}

struct Keyframe_Info {
	float time;
	std::array<Transform_Info, MAX_BONE_COUNT> animation_transform_array;
};

struct Animation_Info {
	std::wstring name;
	UINT bone_count;
	float animation_time;
	std::map<float, Keyframe_Info> keyframe_map;

	Animation_Info() {}
	Animation_Info(std::wstring animation_name, UINT bone_count_in, float animation_time_in, std::map<float, Keyframe_Info>& keyframe_map_in) {
		name = animation_name;
		bone_count = bone_count_in;
		animation_time = animation_time_in;

		keyframe_map.insert(keyframe_map_in.begin(), keyframe_map_in.end());
	}

	float Get_Upper_Keyframe_Time(float time) {
		//float keyframe_time = std::fmod(time, animation_time);
		auto keyframe = keyframe_map.lower_bound(time);

		if (keyframe == keyframe_map.end()) {
			--keyframe;
		}

		return keyframe->first;
	 }
};

struct Animation_Binding_Info {
	std::wstring binded_animation_name = L"";
	float blending_time = 0.0f;
	bool loop = false;
	Object_State next_object_state = Object_State::STATE_IDLE;
	bool movable = true;
};