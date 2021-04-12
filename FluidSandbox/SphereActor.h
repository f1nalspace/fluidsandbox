#pragma once
#include "Actor.h"
struct CSphereActor: public CActor {
	float radius;
	CSphereActor(const EActorType type);
	~CSphereActor(void);
};

