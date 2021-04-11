#pragma once
#include <PxPhysicsAPI.h> 
using namespace physx;

enum FluidType {
	FluidTypeSphere = 3,
	FluidTypeBlob = 2,
	FluidTypeWall = 1,
	FluidTypeDrop = 0
};

struct FluidContainer
{
	PxVec3 Pos;
	PxVec3 Size;
	PxVec3 Vel;
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

	FluidContainer(PxVec3 pos, PxVec3 size, FluidType type){
		Pos = pos;
		Size = size;
		Vel = PxVec3(0.0f);
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
