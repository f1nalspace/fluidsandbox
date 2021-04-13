#pragma once

#include <PxPhysicsAPI.h> 

enum class ActorType
{
	ActorTypeStatic = 0,
	ActorTypeDynamic
};

enum class ActorPrimitiveKind
{
	Cube = 0,
	Sphere
};

struct CActor
{
	physx::PxVec4 color;
	physx::PxVec3 pos;
	physx::PxVec3 velocity;
	physx::PxVec3 rotate;
	float timeElapsed;
	float density;
	ActorType type;
	ActorPrimitiveKind primitive;
	int time;
	bool visible;
	bool blending;
	bool particleDrain;

	CActor(const ActorType type, const ActorPrimitiveKind prim) {
		this->type = type;
		this->time = 0;
		this->timeElapsed = 0.0f;
		this->color = physx::PxVec4(1.0f);
		this->primitive = prim;
		this->density = 1.0f;
		this->velocity = physx::PxVec3(0.0f);
		this->visible = true;
		this->rotate = physx::PxVec3(0.0f);
		this->particleDrain = false;
	}
};
