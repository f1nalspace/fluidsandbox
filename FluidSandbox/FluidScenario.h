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

	static FluidScenario* load(const char* filename, CScene* scene);
};

