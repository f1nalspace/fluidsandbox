#pragma once
#include <PxPhysicsAPI.h> 
using namespace physx;
#include "Singleton.hpp"
class CFrustum: public CSingleton<CFrustum>
{
friend CSingleton<CFrustum>;
private:
	float frustum[6][4];
public:
	CFrustum(void);
	~CFrustum(void);
	void update(const float* proj, const float* modl);
	bool containsPoint(PxVec3 pos);
	bool containsSphere(PxVec3 pos, float radius);
	bool containsBounds(PxBounds3 bounds);
};

