/*
======================================================================================================================
	Fluid Sandbox - Scenario.h

	Copyright (C) Torsten Spaete 2011-2021. All rights reserved.
	MPL v2 licensed. See LICENSE.txt for more details.
======================================================================================================================
*/

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
	char fileName[256];
	char displayName[128];

	glm::vec3 actorCreatePosition;
	glm::vec3 gravity;

	std::vector<const Actor *> bodies;
	std::vector<const FluidActor *> fluids;

	FluidSimulationProperties sim;
	FluidRenderProperties render;

	Scenario();
	~Scenario(void);

	static Scenario* load(const char* filePath, CScene* scene);
};

