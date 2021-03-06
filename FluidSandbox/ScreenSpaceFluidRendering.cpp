/*
======================================================================================================================
	Fluid Sandbox - ScreenSpaceFluidRendering.cpp

	Copyright (C) Torsten Spaete 2011-2021. All rights reserved.
	MPL v2 licensed. See LICENSE.txt for more details.
======================================================================================================================
*/

#include "ScreenSpaceFluidRendering.h"

CScreenSpaceFluidRendering::CScreenSpaceFluidRendering(const int width, const int height, CRenderer *renderer, CTextureCubemap *skyboxCubemap, CTexture2D *sceneTexture, CSphericalPointSprites *pointSprites, GeometryVBO *fullscreenQuad):
	renderer(renderer),
	pointSprites(pointSprites),
	fullscreenQuad(fullscreenQuad),
	fullFrameBuffer(nullptr),
	depthFrameBuffer(nullptr),
	pointSpritesShader(nullptr),
	pointsShader(nullptr),
	depthShader(nullptr),
	thicknessShader(nullptr),
	depthBlurShader(nullptr),
	clearWaterShader(nullptr),
	colorWaterShader(nullptr),
	debugWaterShader(nullptr),
	sceneTexture(sceneTexture),
	skyboxCubemap(skyboxCubemap),
	curFBOFactor(1.0f),
	newFBOFactor(1.0f),
	curFBOWidth(CalcFBOSize(width, curFBOFactor)),
	curFBOHeight(CalcFBOSize(height, curFBOFactor)),
	curWindowWidth(width),
	curWindowHeight(height) {

	// Check if max color attachments is at least 4
	if(CFBO::getMaxColorAttachments() >= 4) {
		// Create frame buffer object for depth
		depthFrameBuffer = new CSSFRDepthFBO(curFBOWidth, curFBOHeight);
		depthFrameBuffer->depthTexture = depthFrameBuffer->addRenderTarget(GL_DEPTH_COMPONENT32, GL_DEPTH_COMPONENT, GL_FLOAT, GL_DEPTH_ATTACHMENT, GL_NEAREST); // Depth
		depthFrameBuffer->colorTexture = depthFrameBuffer->addTextureTarget(GL_RGB32F, GL_RGBA, GL_FLOAT, GL_COLOR_ATTACHMENT0, GL_LINEAR); // Color
		depthFrameBuffer->update();

		// Create frame buffer object
		fullFrameBuffer = new CSSFRFullFBO(curFBOWidth, curFBOHeight);
		fullFrameBuffer->thicknessTexture = fullFrameBuffer->addTextureTarget(GL_RGB32F, GL_RGBA, GL_FLOAT, GL_COLOR_ATTACHMENT0, GL_NEAREST); // Thickness
		fullFrameBuffer->depthSmoothATexture = fullFrameBuffer->addTextureTarget(GL_RGB32F, GL_RGBA, GL_FLOAT, GL_COLOR_ATTACHMENT1, GL_NEAREST); // Depth smooth A
		fullFrameBuffer->depthSmoothBTexture = fullFrameBuffer->addTextureTarget(GL_RGB32F, GL_RGBA, GL_FLOAT, GL_COLOR_ATTACHMENT2, GL_NEAREST); // Depth smooth B
		fullFrameBuffer->waterTexture = fullFrameBuffer->addTextureTarget(GL_RGB32F, GL_RGBA, GL_FLOAT, GL_COLOR_ATTACHMENT3, GL_LINEAR); // Water
		fullFrameBuffer->update();

		// Create shaders
		{
			std::string pointSpritesShaderPath = std::string("shaders\\" + std::string(CPointSpritesShader::ShaderName));
			pointSpritesShader = new CPointSpritesShader();
			Utils::attachShaderFromFile(pointSpritesShader, GL_VERTEX_SHADER, (pointSpritesShaderPath + ".vertex").c_str(), "    ");
			Utils::attachShaderFromFile(pointSpritesShader, GL_FRAGMENT_SHADER, (pointSpritesShaderPath + ".fragment").c_str(), "    ");
		}
		{
			std::string pointsShaderPath = std::string("shaders\\" + std::string(CPointsShader::ShaderName));
			pointsShader = new CPointsShader();
			Utils::attachShaderFromFile(pointsShader, GL_VERTEX_SHADER, (pointsShaderPath + ".vertex").c_str(), "    ");
			Utils::attachShaderFromFile(pointsShader, GL_FRAGMENT_SHADER, (pointsShaderPath + ".fragment").c_str(), "    ");
		}
		{
			std::string depthShaderPath = std::string("shaders\\" + std::string(CDepthShader::ShaderName));
			depthShader = new CDepthShader();
			Utils::attachShaderFromFile(depthShader, GL_VERTEX_SHADER, (depthShaderPath + ".vertex").c_str(), "    ");
			Utils::attachShaderFromFile(depthShader, GL_FRAGMENT_SHADER, (depthShaderPath + ".fragment").c_str(), "    ");
		}
		{
			std::string thicknessShaderPath = std::string("shaders\\" + std::string(CThicknessShader::ShaderName));
			thicknessShader = new CThicknessShader();
			Utils::attachShaderFromFile(thicknessShader, GL_VERTEX_SHADER, (thicknessShaderPath + ".vertex").c_str(), "    ");
			Utils::attachShaderFromFile(thicknessShader, GL_FRAGMENT_SHADER, (thicknessShaderPath + ".fragment").c_str(), "    ");
		}
		{
			std::string depthBlurShaderPath = std::string("shaders\\" + std::string(CDepthBlurShader::ShaderName));
			depthBlurShader = new CDepthBlurShader();
			Utils::attachShaderFromFile(depthBlurShader, GL_VERTEX_SHADER, (depthBlurShaderPath + ".vertex").c_str(), "    ");
			Utils::attachShaderFromFile(depthBlurShader, GL_FRAGMENT_SHADER, (depthBlurShaderPath + ".fragment").c_str(), "    ");
		}
		{
			std::string clearWaterShaderPath = std::string("shaders\\" + std::string(CWaterShader::ClearName));
			clearWaterShader = new CWaterShader();
			Utils::attachShaderFromFile(clearWaterShader, GL_VERTEX_SHADER, (clearWaterShaderPath + ".vertex").c_str(), "    ");
			Utils::attachShaderFromFile(clearWaterShader, GL_FRAGMENT_SHADER, (clearWaterShaderPath + ".fragment").c_str(), "    ");
		}
		{
			std::string colorWaterShaderPath = std::string("shaders\\" + std::string(CWaterShader::ColorName));
			colorWaterShader = new CWaterShader();
			Utils::attachShaderFromFile(colorWaterShader, GL_VERTEX_SHADER, (colorWaterShaderPath + ".vertex").c_str(), "    ");
			Utils::attachShaderFromFile(colorWaterShader, GL_FRAGMENT_SHADER, (colorWaterShaderPath + ".fragment").c_str(), "    ");
		}
		{
			std::string debugWaterShaderPath = std::string("shaders\\" + std::string(CWaterShader::DebugName));
			debugWaterShader = new CWaterShader();
			Utils::attachShaderFromFile(debugWaterShader, GL_VERTEX_SHADER, (debugWaterShaderPath + ".vertex").c_str(), "    ");
			Utils::attachShaderFromFile(debugWaterShader, GL_FRAGMENT_SHADER, (debugWaterShaderPath + ".fragment").c_str(), "    ");
		}

		printf("    Screen space fluid rendering is supported.\n");
	} else
		printf("    Warning: Screen space fluid rendering is not supported on this hardware!\n");
}

CScreenSpaceFluidRendering::~CScreenSpaceFluidRendering(void) {
	// Release shaders
	CGLSL *shaders[] = { pointSpritesShader, pointsShader, depthShader, thicknessShader, depthBlurShader, clearWaterShader, colorWaterShader, debugWaterShader };
	int shaderCount = sizeof(shaders) / sizeof(shaders[0]);
	for(int i = shaderCount - 1; i > 0; i--) {
		if(shaders[i])
			delete shaders[i];
	}

	// Release framebuffer
	if(depthFrameBuffer)
		delete depthFrameBuffer;
	if(fullFrameBuffer)
		delete fullFrameBuffer;

	// Release pointers
	sceneTexture = nullptr;
	pointSprites = nullptr;
	pointSpritesShader = nullptr;
	renderer = nullptr;
}

void CScreenSpaceFluidRendering::DepthPass(const uint32_t numPointSprites, const glm::mat4 &proj, const glm::mat4 &view, const float zfar, const float znear, const int wH, const float particleRadius) {
	depthShader->enable();
	depthShader->uniform1f(depthShader->ulocPointScale, CSphericalPointSprites::GetPointScale(wH, 50.0f));
	depthShader->uniform1f(depthShader->ulocPointRadius, particleRadius);
	depthShader->uniform1f(depthShader->ulocNear, znear);
	depthShader->uniform1f(depthShader->ulocFar, zfar);
	depthShader->uniformMatrix4(depthShader->ulocViewMat, &view[0][0]);
	depthShader->uniformMatrix4(depthShader->ulocProjMat, &proj[0][0]);
	pointSprites->Draw(numPointSprites);
	depthShader->disable();
}

void CScreenSpaceFluidRendering::ThicknessPass(const uint32_t numPointSprites, const glm::mat4 &proj, const glm::mat4 &view, const float zfar, const float znear, const int wH, const float particleRadius) {
	renderer->ClearColor(0, 0, 0, 0);
	renderer->Clear(ClearFlags::Color);
	renderer->SetBlendFunc(GL_ONE, GL_ONE);
	renderer->SetBlending(true);
	renderer->SetDepthMask(false);

	thicknessShader->enable();
	thicknessShader->uniform1f(thicknessShader->ulocPointScale, CSphericalPointSprites::GetPointScale(wH, 50.0f));
	thicknessShader->uniform1f(thicknessShader->ulocPointRadius, particleRadius * 2.0f);
	thicknessShader->uniform1f(thicknessShader->ulocNear, znear);
	thicknessShader->uniform1f(thicknessShader->ulocFar, zfar);
	thicknessShader->uniformMatrix4(thicknessShader->ulocViewMat, &view[0][0]);
	thicknessShader->uniformMatrix4(thicknessShader->ulocProjMat, &proj[0][0]);

	pointSprites->Draw(numPointSprites);

	thicknessShader->disable();

	renderer->SetDepthMask(true);
	renderer->SetBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	renderer->SetBlending(false);
}

void CScreenSpaceFluidRendering::RenderPoints(const uint32_t numPointSprites, const glm::mat4 &mvp, const glm::vec4 &color) {
	CPointsShader *shader = pointsShader;
	shader->enable();
	shader->uniformMatrix4(shader->ulocMVP, &mvp[0][0]);
	shader->uniform4f(shader->ulocColor, &color[0]);
	pointSprites->Draw(numPointSprites);
	shader->disable();
}

void CScreenSpaceFluidRendering::RenderPointSprites(const uint32_t numPointSprites, const glm::mat4 &proj, const glm::mat4 &view, const float zfar, const float znear, const int wH, const float particleRadius, const glm::vec4 &color) {
	CPointSpritesShader *shader = pointSpritesShader;
	shader->enable();
	shader->uniform1f(shader->ulocPointScale, CSphericalPointSprites::GetPointScale(wH, 50.0f));
	shader->uniform1f(shader->ulocPointRadius, particleRadius);
	shader->uniform1f(shader->ulocNear, znear);
	shader->uniform1f(shader->ulocFar, zfar);
	shader->uniform4f(shader->ulocColor, &color[0]);
	shader->uniformMatrix4(shader->ulocViewMat, &view[0][0]);
	shader->uniformMatrix4(shader->ulocProjMat, &proj[0][0]);
	pointSprites->Draw(numPointSprites);
	shader->disable();
}

void CScreenSpaceFluidRendering::RenderFullscreenQuad() {
	renderer->DrawPrimitive(fullscreenQuad, false);
}

void CScreenSpaceFluidRendering::BlurDepthPass(const glm::mat4 &mvp, CTexture2D *depthTexture, const float dirX, const float dirY) {
	renderer->SetDepthTest(false);
	renderer->SetDepthMask(false);

	// Clear buffer
	renderer->Clear(ClearFlags::Color);

	// Enable depth texture
	renderer->EnableTexture(0, depthTexture);

	// Process depth smooth shader on a fullscreen quad if active
	depthBlurShader->enable();
	depthBlurShader->uniform1i(depthBlurShader->ulocDepthTex, 0);
	depthBlurShader->uniform2f(depthBlurShader->ulocScale, dirX, dirY);
	depthBlurShader->uniform1f(depthBlurShader->ulocRadius, 10.0f);
	depthBlurShader->uniform1f(depthBlurShader->ulocMinDepth, MIN_DEPTH);
	depthBlurShader->uniformMatrix4(depthBlurShader->ulocMVPMat, &mvp[0][0]);
	RenderFullscreenQuad();
	depthBlurShader->disable();

	// Unbind textures
	renderer->DisableTexture(0, depthTexture);

	renderer->SetDepthTest(true);
	renderer->SetDepthMask(true);
}

void CScreenSpaceFluidRendering::WaterPass(const CCamera &cam, const glm::mat4 &mvp, CTexture2D *depthTexture, CTexture2D *thicknessTexture, const FluidColor &color, const FluidDebugType showType) {
	// Bind 3 textures (Depth, Thickness, Scene)
	renderer->SetBlending(true);
	renderer->EnableTexture(0, depthTexture); // Depth Texture0
	renderer->EnableTexture(1, thicknessTexture); // Thickness Texture1
	renderer->EnableTexture(2, sceneTexture); // Scene Texture2
	renderer->EnableTexture(3, skyboxCubemap); // Skybox Texture3 (Cubemap)

	// Process normal and shading shader on a fullscreen quad
	CWaterShader *shader;
	if(showType == FluidDebugType::Final)
		shader = !color.isClear ? colorWaterShader : clearWaterShader;
	else
		shader = debugWaterShader;
	shader->enable();
	shader->uniform1i(shader->ulocDepthTex, 0);
	shader->uniform1i(shader->ulocThicknessTex, 1);
	shader->uniform1i(shader->ulocSceneTex, 2);
	shader->uniform1i(shader->ulocSkyboxCubemap, 3);
	shader->uniform1f(shader->ulocXFactor, 1.0f / ((float)curFBOWidth));
	shader->uniform1f(shader->ulocYFactor, 1.0f / ((float)curFBOHeight));
	shader->uniform1f(shader->ulocZFar, cam.farClip);
	shader->uniform1f(shader->ulocZNear, cam.nearClip);
	shader->uniform1f(shader->ulocMinDepth, MIN_DEPTH);
	shader->uniform4f(shader->ulocColorFalloff, (GLfloat *)&color.falloff[0]);
	shader->uniform1f(shader->ulocFalloffScale, color.falloffScale);

	shader->uniform4f(shader->ulocFluidColor, (GLfloat *)&color.color[0]);
	shader->uniform1i(shader->ulocShowType, (int)showType);

	shader->uniformMatrix4(shader->ulocMVPMat, &mvp[0][0]);
	RenderFullscreenQuad();
	shader->disable();

	// Unbind 4 textures
	renderer->DisableTexture(3, skyboxCubemap);
	renderer->DisableTexture(2, sceneTexture);
	renderer->DisableTexture(1, thicknessTexture);
	renderer->DisableTexture(0, depthTexture);
	renderer->SetBlending(false);
}

void CScreenSpaceFluidRendering::RenderSSF(const CCamera &cam, const uint32_t numPointSprites, const SSFDrawingOptions &dstate, const int wW, const int wH, const float particleRadius) {
	assert(fullFrameBuffer);
	assert(depthFrameBuffer);

	// Resize FBO if needed
	if((wW != curWindowWidth) ||
		(wH != curWindowHeight) ||
		(curFBOFactor != newFBOFactor)) {
		curWindowWidth = wW;
		curWindowHeight = wH;
		curFBOFactor = newFBOFactor;
		curFBOWidth = CalcFBOSize(wW, curFBOFactor);
		curFBOHeight = CalcFBOSize(wH, curFBOFactor);
		fullFrameBuffer->resize(curFBOWidth, curFBOHeight);
		depthFrameBuffer->resize(curFBOWidth, curFBOHeight);
	}

	// Retrieve texture pointers
	CTexture2D *depthTexture = depthFrameBuffer->depthTexture;
	CTexture2D *colorTexture = depthFrameBuffer->colorTexture;

	CTexture2D *thicknessTexture = fullFrameBuffer->thicknessTexture;
	CTexture2D *depthSmoothATexture = fullFrameBuffer->depthSmoothATexture;
	CTexture2D *depthSmoothBTexture = fullFrameBuffer->depthSmoothBTexture;
	CTexture2D *waterTexture = fullFrameBuffer->waterTexture;

	// Save latest draw buffer
	GLint latestDrawBuffer = fullFrameBuffer->getDrawBuffer();

	// Get required matrices
	glm::mat4 mvp = cam.mvp;
	glm::mat4 mview = cam.modelview;
	glm::mat4 mproj = cam.projection;

	// Calculate ortho matrix
	glm::mat4 orthoMVP = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f); // Unit-Cube OpenGL

	// Get depth range
	float nf[2];
	glGetFloatv(GL_DEPTH_RANGE, nf);
	float near_depth = nf[0];
	float far_depth = nf[1];

	// Set view and scissor
	renderer->SetViewport(0, 0, curFBOWidth, curFBOHeight);
	renderer->SetScissor(0, 0, curFBOWidth, curFBOHeight);

	// Pass 1: Render point sprites to depth and color
	depthFrameBuffer->enable();
	depthFrameBuffer->setDrawBuffer(GL_COLOR_ATTACHMENT0);
	renderer->ClearColor(-10000.0f, 0.0f, 0.0f, 0.0f);
	renderer->Clear(ClearFlags::Color | ClearFlags::Depth);
	DepthPass(numPointSprites, mproj, mview, far_depth, near_depth, curFBOHeight, particleRadius);
	depthFrameBuffer->disable();

	// Enable FBO
	fullFrameBuffer->enable();

	// Pass 2: Render point sprites to thickness
	// -------------------------------------
	fullFrameBuffer->setDrawBuffer(GL_COLOR_ATTACHMENT0); // Draw to color attachment 0: Thickness
	ThicknessPass(numPointSprites, mproj, mview, far_depth, near_depth, curFBOHeight, particleRadius);

	if(dstate.blurEnabled) {
		// Pass 3: Blur depth A
		// -------------------------------------
		fullFrameBuffer->setDrawBuffer(GL_COLOR_ATTACHMENT1); // Draw to color attachment 1: Smooth depth A
		BlurDepthPass(orthoMVP, colorTexture, dstate.blurScale, 0.0f);

		// Pass 4: Blur depth B
		// -------------------------------------
		fullFrameBuffer->setDrawBuffer(GL_COLOR_ATTACHMENT2); // Draw to color attachment 2: Smooth depth B
		BlurDepthPass(orthoMVP, depthSmoothATexture, 0.0f, dstate.blurScale);
	} else {
		depthSmoothBTexture = colorTexture;
	}

	// Disable FBO
	fullFrameBuffer->disable();

	// Restore latest draw buffer
	fullFrameBuffer->setDrawBuffer(latestDrawBuffer);

	// Set view and scissor
	renderer->SetViewport(0, 0, curWindowWidth, curWindowHeight);
	renderer->SetScissor(0, 0, curWindowWidth, curWindowHeight);

	// Pass 5: Water rendering
	WaterPass(cam, orthoMVP, depthSmoothBTexture, thicknessTexture, dstate.fluidColor, dstate.debugType);
}

void CScreenSpaceFluidRendering::Render(const CCamera &cam, const uint32_t numPointSprites, const SSFDrawingOptions &dstate, const int wW, const int wH, const float particleRadius) {
	assert(pointSpritesShader);
	assert(pointSprites);

	glm::mat4 mproj = cam.projection;
	glm::mat4 mview = cam.modelview;
	glm::mat4 mvp = cam.mvp;

	bool waterIsColored = !dstate.fluidColor.isClear;

	// Render SSF or not
	switch(dstate.renderMode) {
		case SSFRenderMode::PointSprites:
		{
			glm::vec4 color = dstate.fluidColor.color;
			if(waterIsColored)
				color = dstate.fluidColor.color;
			else
				color = glm::vec4(1, 1, 1, 1);
			RenderPointSprites(numPointSprites, mproj, mview, cam.farClip, cam.nearClip, wH, particleRadius, color);
			break;
		}
		case SSFRenderMode::Points:
		{
			glm::vec4 color = dstate.fluidColor.color;
			if(waterIsColored)
				color = dstate.fluidColor.color;
			else
				color = glm::vec4(1, 1, 1, 1);
			RenderPoints(numPointSprites, mvp, color);
			break;
		}
		case SSFRenderMode::Fluid:
		{
			RenderSSF(cam, numPointSprites, dstate, wW, wH, particleRadius);
			break;
		}
	}
}