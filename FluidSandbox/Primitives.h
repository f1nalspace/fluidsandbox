#pragma once

#include "VBO.h"

#define QUAD_TO_TRIANGLE(a, b, c, d) a, b, c, c, d, a

namespace Primitives
{
	CVBO* createCube(const float sizeX, const float sizeY, const float sizeZ, const bool normalPositive);
};