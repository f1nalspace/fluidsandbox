#pragma once

#include "VBO.h"

struct GeometryVBO: public CVBO {
	GLuint triangleIndexCount;
	GLuint lineIndexCount;

	GeometryVBO():
		CVBO(),
		triangleIndexCount(0),
		lineIndexCount(0) {

	}
};

