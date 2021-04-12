#include "SphereActor.h"


CSphereActor::CSphereActor(const EActorType type): 
	CActor(type, EActorPrimitive::ActorPrimitiveSphere)
{
	radius = 0.5f;
}


CSphereActor::~CSphereActor(void)
{
}
