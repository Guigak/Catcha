#pragma once
#include "common.h"
#include "Object.h"

class UIObject : public Object {
private:
	DirectX::XMFLOAT4 m_top_left_bottom_right = DirectX::XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f);

public:
	UIObject(float position_x, float position_y, float scale_x, float scale_y,
		UINT texture_width, UINT texture_height, float top, float left, float bottom, float right);
	virtual ~UIObject() {}

	virtual void Update(float elapsed_time);

	void Set_Texture_WH(UINT texture_width, UINT texture_height);	// Set Texture Width Height
	void Set_TLBR(float top, float left, float bottom, float right);	// Set Top Left Bottom Right

	//
	virtual bool Picking(DirectX::XMFLOAT4 origin_ray, DirectX::XMFLOAT4 ray_direction,
		DirectX::XMFLOAT4X4 inverse_view_matrix, float picking_distance);
};

