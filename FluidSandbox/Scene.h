#pragma once

#include <vector>

#include "ScreenSpaceFluidRendering.h"
#include "XMLUtils.h"

struct CScene
{
	physx::PxVec3 backgroundColor;

	std::vector<FluidColor*> fluidColors;

	float fluidRestitution;
	float fluidDamping;
	float fluidDynamicFriction;
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

	CScene(const float fluidViscosity, const float fluidStiffness, const float fluidParticleDistanceFactor, const float fluidParticleRenderFactor, const float fluidParticleRadius, const float fluidParticleMinDensity, const float defaultActorDensity);
	~CScene(void);

	inline size_t getFluidColorCount() { return fluidColors.size(); };
	inline FluidColor* getFluidColor(int index) { return fluidColors[index]; };
	inline void addFluidColor(FluidColor* color) { fluidColors.push_back(color); };
	void resetFluidColors();

	void load(const char* filename);
};

