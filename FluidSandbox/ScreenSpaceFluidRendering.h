/*
======================================================================================================================
	Fluid Sandbox - ScreenSpaceFluidRendering.h

	Copyright (C) Torsten Spaete 2011-2021. All rights reserved.
	MPL v2 licensed. See LICENSE.txt for more details.
======================================================================================================================
*/

#pragma once
#include <string>
#include <cstdint>

#include <math.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "GLSL.h"
#include "Texture.h"
#include "TextureCubemap.h"
#include "FBO.h"
#include "SphericalPointSprites.h"
#include "Renderer.h"
#include "Camera.hpp"
#include "Utils.h"

#include "AllShaders.hpp"
#include "AllFBOs.hpp"

struct FluidColor {
	glm::vec4 color;
	glm::vec4 falloff;
	char name[128];
	float falloffScale;
	bool isClear;

	FluidColor() {
		this->color = glm::vec4(0.5f, 0.69f, 1.0f, 1.0f);
		this->falloff = glm::vec4(2.0f, 1.0f, 0.5f, 0.5f);
		this->isClear = false;
		this->name[0] = 0;
		this->falloffScale = 0.1f;
	}

	FluidColor(const glm::vec4 &color, const glm::vec4 &falloff, const bool &isClear, const char *name) {
		this->color = color;
		this->falloff = falloff;
		this->isClear = isClear;
		this->name[0] = 0;
		strcpy_s(this->name, sizeof(this->name), name);
		this->falloffScale = isClear ? 0.0f : 0.1f;
	}
};

enum class SSFRenderMode: int {
	Fluid = 0,
	PointSprites,
	Points,
	Disabled,
	Count
};

enum class FluidDebugType: int {
	Final = 0,
	Depth = 1,
	Normal = 2,
	Color = 3,
	Diffuse = 4,
	Specular = 5,
	DiffuseSpecular = 6,
	Scene = 7,
	Fresnel = 8,
	Refraction = 9,
	Reflection = 10,
	FresnelReflection = 11,
	Thickness = 12,
	Absorbtion = 13,
	Max = Absorbtion
};

struct SSFDrawingOptions {
	glm::vec3 clearColor;
	FluidColor fluidColor;
	float blurScale;
	uint32_t textureState;
	FluidDebugType debugType;
	SSFRenderMode renderMode;
	bool blurEnabled;

	SSFDrawingOptions() {
		textureState = 0;
		renderMode = SSFRenderMode::Fluid;
		fluidColor = {};
		clearColor[0] = 0.0f;
		clearColor[1] = 0.0f;
		clearColor[2] = 0.0f;
		blurScale = 0.001f;
		blurEnabled = true;
		debugType = FluidDebugType::Final;
	}
};

constexpr float MAX_DEPTH = 0.9999f;
constexpr float MIN_DEPTH = -9999.0f;

class CScreenSpaceFluidRendering {
private:
	CRenderer *renderer;
	CSphericalPointSprites *pointSprites;

	CSSFRFullFBO *fullFrameBuffer;
	CSSFRDepthFBO *depthFrameBuffer;

	CPointSpritesShader *pointSpritesShader;
	CPointsShader *pointsShader;
	CDepthShader *depthShader;
	CThicknessShader *thicknessShader;
	CDepthBlurShader *depthBlurShader;
	CWaterShader *clearWaterShader;
	CWaterShader *colorWaterShader;
	CWaterShader *debugWaterShader;

	CTexture2D *sceneTexture;
	CTextureCubemap *skyboxCubemap;

	float curFBOFactor;
	float newFBOFactor;

	int curFBOWidth;
	int curFBOHeight;
	int curWindowWidth;
	int curWindowHeight;

	void DepthPass(const uint32_t numPointSprites, const glm::mat4 &proj, const glm::mat4 &view, const float zfar, const float znear, const int wH, const float particleRadius);
	void ThicknessPass(const uint32_t numPointSprites, const glm::mat4 &proj, const glm::mat4 &view, const float zfar, const float znear, const int wH, const float particleRadius);
	void RenderSSF(const CCamera &cam, const uint32_t numPointSprites, const SSFDrawingOptions &dstate, const int wW, const int wH, const float particleRadius);
	void RenderPoints(const uint32_t numPointSprites, const glm::mat4 &mvp, const glm::vec4 &color);
	void RenderPointSprites(const uint32_t numPointSprites, const glm::mat4 &proj, const glm::mat4 &view, const float zfar, const float znear, const int wH, const float particleRadius, const glm::vec4 &color);
	void RenderFullscreenQuad();
	void BlurDepthPass(const glm::mat4 &mvp, CTexture2D *depthTexture, const float dirX, const float dirY);
	void WaterPass(const CCamera &cam, const glm::mat4 &mvp, CTexture2D *depthTexture, CTexture2D *thicknessTexture, const FluidColor &color, const FluidDebugType showType);
	int CalcFBOSize(int size, float factor) { return (int)(size * factor); }
public:
	CScreenSpaceFluidRendering(const int width, const int height, CRenderer *renderer, CTextureCubemap *skyboxCubemap, CTexture2D *sceneTexture, CSphericalPointSprites *pointSprites);
	~CScreenSpaceFluidRendering(void);
	void Render(const CCamera &cam, const uint32_t numPointSprites, const SSFDrawingOptions &dstate, const int wW, const int wH, const float particleRadius);
	void SetFBOFactor(float factor) {
		if(factor > 1.0f) factor = 1.0f;
		if(factor < 0.0f) factor = 0.0f;
		newFBOFactor = factor;
	}
};

