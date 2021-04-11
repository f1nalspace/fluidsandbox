#pragma once
#include "VBO.h"

#define QUAD_TO_TRIANGLE(a, b, c, d) a, b, c, c, d, a

namespace Primitives
{
	CVBO* createCube(float sizeX, float sizeY, float sizeZ, bool normalPositive);
};