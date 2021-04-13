#pragma once
#include <string>

#include <math.h>

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "GLSL.h"
#include "Texture.h"
#include "FBO.h"
#include "SphericalPointSprites.h"
#include "Renderer.h"
#include "Camera.hpp"
#include "Utils.h"

#include "AllShaders.h"
#include "AllFBOs.h"

const int SSFShaderCount = 6;

const int SSFShaderIndex_DepthPass = 0;
const int SSFShaderIndex_ThicknessPass = 1;
const int SSFShaderIndex_DepthBlurFast = 2;
const int SSFShaderIndex_ClearWater = 3;
const int SSFShaderIndex_ColorWater = 4;
const int SSFShaderIndex_DebugWater = 5;

struct FluidColor
{
	physx::PxVec4 color;
	physx::PxVec4 falloff;
	std::string name;
	float falloffScale;
	bool isClear;

	FluidColor(){
		this->color = physx::PxVec4(0.5f, 0.69f, 1.0f, 1.0f);
		this->falloff = physx::PxVec4(2.0f, 1.0f, 0.5f, 0.5f);
		this->isClear = false;
		this->name = "";
		this->falloffScale = 0.1f;
	}

	FluidColor(const physx::PxVec4& color, const physx::PxVec4& falloff, const bool &isClear, const char *name){
		this->color = color;
		this->falloff = falloff;
		this->isClear = isClear;
		this->name = name;
		this->falloffScale = isClear ? 0.0f : 0.1f;
	}
};

struct SSFDrawingOptions
{
	glm::vec3 clearColor;
	FluidColor *fluidColor;
	float blurScale;
	unsigned int renderMode;
	unsigned int textureState;
	int debugType;
	bool blurEnabled;

	SSFDrawingOptions()
	{
		textureState = 0;
		renderMode = 0; // SSF
		fluidColor = NULL;
		clearColor[0] = 0.0f;
		clearColor[1] = 0.0f;
		clearColor[2] = 0.0f;
		blurScale = 0.001f;
		blurEnabled = true;
		debugType = 0;
	}
};

const unsigned int SSFRenderMode_Fluid = 0;
const unsigned int SSFRenderMode_PointSprites = 1;
const unsigned int SSFRenderMode_Points = 2;
const unsigned int SSFRenderMode_Disabled = 3;

const float MAX_DEPTH = 0.9999f;
const float MIN_DEPTH = -9999.0f;

#define SSFFBOFACTOR(x, y) (x / roundf(y))

class CScreenSpaceFluidRendering
{
private:
	CGLSL *aShaders[SSFShaderCount];

	CRenderer *renderer;
	CSphericalPointSprites *pointSprites;
	CPointSpritesShader *pointSpritesShader;

	CSSFRFullFBO *fullFrameBuffer;
	CSSFRDepthFBO *depthFrameBuffer;

	CDepthShader *depthShader;
	CThicknessShader *thicknessShader;
	CDepthBlurShader *depthBlurShader;
	CWaterShader *clearWaterShader;
	CWaterShader *colorWaterShader;
	CWaterShader *debugWaterShader;

	CTexture *sceneTexture;
	CTexture *skyboxCubemap;

	float curFBOFactor;
	float newFBOFactor;
	float particleRadius;

	int curFBOWidth;
	int curFBOHeight;
	int curWindowWidth;
	int curWindowHeight;

	void DepthPass(const unsigned int numPointSprites, const glm::mat4 &proj, const glm::mat4 &view, const float zfar, const float znear, const int wH);
	void ThicknessPass(const unsigned int numPointSprites, const glm::mat4 &proj, const glm::mat4 &view, const float zfar, const float znear, const int wH);
	void RenderSSF(CCamera &cam, const unsigned int numPointSprites, const SSFDrawingOptions &dstate, const int wW, const int wH);
	void RenderPointSprites(const unsigned int numPointSprites, const glm::mat4 &proj, const glm::mat4 &view, const float zfar, const float znear, CPointSpritesShader* shader, const int wH);
	void RenderFullscreenQuad();
	void BlurDepthPass(const glm::mat4 &mvp, CTexture2D* depthTexture, const float dirX, const float dirY);
	void WaterPass(const glm::mat4 &mvp, CCamera &cam, CTexture2D* depthTexture, CTexture2D* thicknessTexture, const FluidColor *color, const int showType);
	int CalcFBOSize(int size, float factor) { return (int)(size * factor); }
public:
	CScreenSpaceFluidRendering(const int width, const int height, const float particleRadius);
	~CScreenSpaceFluidRendering(void);
	void Render(CCamera &cam, const unsigned int numPointSprites, const SSFDrawingOptions &dstate, const int wW, const int wH);
	void SetRenderer(CRenderer* value) { renderer = value; }
	void SetPointSprites(CSphericalPointSprites* value) { pointSprites = value; }
	void SetPointSpritesShader(CPointSpritesShader* value) { pointSpritesShader = value; }
	void SetSceneTexture(CTexture* texture) { sceneTexture = texture; }
	void SetSkyboxCubemap(CTexture* cubemap) { skyboxCubemap = cubemap; }
	bool IsSupported() { return fullFrameBuffer != NULL; }
	void SetFBOFactor(float factor) {
		if (factor > 1.0f) factor = 1.0f;
		if (factor < 0.0f) factor = 0.0f;
		newFBOFactor = factor;
	}
	void setParticleRadius(const float value) { particleRadius = value; }
};

