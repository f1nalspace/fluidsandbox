#pragma once

#include <vector>

#include "ScreenSpaceFluidRendering.h"
#include "XMLUtils.h"

class CScene
{
private:
	physx::PxVec3 backgroundColor;
	std::vector<FluidColor*> fluidColors;
	float fluidRestitution;
	float fluidDamping;
	float fluidDynamicFrictionn;
	float fluidMaxMotionDistance;
	float fluidRestOffset;
	float fluidContactOffset;
	float fluidParticleMass;
	float fluidViscosity;
	float fluidStiffness;
	float fluidParticleRadius;
	float fluidParticleDistanceFactor;
	float fluidParticleRenderFactor;
	float fluidParticleMinDensity;
	float defaultFluidViscosity;
	float defaultFluidStiffness;
	float defaultFluidParticleRadius;
	float defaultFluidParticleDistanceFactor;
	float defaultFluidParticleRenderFactor;
	float defaultFluidParticleMinDensity;
	float defaultFluidColorFalloffScale;
	float defaultActorDensity;
	int fluidColorDefaultIndex;
	int numCPUThreads;
public:
	CScene(const float fluidViscosity, const float fluidStiffness, const float fluidParticleDistanceFactor, const float fluidParticleRenderFactor, const float fluidParticleRadius, const float fluidParticleMinDensity, const float defaultActorDensity);
	~CScene(void);

	inline int getFluidColors() { return fluidColors.size(); };
	inline FluidColor* getFluidColor(int index) { return fluidColors[index]; };
	inline void addFluidColor(FluidColor* color) { fluidColors.push_back(color); };
	void resetFluidColors();

	inline float getFluidRestitution() { return fluidRestitution; };
	inline float getFluidDamping() { return fluidDamping; };
	inline float getFluidDynamicFriction() { return fluidDynamicFrictionn; };
	inline float getFluidMaxMotionDistance() { return fluidMaxMotionDistance; };
	inline float getFluidRestOffset() { return fluidRestOffset; };
	inline float getFluidContactOffset() { return fluidContactOffset; };
	inline float getFluidParticleMass() { return fluidParticleMass; };
	inline float getFluidViscosity() { return fluidViscosity; };
	inline float getFluidStiffness() { return fluidStiffness; };
	inline float getFluidParticleDistanceFactor() { return fluidParticleDistanceFactor; };
	inline float getFluidParticleRenderFactor() { return fluidParticleRenderFactor; };
	inline float getFluidParticleRadius() { return fluidParticleRadius; };
	inline float getFluidParticleMinDensity() { return fluidParticleMinDensity; };
	inline float getDefaultActorDensity() { return defaultActorDensity; };
	inline int getFluidColorDefaultIndex() { return fluidColorDefaultIndex; };
	inline int getNumCPUThreads() { return numCPUThreads; };
	inline void setFluidRestitution(const float value) { fluidRestitution = value; };
	inline void setFluidDamping(const float value) { fluidDamping = value; };
	inline void setFluidDynamicFriction(const float value) { fluidDynamicFrictionn = value; };
	inline void setFluidMaxMotionDistance(const float value) { fluidMaxMotionDistance = value; };
	inline void setFluidRestOffset(const float value) { fluidRestOffset = value; };
	inline void setFluidContactOffset(const float value) { fluidContactOffset = value; };
	inline void setFluidParticleMass(const float value) { fluidParticleMass = value; };
	inline void setFluidViscosity(const float value) { fluidViscosity = value; };
	inline void setFluidStiffness(const float value) { fluidStiffness = value; };
	inline void setFluidParticleRadius(const float value) { fluidParticleRadius = value; };
	inline void setFluidParticleDistanceFactor(const float value) { fluidParticleDistanceFactor = value; };
	inline void setFluidParticleRenderFactor(const float value) { fluidParticleRenderFactor = value; };
	inline void setFluidParticleMinDenstiy(const float value) { fluidParticleMinDensity = value; };
	inline void setNumCPUThreads(const int value) { numCPUThreads = value; };

	inline physx::PxVec3 getBackgroundColor() { return backgroundColor; };
	inline void setBackgroundColor(const physx::PxVec3 value) { backgroundColor = value; };

	void load(const char* filename);
};

