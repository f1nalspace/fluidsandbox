#pragma once
#include "actor.h"
class CCubeActor :
	public CActor
{
private:
	PxVec3 size;
public:
	CCubeActor(const EActorType type);
	~CCubeActor(void);
	void setSize(const PxVec3 size) { this->size = size; }
	PxVec3 getSize() { return size; }
};

