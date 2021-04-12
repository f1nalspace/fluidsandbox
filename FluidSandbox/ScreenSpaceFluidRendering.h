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
	bool isClear;
	std::string name;
	float falloffScale;
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
	unsigned int renderMode;
	unsigned int textureState;
	FluidColor* fluidColor;
	glm::vec3 clearColor;
	float blurScale;
	bool blurEnabled;
	int debugType;
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

	CFBO *cFrameBuffer;
	CFBO *cFrameBufferDepth;
	CRenderer *pRenderer;
	CSphericalPointSprites *pPointSprites;
	CPointSpritesShader *pPointSpritesShader;

	CDepthShader *depthShader;
	CThicknessShader *thicknessShader;
	CWa *depthBlurShader;
	CWaterShader *clearWaterShader;
	CWaterShader *colorWaterShader;
	CWaterShader *debugWaterShader;

	CTexture *pSceneTexture;
	CTexture *pSkyboxCubemap;

	int iFBOWidth;
	int iFBOHeight;
	int iWindowWidth;
	int iWindowHeight;
	float fFBOFactor;
	float fNewFBOFactor;
	float particleRadius;

	void DepthPass(unsigned int numPointSprites, const glm::mat4 &proj, const glm::mat4 &view, float zfar, float znear, int wH);
	void ThicknessPass(unsigned int numPointSprites, const glm::mat4 &proj, const glm::mat4 &view, float zfar, float znear, int wH);
	void RenderSSF(CCamera &cam, unsigned int numPointSprites, SSFDrawingOptions &dstate, int wW, int wH);
	void RenderPointSprites(unsigned int numPointSprites, glm::mat4 &proj, glm::mat4 &view, float zfar, float znear, CPointSpritesShader* shader, int wH);
	void RenderFullscreenQuad();
	void BlurDepthPass(const glm::mat4 &mvp, CTexture2D* depthTexture, float dirX, float dirY);
	void WaterPass(const glm::mat4 &mvp, CCamera &cam, CTexture2D* depthTexture, CTexture2D* thicknessTexture, FluidColor *color, const int showType);
	int CalcFBOSize(int size, float factor) { return (int)(size * factor); }
public:
	CScreenSpaceFluidRendering(int width, int height, float particleRadius);
	~CScreenSpaceFluidRendering(void);
	void Render(CCamera &cam, unsigned int numPointSprites, SSFDrawingOptions &dstate, int wW, int wH);
	void SetRenderer(CRenderer* value) { pRenderer = value; }
	void SetPointSprites(CSphericalPointSprites* value) { pPointSprites = value; }
	void SetPointSpritesShader(CPointSpritesShader* value) { pPointSpritesShader = value; }
	void SetSceneTexture(CTexture* texture) { pSceneTexture = texture; }
	void SetSkyboxCubemap(CTexture* cubemap) { pSkyboxCubemap = cubemap; }
	bool IsSupported() { return cFrameBuffer != NULL; }
	void SetFBOFactor(float factor) {
		if (factor > 1.0f) factor = 1.0f;
		if (factor < 0.0f) factor = 0.0f;
		fNewFBOFactor = factor; 
	}
	void setParticleRadius(const float value) { particleRadius = value; }
};

