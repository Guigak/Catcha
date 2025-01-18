#pragma once
#include "common.h"
#include "InstanceObject.h"

#define TEXT_TEXTURE_WIDTH 32
#define TEXT_UI_TEXTURE_WIDTH 8192

#define TEXT_MAX_COUNT 128

constexpr float TEXT_WIDTH_MULTIPLIER = 3.0f / 5.0f;

class TextUIObject : public InstanceObject {
private:
	std::wstring m_text = L"";
	DirectX::XMFLOAT2 m_position = DirectX::XMFLOAT2(0.0f, 0.0f);	// x, y
	DirectX::XMFLOAT2 m_scale = DirectX::XMFLOAT2(1.0f, 1.0f);	// x, y

public:
	TextUIObject(float position_x, float position_y, float scale_x, float scale_y);
	virtual ~TextUIObject() {}

	virtual void Update(float elapsed_time);

	void Set_Text(std::wstring text);
};

