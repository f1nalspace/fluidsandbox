#pragma once

#include <PxPhysicsAPI.h> 

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
	bool containsPoint(physx::PxVec3 pos);
	bool containsSphere(physx::PxVec3 pos, float radius);
	bool containsBounds(physx::PxBounds3 bounds);
};

