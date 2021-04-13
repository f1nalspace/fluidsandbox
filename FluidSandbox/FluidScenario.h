#pragma once

#include <string>
#include <vector>
#include <map>

#include <PxPhysicsAPI.h> 

#include "FluidContainer.hpp"
#include "Actor.hpp"
#include "Scene.h"

struct CFluidScenario
{
	physx::PxVec3 actorCreatePosition;
	physx::PxVec3 gravity;

	std::vector<CActor *> actors;
	std::vector<FluidContainer *> fluidContainers;

	char name[128];

	float viscosity;
	float stiffness;
	float damping;
	float particleDistanceFactor;
	float particleRenderFactor;
	float particleRadius;
	float particleMinDensity;

	CFluidScenario();
	~CFluidScenario(void);

	void addActor(CActor* actor) { actors.push_back(actor); }
	void addFluidContainer(FluidContainer* fluidContainer) { fluidContainers.push_back(fluidContainer); }

	size_t getActorCount() const { return actors.size(); }
	CActor* getActor(const size_t index) { return actors[index]; }

	size_t getFluidContainerCount() const { return fluidContainers.size(); }
	FluidContainer* getFluidContainer(size_t index) { return fluidContainers[index]; }

	static CFluidScenario* load(const char* filename, CScene* scene);
};

