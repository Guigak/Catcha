#pragma once
#include "common.h"
#include "InstanceObject.h"

#define MAX_PARTICLE_COUNT 1024
#define MAX_PARTICLE_LIFE_TIME 1.0

class ParticleObject : public InstanceObject {
public:
	ParticleObject();
	virtual ~ParticleObject() {};

	virtual void Update(float total_time);

	void Add_Particle(DirectX::XMFLOAT3 position, DirectX::XMFLOAT3 scale, DirectX::XMFLOAT4 color, UINT count, float now_time);
};

