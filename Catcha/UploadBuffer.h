#pragma once
#include "common.h"

template<typename T>
class UploadBuffer {
private:
	Microsoft::WRL::ComPtr<ID3D12Resource> m_upload_buffer;
	BYTE* m_data = nullptr;
	UINT m_size = 0;
	bool m_constant = false;

public:
	UploadBuffer(ID3D12Device* device, UINT count, bool constant) : m_constant(constant) {
		m_size = sizeof(T);

		if (m_constant) {
			m_size = Calc_CB_Size(sizeof(T));
		}

		Throw_If_Failed(device->CreateCommittedResource(&D3D12_HEAP_PROPERTIES_EX(D3D12_HEAP_TYPE_UPLOAD), D3D12_HEAP_FLAG_NONE,
			&D3D12_RESOURCE_DESC_EX::Buffer(m_size * count), D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_upload_buffer)));

		Throw_If_Failed(m_upload_buffer->Map(0, nullptr, reinterpret_cast<void**>(&m_data)));
	}
	~UploadBuffer() {
		if (m_upload_buffer != nullptr) {
			m_upload_buffer->Unmap(0, nullptr);
		}

		m_data = nullptr;
	}

	ID3D12Resource* Get_Resource() const { return m_upload_buffer.Get(); }

	void Copy_Data(int index, const T& data) {
		memcpy(&m_data[index * m_size], &data, sizeof(T));
	}
};