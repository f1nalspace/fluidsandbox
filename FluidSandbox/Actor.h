#pragma once

#include <PxPhysicsAPI.h> 

enum class EActorType
{
	ActorTypeStatic = 0,
	ActorTypeDynamic
};

enum class EActorPrimitive
{
	ActorPrimitiveCube = 0,
	ActorPrimitiveSphere
};

struct CActor
{
	physx::PxVec4 color;
	physx::PxVec3 pos;
	physx::PxVec3 velocity;
	physx::PxVec3 rotate;
	float timeElapsed;
	float density;
	EActorType type;
	EActorPrimitive primitive;
	int time;
	bool visible;
	bool blending;
	bool particleDrain;
	CActor(const EActorType type, const EActorPrimitive prim);
	~CActor(void);
};
