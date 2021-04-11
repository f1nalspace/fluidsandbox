#pragma once
#include "Actor.h"
class CSphereActor :
	public CActor
{
private:
	float radius;
public:
	CSphereActor(const EActorType type);
	~CSphereActor(void);
	float getRadius() { return radius; }
	void setRadius(const float radius) { this->radius = radius; }
};

