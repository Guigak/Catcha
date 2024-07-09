#pragma once

// about memory leak
#if defined(DEBUG) || defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

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
#include <string>
#include <memory>
#include <algorithm>
#include <vector>
#include <array>
#include <stack>
#include <unordered_map>
#include <cstdint>
#include <fstream>
#include <sstream>
#include <cassert>

// constant
constexpr int CLIENT_WIDTH = 800;
constexpr int CLIENT_HEIGHT = 600;

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

#define Throw_If_Failed(x)															\
{																					\
	HRESULT hresult = (x);															\
	std::wstring file_name = Str_2_WStr(__FILE__);									\
	if (FAILED(hresult)) { throw DXException(hresult, L#x, file_name, __LINE__); }	\
}

#define Release_Com(x) { if (x) { x->Release(); x = 0; } }

// from d3dx12.h
struct D3D12_DEFAULT {};

struct D3D12_RESOURCE_BARRIER_HELPER : public D3D12_RESOURCE_BARRIER {
	D3D12_RESOURCE_BARRIER_HELPER() {}
	explicit D3D12_RESOURCE_BARRIER_HELPER(const D3D12_RESOURCE_BARRIER& object) : D3D12_RESOURCE_BARRIER(object) {}

	static inline D3D12_RESOURCE_BARRIER_HELPER Transition(
		_In_ ID3D12Resource* resource,
		D3D12_RESOURCE_STATES before_state,
		D3D12_RESOURCE_STATES after_state,
		UINT subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES,
		D3D12_RESOURCE_BARRIER_FLAGS flags = D3D12_RESOURCE_BARRIER_FLAG_NONE
	) {
		D3D12_RESOURCE_BARRIER_HELPER result;
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

	static inline D3D12_RESOURCE_BARRIER_HELPER Aliasing(
		_In_ ID3D12Resource* before_resource,
		_In_ ID3D12Resource* after_resource
	) {
		D3D12_RESOURCE_BARRIER_HELPER result;
		ZeroMemory(&result, sizeof(result));
		D3D12_RESOURCE_BARRIER& barrier = result;
		result.Type = D3D12_RESOURCE_BARRIER_TYPE_ALIASING;
		barrier.Aliasing.pResourceBefore = before_resource;
		barrier.Aliasing.pResourceAfter = after_resource;

		return result;
	}

	static inline D3D12_RESOURCE_BARRIER_HELPER UAV(_In_ ID3D12Resource* resource) {
		D3D12_RESOURCE_BARRIER_HELPER result;
		ZeroMemory(&result, sizeof(result));
		D3D12_RESOURCE_BARRIER& barrier = result;
		result.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
		barrier.UAV.pResource = resource;

		return result;
	}

	operator const D3D12_RESOURCE_BARRIER_HELPER& () const { return *this; }
};

struct D3D12_HEAP_PROPERTIES_EX : public D3D12_HEAP_PROPERTIES {
	D3D12_HEAP_PROPERTIES_EX() {}
	explicit D3D12_HEAP_PROPERTIES_EX(const D3D12_HEAP_PROPERTIES& o) : D3D12_HEAP_PROPERTIES(o) {}

	D3D12_HEAP_PROPERTIES_EX(
		D3D12_CPU_PAGE_PROPERTY cpu_page_property,
		D3D12_MEMORY_POOL memory_pool_preference,
		UINT creation_nodemask = 1,
		UINT nodeMask = 1
	) {
		Type = D3D12_HEAP_TYPE_CUSTOM;
		CPUPageProperty = cpu_page_property;
		MemoryPoolPreference = memory_pool_preference;
		CreationNodeMask = creation_nodemask;
		VisibleNodeMask = nodeMask;
	}

	explicit D3D12_HEAP_PROPERTIES_EX(
		D3D12_HEAP_TYPE type,
		UINT creation_nodemask = 1,
		UINT nodeMask = 1
	) {
		Type = type;
		CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		CreationNodeMask = creation_nodemask;
		VisibleNodeMask = nodeMask;
	}

	operator const D3D12_HEAP_PROPERTIES& () const { return *this; }

	bool IsCPUAccessible() const {
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

struct D3D12_CPU_DESCRIPTOR_HANDLE_EX : public D3D12_CPU_DESCRIPTOR_HANDLE {
	D3D12_CPU_DESCRIPTOR_HANDLE_EX() {}
	explicit D3D12_CPU_DESCRIPTOR_HANDLE_EX(const D3D12_CPU_DESCRIPTOR_HANDLE& object) : D3D12_CPU_DESCRIPTOR_HANDLE(object) {}

	D3D12_CPU_DESCRIPTOR_HANDLE_EX(D3D12_DEFAULT) { ptr = 0; }

	D3D12_CPU_DESCRIPTOR_HANDLE_EX(_In_ const D3D12_CPU_DESCRIPTOR_HANDLE& other, INT offset_scale)
	{
		Init_Offset(other, offset_scale);
	}

	D3D12_CPU_DESCRIPTOR_HANDLE_EX(_In_ const D3D12_CPU_DESCRIPTOR_HANDLE& other, INT offset, UINT increment_size)
	{
		Init_Offset(other, offset, increment_size);
	}

	D3D12_CPU_DESCRIPTOR_HANDLE_EX& Offset(INT offset, UINT increment_size)
	{
		ptr += offset * increment_size;

		return *this;
	}

	D3D12_CPU_DESCRIPTOR_HANDLE_EX& Offset(INT offset_scale)
	{
		ptr += offset_scale;

		return *this;
	}

	bool operator==(_In_ const D3D12_CPU_DESCRIPTOR_HANDLE& other)
	{
		return (ptr == other.ptr);
	}

	bool operator!=(_In_ const D3D12_CPU_DESCRIPTOR_HANDLE& other)
	{
		return (ptr != other.ptr);
	}

	D3D12_CPU_DESCRIPTOR_HANDLE_EX& operator=(const D3D12_CPU_DESCRIPTOR_HANDLE& other)
	{
		ptr = other.ptr;

		return *this;
	}


	//
	inline void Init_Offset(_In_ const D3D12_CPU_DESCRIPTOR_HANDLE& base, INT offset_scale)
	{
		Init_Offset(*this, base, offset_scale);
	}

	inline void Init_Offset(_In_ const D3D12_CPU_DESCRIPTOR_HANDLE& base, INT offset, UINT increment_size)
	{
		Init_Offset(*this, base, offset, increment_size);
	}

	static inline void Init_Offset(_Out_ D3D12_CPU_DESCRIPTOR_HANDLE& handle, _In_ const D3D12_CPU_DESCRIPTOR_HANDLE& base, INT offset_scale)
	{
		handle.ptr = base.ptr + offset_scale;
	}

	static inline void Init_Offset(_Out_ D3D12_CPU_DESCRIPTOR_HANDLE& handle, _In_ const D3D12_CPU_DESCRIPTOR_HANDLE& base, INT offset, UINT increment_size)
	{
		handle.ptr = base.ptr + offset * increment_size;
	}
};