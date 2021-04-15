#pragma once

#include "FBO.h"

class CSceneFBO: public CFBO {
public:
	CTexture2D *depthTexture;
	CTexture2D *sceneTexture;
	CSceneFBO(int width, int height):
		CFBO(width, height),
		depthTexture(nullptr),
		sceneTexture(nullptr) {

	}
};

class CSSFRDepthFBO: public CFBO {
public:
	CTexture2D *depthTexture;
	CTexture2D *colorTexture;
	CSSFRDepthFBO(int width, int height):
		CFBO(width, height),
		depthTexture(nullptr),
		colorTexture(nullptr) {

	}
};

class CSSFRFullFBO: public CFBO {
public:
	CTexture2D *thicknessTexture;
	CTexture2D *depthSmoothATexture;
	CTexture2D *depthSmoothBTexture;
	CTexture2D *waterTexture;
	CSSFRFullFBO(int width, int height):
		CFBO(width, height),
		thicknessTexture(nullptr),
		depthSmoothATexture(nullptr),
		depthSmoothBTexture(nullptr),
		waterTexture(nullptr) {
	}
};

