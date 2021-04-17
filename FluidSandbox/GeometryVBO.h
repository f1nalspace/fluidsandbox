/*
======================================================================================================================
	Fluid Sandbox - GeometryVBO.h

	Copyright (C) Torsten Spaete 2011-2021. All rights reserved.
	MPL v2 licensed. See LICENSE.txt for more details.
======================================================================================================================
*/

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

