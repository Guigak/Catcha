#pragma once
#include "common.h"

struct VertexData {
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT3 normal;
	DirectX::XMFLOAT3 tangent;
	DirectX::XMFLOAT2 uv;

	VertexData() {}
	VertexData(DirectX::XMFLOAT3& position, DirectX::XMFLOAT3& normal, DirectX::XMFLOAT3& tangent, DirectX::XMFLOAT2& uv)
		: position(position), normal(normal), tangent(tangent), uv(uv) {}

	VertexData(
		float position_x, float position_y, float position_z,
		float normal_x, float normal_y, float normal_z,
		float tangent_x, float tangent_y, float tangent_z,
		float u, float v)
		:
		position(position_x, position_y, position_z),
		normal(normal_x, normal_y, normal_z),
		tangent(tangent_x, tangent_y, tangent_z),
		uv(u, v) {}
};

struct MeshData {
	std::vector<VertexData> vertices;
	std::vector<std::uint16_t> indices_16;
	std::vector<std::uint32_t> indices_32;

	std::vector<std::uint16_t>& Get_Idxs_16() {	// Get Indices 16
		if (indices_16.empty()) {
			indices_16.resize(indices_32.size());

			for (size_t i = 0; i < indices_32.size(); ++i) {
				indices_16[i] = (std::uint16_t)indices_32[i];
			}
		}

		return indices_16;
	}
};

class MeshCreater {
public:
	MeshData Crt_Box(float width, float height, float depth, std::uint32_t subdivisions_number);

	MeshData Crt_Mesh_From_File(std::wstring file_name);
	
	void Subdivide(MeshData& mesh);
	VertexData Calc_Mid_Point(VertexData& a, VertexData& b);
};

