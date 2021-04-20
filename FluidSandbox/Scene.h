/*
======================================================================================================================
	Fluid Sandbox - Scene.h

	Copyright (C) Torsten Spaete 2011-2021. All rights reserved.
	MPL v2 licensed. See LICENSE.txt for more details.
======================================================================================================================
*/

#pragma once

#include <vector>

#include <glm/glm.hpp>

#include "ScreenSpaceFluidRendering.h"
#include "FluidProperties.h"
#include "XMLUtils.h"

struct CScene
{
	glm::vec3 backgroundColor;

	std::vector<FluidColor> fluidColors;

	FluidSimulationProperties sim;
	FluidRenderProperties render;

	float defaultActorDensity;

	int fluidColorDefaultIndex;
	uint32_t numCPUThreads;

	CScene(const float defaultActorDensity);
	~CScene(void);

	inline size_t getFluidColorCount() const { return fluidColors.size(); };

	inline FluidColor &getFluidColor(int index) { return fluidColors[index]; };
	inline void addFluidColor(const FluidColor &color) { fluidColors.push_back(color); };
	void resetFluidColors();

	bool load(const char* filePath);
};

