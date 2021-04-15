#pragma once

#include <string>
#include <vector>
#include <map>

#include <glm/glm.hpp> 

#include "Actor.hpp"
#include "Scene.h"

#include "AllActors.hpp"

struct Scenario
{
	glm::vec3 actorCreatePosition;
	glm::vec3 gravity;

	std::vector<const Actor *> bodies;
	std::vector<const FluidActor *> fluids;

	char name[128];

	FluidSimulationProperties sim;
	FluidRenderProperties render;

	Scenario();
	~Scenario(void);

	static Scenario* load(const char* filename, CScene* scene);
};

