#pragma once

#include "GLSL.h"

class CLightingShader: public CGLSL {
protected:
	void updateUniformLocations();
public:
	GLuint ulocColor;

	CLightingShader():
		CGLSL(),
		ulocColor(0) {
	}

};

class CSkyboxShader: public CGLSL {
protected:
	void updateUniformLocations();
public:
	GLuint ulocMVP;
	GLuint ulocCubemap;

	CSkyboxShader():
		CGLSL(),
		ulocMVP(0),
		ulocCubemap(0) {

	}

};

class CPointSpritesShader: public CGLSL {
protected:
	void updateUniformLocations();
public:
	GLuint ulocPointScale;
	GLuint ulocPointRadius;
	GLuint ulocNear;
	GLuint ulocFar;
	GLuint ulocViewMat;
	GLuint ulocProjMat;

	CPointSpritesShader():
		CGLSL(),
		ulocPointScale(0),
		ulocPointRadius(0),
		ulocNear(0),
		ulocFar(0),
		ulocViewMat(0),
		ulocProjMat(0) {
	}

};

class CDepthShader: public CPointSpritesShader {
public:
	static constexpr char *ShaderName = "Depth";
	CDepthShader(): CPointSpritesShader() {}
};

class CThicknessShader: public CPointSpritesShader {
public:
	static constexpr char *ShaderName = "Thickness";
	CThicknessShader(): CPointSpritesShader() {}
};

class CWa: public CGLSL {
protected:
	void updateUniformLocations();
public:
	static constexpr char *ShaderName = "DepthBlur";
	GLuint ulocDepthTex;
	GLuint ulocScale;
	GLuint ulocRadius;
	GLuint ulocMinDepth;
	GLuint ulocMVPMat;

	CWa():
		CGLSL(),
		ulocDepthTex(0),
		ulocScale(0),
		ulocRadius(0),
		ulocMinDepth(0),
		ulocMVPMat(0) {

	}

};

class CWaterShader: public CGLSL {
protected:
	void updateUniformLocations();
public:
	static constexpr char *ClearName = "ClearWater";
	static constexpr char *ColorName = "ColorWater";
	static constexpr char *DebugName = "Debug";
	GLuint ulocDepthTex;
	GLuint ulocThicknessTex;
	GLuint ulocSceneTex;
	GLuint ulocSkyboxCubemap;
	GLuint ulocXFactor;
	GLuint ulocYFactor;
	GLuint ulocZNear;
	GLuint ulocZFar;
	GLuint ulocMinDepth;
	GLuint ulocColorFalloff;
	GLuint ulocFluidColor;
	GLuint ulocShowType;
	GLuint ulocMVPMat;
	GLuint ulocFalloffScale;

	CWaterShader():
		CGLSL(),
		ulocDepthTex(0),
		ulocThicknessTex(0),
		ulocSceneTex(0),
		ulocSkyboxCubemap(0),
		ulocXFactor(0),
		ulocYFactor(0),
		ulocZNear(0),
		ulocZFar(0),
		ulocMinDepth(0),
		ulocColorFalloff(0),
		ulocFluidColor(0),
		ulocShowType(0),
		ulocMVPMat(0),
		ulocFalloffScale(0) {

	}

};