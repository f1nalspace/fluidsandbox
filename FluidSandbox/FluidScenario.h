#pragma once

#include <string>
#include <vector>
#include <map>

#include <glm/glm.hpp> 

#include "FluidContainer.hpp"
#include "Actor.hpp"
#include "Scene.h"

struct FluidScenario
{
	glm::vec3 actorCreatePosition;
	glm::vec3 gravity;

	std::vector<Actor *> actors;
	std::vector<FluidContainer *> fluidContainers;

	char name[128];

	FluidSimulationProperties sim;
	FluidRenderProperties render;

	FluidScenario();
	~FluidScenario(void);

	void addActor(Actor* actor) { actors.push_back(actor); }
	void addFluidContainer(FluidContainer* fluidContainer) { fluidContainers.push_back(fluidContainer); }

	size_t getActorCount() const { return actors.size(); }
	Actor* getActor(const size_t index) { return actors[index]; }

	size_t getFluidContainerCount() const { return fluidContainers.size(); }
	FluidContainer* getFluidContainer(size_t index) { return fluidContainers[index]; }

	static FluidScenario* load(const char* filename, CScene* scene);
};

