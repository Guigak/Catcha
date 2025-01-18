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

public:
	TextUIObject(float position_x, float position_y, float scale_x, float scale_y);
	virtual ~TextUIObject() {}

	virtual void Update(float elapsed_time);

	void Set_Text(std::wstring text);
};

