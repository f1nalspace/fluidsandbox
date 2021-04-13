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
	physx::PxVec3 Pos;
	physx::PxVec3 Size;
	physx::PxVec3 Vel;
	float Radius;
	int Time;
	float TimeElapsed;
	FluidType Type;
	bool IsEmitter;
	unsigned int EmitterCoolDown;
	float EmitterRate;
	float EmitterTime;
	unsigned int EmitterDuration;
	float EmitterElapsed;
	bool EmitterCoolDownActive;
	float EmitterCoolDownElapsed;

	FluidContainer(physx::PxVec3 pos, physx::PxVec3 size, FluidType type){
		Pos = pos;
		Size = size;
		Vel = physx::PxVec3(0.0f);
		Time = 0;
		TimeElapsed = 0.0f;
		Type = type;
		Radius = 0.0f;
		IsEmitter = false;
		EmitterCoolDown = 0;
		EmitterRate = 0.0f;
		EmitterTime = 0.0f;
		EmitterDuration = 0;
		EmitterElapsed = 0.0f;
		EmitterCoolDownActive = false;
		EmitterCoolDownElapsed = 0.0f;
	}
};
