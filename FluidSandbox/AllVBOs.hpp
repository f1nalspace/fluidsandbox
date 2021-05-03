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

