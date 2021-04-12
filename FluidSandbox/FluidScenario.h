#pragma once

#include <string>
#include <vector>
#include <map>

#include <PxPhysicsAPI.h> 

#include <glm/glm.hpp>

#include "FluidContainer.hpp"
#include "OSLowLevel.h"
#include "XMLUtils.h"
#include "Utils.h"
#include "Actor.h"
#include "CubeActor.h"
#include "SphereActor.h"
#include "Scene.h"

struct CFluidScenario
{
	physx::PxVec3 actorCreatePosition;
	physx::PxVec3 gravity;

	std::vector<CActor *> actors;
	std::vector<FluidContainer *> fluidContainers;

	std::string name;

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
	size_t getActorCount() { return actors.size(); }
	CActor* getActor(const size_t index) { return actors[index]; }
	size_t getFluidContainerCount() { return fluidContainers.size(); }
	FluidContainer* getFluidContainer(size_t index) { return fluidContainers[index]; }
	static CFluidScenario* load(const char* filename, CScene* scene);
};

