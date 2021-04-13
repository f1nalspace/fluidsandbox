#pragma once

#include <vector>
#include <iostream>

// PhysX API
#include <PxPhysicsAPI.h>

#include "FluidDescription.h"

class CFluidSystem
{
private:
	physx::PxParticleExt::IndexPool* indexPool;
	physx::PxPhysics* physics;
	physx::PxParticleFluid* particleFluid;
	unsigned int maxParticles;
	unsigned int currentParticles;
public:
	CFluidSystem(physx::PxPhysics* physics, const FluidDescription &desc, const unsigned int maxParticles);
	~CFluidSystem(void);
	int createParticles(const unsigned int numParticles, physx::PxVec3 *pos, physx::PxVec3 *vel);
	void releaseParticles(const physx::PxStrideIterator<physx::PxU32> &indices, const physx::PxU32 count);
	physx::PxParticleFluidReadData* lockReadData();
	void writeToVBO(float* data, unsigned int &count, const bool noDensity, const float minDensity);
	void setExternalAcceleration(const physx::PxVec3 &acc);
	void addForce(const physx::PxVec3 &acc, const physx::PxForceMode::Enum &mode);
	inline physx::PxActor* getActor() { return particleFluid; };
	inline void setParticleBaseFlag(const physx::PxParticleBaseFlag::Enum flag, bool value) { particleFluid->setParticleBaseFlag(flag, value); };
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

