/*
======================================================================================================================
	Fluid Sandbox - AllVBOs.hpp

	Copyright (C) Torsten Spaete 2011-2021. All rights reserved.
	MPL v2 licensed. See LICENSE.txt for more details.
======================================================================================================================
*/

#pragma once

#include "VBO.h"

struct DynamicVBO: public CVBO {
	uint32_t vertexCount;
	uint32_t indexCount;

	DynamicVBO():
		CVBO(),
		vertexCount(0),
		indexCount(0) {

	}
};

