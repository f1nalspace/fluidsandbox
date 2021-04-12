#pragma once

#define _USE_MATH_DEFINES
#include <math.h>
#include <stdio.h>

#include <GL\glew.h>

class CSphericalPointSprites
{
private:
	unsigned int iTotalSprites;
    GLuint iVBO;
public:
	CSphericalPointSprites();
	~CSphericalPointSprites(void);
	void Allocate(unsigned int total);
	void Draw(unsigned int count);
	float* Map();
	void UnMap();
	static float GetPointScale(int windowHeight, float fov) {
		return (float)(windowHeight / tan(fov*0.5*M_PI/180.0));
	}
};

