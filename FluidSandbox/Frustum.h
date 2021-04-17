/*
======================================================================================================================
	Fluid Sandbox - Frustum.h

	Copyright (C) Torsten Spaete 2011-2021. All rights reserved.
	MPL v2 licensed. See LICENSE.txt for more details.
======================================================================================================================
*/

#pragma once

#include <glm/glm.hpp> 

struct Frustum
{
	float frustum[6][4];
	void update(const float* proj, const float* modl);
	bool containsPoint(const glm::vec3 &pos);
	bool containsSphere(const glm::vec3 &pos, const float radius);
	bool containsBounds(const glm::vec3 &minimum, const glm::vec3 &maximum);
};

