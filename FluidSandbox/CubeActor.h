#pragma once

#include "actor.h"

struct CCubeActor :	public CActor
{
	physx::PxVec3 size;
	CCubeActor(const EActorType type);
	~CCubeActor(void);
};

