#pragma once

#define _USE_MATH_DEFINES
#include <math.h>
#include <stdio.h>

#include <GL\glew.h>

class CSphericalPointSprites
{
private:
	unsigned int totalSpriteCount;
    GLuint vbo;
public:
	CSphericalPointSprites();
	~CSphericalPointSprites(void);
	void Allocate(const unsigned int total);
	void Draw(const unsigned int count);
	float* Map();
	void UnMap();
	static float GetPointScale(int windowHeight, float fov);
};

