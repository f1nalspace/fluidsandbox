#pragma once

#include <string>
#include <vector>
#include <map>

#include <glm/glm.hpp> 

#include "Actor.hpp"
#include "Scene.h"

#include "AllActors.hpp"

struct FluidScenario
{
	glm::vec3 actorCreatePosition;
	glm::vec3 gravity;

	std::vector<const Actor *> bodies;
	std::vector<const FluidActor *> fluids;

	char name[128];

	FluidSimulationProperties sim;
	FluidRenderProperties render;

	FluidScenario();
	~FluidScenario(void);

	static FluidScenario* load(const char* filename, CScene* scene);
};

