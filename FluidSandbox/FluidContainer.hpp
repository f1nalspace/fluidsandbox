#pragma once

#include <PxPhysicsAPI.h> 

enum class FluidType {
	Sphere = 3,
	Blob = 2,
	Wall = 1,
	Drop = 0
};

struct FluidContainer
{
	physx::PxVec3 pos;
	physx::PxVec3 size;
	physx::PxVec3 vel;
	float radius;
	float timeElapsed;
	float emitterRate;
	float emitterTime;
	float emitterElapsed;
	float emitterCoolDownElapsed;
	uint32_t emitterCoolDown;
	uint32_t emitterDuration;
	int time;
	FluidType type;
	bool isEmitter;
	bool emitterCoolDownActive;

	FluidContainer(const physx::PxVec3 &pos, const physx::PxVec3 &size, const FluidType type){
		this->pos = pos;
		this->size = size;
		this->type = type;
		vel = physx::PxVec3(0.0f);
		time = 0;
		timeElapsed = 0.0f;
		radius = 0.0f;
		isEmitter = false;
		emitterCoolDown = 0;
		emitterRate = 0.0f;
		emitterTime = 0.0f;
		emitterDuration = 0;
		emitterElapsed = 0.0f;
		emitterCoolDownActive = false;
		emitterCoolDownElapsed = 0.0f;
	}
};
