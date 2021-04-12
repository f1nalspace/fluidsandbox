#pragma once

#include "actor.h"

class CCubeActor :
	public CActor
{
private:
	physx::PxVec3 size;
public:
	CCubeActor(const EActorType type);
	~CCubeActor(void);
	void setSize(const physx::PxVec3 size) { this->size = size; }
	physx::PxVec3 getSize() { return size; }
};

