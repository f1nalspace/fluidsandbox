#include "Actor.h"

CActor::CActor(const EActorType type, const EActorPrimitive prim)
{
	this->type = type;
	this->time = 0;
	this->timeElapsed = 0.0f;
	this->color = PxVec4(1.0f);
	this->primitive = prim;
	this->density = 1.0f;
	this->velocity = PxVec3(0.0f);
	this->visible = true;
	this->rotate = PxVec3(0.0f);
	this->particleDrain = false;
}


CActor::~CActor(void)
{
}
