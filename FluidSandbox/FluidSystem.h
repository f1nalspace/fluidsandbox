#pragma once
#include <vector>
#include <iostream>
using namespace std;

// PhysX API
#include <PxPhysicsAPI.h>
using namespace physx;

#include "FluidDescription.h"

class CFluidSystem
{
private:
	unsigned int maxParticles;
	unsigned int currentParticles;
	PxParticleExt::IndexPool* indexPool;
	PxPhysics* physics;
	PxParticleFluid* particleFluid;
public:
	CFluidSystem(PxPhysics* physics, FluidDescription &desc, const unsigned int maxParticles);
	~CFluidSystem(void);
	int createParticles(const unsigned int numParticles, PxVec3 *pos, PxVec3 *vel);
	void releaseParticles(const PxStrideIterator<PxU32> &indices, const PxU32 count);
	PxParticleFluidReadData* lockReadData();
	void writeToVBO(float* data, unsigned int &count, const bool noDensity, const float minDensity);
	void setExternalAcceleration(const PxVec3 &acc);
	void addForce(const PxVec3 &acc, const PxForceMode::Enum &mode);
	inline PxActor* getActor() { return particleFluid; };
	inline void setParticleBaseFlag(const PxParticleBaseFlag::Enum flag, bool value) { particleFluid->setParticleBaseFlag(flag, value); };
	inline void setViscosity(const float value) { particleFluid->setViscosity(value); };
	inline void setStiffness(const float value) { particleFluid->setStiffness(value); };
	inline void setMaxMotionDistance(const float value) { particleFluid->setMaxMotionDistance(value); };
	inline void setContactOffset(const float value) { particleFluid->setContactOffset(value); };
	inline void setRestOffset(const float value) { particleFluid->setRestOffset(value); };
	inline void setRestParticleDistance(const float value) { particleFluid->setRestParticleDistance(value); };
	inline void setRestitution(const float value) { particleFluid->setRestitution(value); };
	inline void setDamping(const float value) { particleFluid->setDamping(value); };
	inline void setDynamicFriction(const float value) { particleFluid->setDynamicFriction(value); };
	inline void setParticleMass(const float value) { particleFluid->setParticleMass(value); };
	inline unsigned int getCurrentParticles() { return currentParticles; };
};

