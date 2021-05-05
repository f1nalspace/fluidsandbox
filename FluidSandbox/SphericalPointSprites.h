/*
======================================================================================================================
	Fluid Sandbox - SphericalPointSprites.h

	Copyright (C) Torsten Spaete 2011-2021. All rights reserved.
	MPL v2 licensed. See LICENSE.txt for more details.
======================================================================================================================
*/

#pragma once

#define _USE_MATH_DEFINES
#include <math.h>
#include <stdio.h>

#include <final_dynamic_opengl.h>

class CSphericalPointSprites
{
private:
	unsigned int totalSpriteCount;
    GLuint vboId;
public:
	CSphericalPointSprites();
	~CSphericalPointSprites(void);
	void Allocate(const unsigned int total);
	void Draw(const unsigned int count);
	float* Map();
	void UnMap();
	static float GetPointScale(int windowHeight, float fov);
};

