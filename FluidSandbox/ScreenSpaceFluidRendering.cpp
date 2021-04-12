#include "ScreenSpaceFluidRendering.h"

#include "TextureIDs.h"

//#define BETTERBLUR

CScreenSpaceFluidRendering::CScreenSpaceFluidRendering(int width, int height, float particleRadius)
{
	this->particleRadius = particleRadius;
	iWindowWidth = width;
	iWindowHeight = height;
	fFBOFactor = 1.0f;
	fNewFBOFactor = 1.0f;
	iFBOWidth = CalcFBOSize(width, fFBOFactor);
	iFBOHeight = CalcFBOSize(height, fFBOFactor);

	// Init pointers
	pRenderer = NULL;
	pPointSprites = NULL;
	pPointSpritesShader = NULL;
	pSceneTexture = NULL;
	pSkyboxCubemap = NULL;

	// Check if max color attachments is at least 4
	if (CFBO::getMaxColorAttachments() >= 4)
	{
		// Create frame buffer object for depth
		cFrameBufferDepth = new CFBO(iFBOWidth, iFBOHeight);
		cFrameBufferDepth->addRenderTarget(GL_DEPTH_COMPONENT32, GL_DEPTH_COMPONENT, GL_FLOAT, GL_DEPTH_ATTACHMENT, TEXTURE_ID_SSF_DEPTH, GL_NEAREST); // Depth
		cFrameBufferDepth->addTextureTarget(GL_RGB32F, GL_RGBA, GL_FLOAT, GL_COLOR_ATTACHMENT0, TEXTURE_ID_SSF_COLOR, GL_LINEAR); // Color
		cFrameBufferDepth->update();

		// Create frame buffer object
		cFrameBuffer = new CFBO(iFBOWidth, iFBOHeight);
		cFrameBuffer->addTextureTarget(GL_RGB32F, GL_RGBA, GL_FLOAT, GL_COLOR_ATTACHMENT0, TEXTURE_ID_SSF_THICKNESS, GL_NEAREST); // Thickness
		cFrameBuffer->addTextureTarget(GL_RGB32F, GL_RGBA, GL_FLOAT, GL_COLOR_ATTACHMENT1, TEXTURE_ID_SSF_DEPTH_SMOOTH_A, GL_NEAREST); // Depth smooth A
		cFrameBuffer->addTextureTarget(GL_RGB32F, GL_RGBA, GL_FLOAT, GL_COLOR_ATTACHMENT2, TEXTURE_ID_SSF_DEPTH_SMOOTH_B, GL_NEAREST); // Depth smooth B
		cFrameBuffer->addTextureTarget(GL_RGB32F, GL_RGBA, GL_FLOAT, GL_COLOR_ATTACHMENT3, TEXTURE_ID_SSF_WATER, GL_LINEAR); // Water
		cFrameBuffer->update();

		// Create shaders
		{
			std::string depthShaderPath = std::string("shaders\\" + std::string(CDepthShader::ShaderName));
			aShaders[SSFShaderIndex_DepthPass] = depthShader = new CDepthShader();
			Utils::attachShaderFromFile(depthShader, GL_VERTEX_SHADER, (depthShaderPath + ".vertex").c_str(), "    ");
			Utils::attachShaderFromFile(depthShader, GL_FRAGMENT_SHADER, (depthShaderPath + ".fragment").c_str(), "    ");
		}
		{
			std::string thicknessShaderPath = std::string("shaders\\" + std::string(CThicknessShader::ShaderName));
			aShaders[SSFShaderIndex_ThicknessPass] = thicknessShader = new CThicknessShader();
			Utils::attachShaderFromFile(thicknessShader, GL_VERTEX_SHADER, (thicknessShaderPath + ".vertex").c_str(), "    ");
			Utils::attachShaderFromFile(thicknessShader, GL_FRAGMENT_SHADER, (thicknessShaderPath + ".fragment").c_str(), "    ");
		}
		{
			std::string depthBlurShaderPath = std::string("shaders\\" + std::string(CWa::ShaderName));
			aShaders[SSFShaderIndex_DepthBlurFast] = depthBlurShader = new CWa();
			Utils::attachShaderFromFile(depthBlurShader, GL_VERTEX_SHADER, (depthBlurShaderPath + ".vertex").c_str(), "    ");
			Utils::attachShaderFromFile(depthBlurShader, GL_FRAGMENT_SHADER, (depthBlurShaderPath + ".fragment").c_str(), "    ");
		}
		{
			std::string clearWaterShaderPath = std::string("shaders\\" + std::string(CWaterShader::ClearName));
			aShaders[SSFShaderIndex_ClearWater] = clearWaterShader = new CWaterShader();
			Utils::attachShaderFromFile(clearWaterShader, GL_VERTEX_SHADER, (clearWaterShaderPath + ".vertex").c_str(), "    ");
			Utils::attachShaderFromFile(clearWaterShader, GL_FRAGMENT_SHADER, (clearWaterShaderPath + ".fragment").c_str(), "    ");
		}
		{
			std::string colorWaterShaderPath = std::string("shaders\\" + std::string(CWaterShader::ColorName));
			aShaders[SSFShaderIndex_ColorWater] = colorWaterShader = new CWaterShader();
			Utils::attachShaderFromFile(colorWaterShader, GL_VERTEX_SHADER, (colorWaterShaderPath + ".vertex").c_str(), "    ");
			Utils::attachShaderFromFile(colorWaterShader, GL_FRAGMENT_SHADER, (colorWaterShaderPath + ".fragment").c_str(), "    ");
		}
		{
			std::string debugWaterShaderPath = std::string("shaders\\" + std::string(CWaterShader::DebugName));
			aShaders[SSFShaderIndex_DebugWater] = debugWaterShader = new CWaterShader();
			Utils::attachShaderFromFile(debugWaterShader, GL_VERTEX_SHADER, (debugWaterShaderPath + ".vertex").c_str(), "    ");
			Utils::attachShaderFromFile(debugWaterShader, GL_FRAGMENT_SHADER, (debugWaterShaderPath + ".fragment").c_str(), "    ");
		}

		for (int i = 0; i < SSFShaderCount; i++)
			assert(aShaders[i] != NULL);

		printf("    Screen space fluid rendering is supported.\n");
	}
	else
		printf("    Warning: Screen space fluid rendering is not supported on this hardware!\n");
}

CScreenSpaceFluidRendering::~CScreenSpaceFluidRendering(void)
{
	// Release shaders
	for (int i = SSFShaderCount-1; i > 0; i--)
		if (aShaders[i])
			delete aShaders[i];

	// Release framebuffer
	if (cFrameBufferDepth)
		delete cFrameBufferDepth;
	if (cFrameBuffer) 
		delete cFrameBuffer;

	// Release pointers
	pSceneTexture = NULL;
	pPointSprites = NULL;
	pPointSpritesShader = NULL;
	pRenderer = NULL;
}

void CScreenSpaceFluidRendering::DepthPass(unsigned int numPointSprites, const glm::mat4 &proj, const glm::mat4 &view, float zfar, float znear, int wH)
{
	depthShader->enable();
	depthShader->uniform1f(depthShader->ulocPointScale, CSphericalPointSprites::GetPointScale(wH, 50.0f));
	depthShader->uniform1f(depthShader->ulocPointRadius, particleRadius);
	depthShader->uniform1f(depthShader->ulocNear, znear);
	depthShader->uniform1f(depthShader->ulocFar, zfar);
	depthShader->uniformMatrix4(depthShader->ulocViewMat, &view[0][0]);
	depthShader->uniformMatrix4(depthShader->ulocProjMat, &proj[0][0]);
	pPointSprites->Draw(numPointSprites);
	depthShader->disable();
}

void CScreenSpaceFluidRendering::ThicknessPass(unsigned int numPointSprites, const glm::mat4 &proj, const glm::mat4 &view, float zfar, float znear, int wH)
{
	pRenderer->ClearColor(0,0,0,0);
	pRenderer->Clear(ClearFlags::Color);
	pRenderer->SetBlendFunc(GL_ONE, GL_ONE);
	pRenderer->SetBlending(true);
	pRenderer->SetDepthMask(false);

	thicknessShader->enable();
	thicknessShader->uniform1f(thicknessShader->ulocPointScale, CSphericalPointSprites::GetPointScale(wH, 50.0f));
	thicknessShader->uniform1f(thicknessShader->ulocPointRadius, particleRadius * 2.0f);
	thicknessShader->uniform1f(thicknessShader->ulocNear, znear);
	thicknessShader->uniform1f(thicknessShader->ulocFar, zfar);
	thicknessShader->uniformMatrix4(thicknessShader->ulocViewMat, &view[0][0]);
	thicknessShader->uniformMatrix4(thicknessShader->ulocProjMat, &proj[0][0]);

	pPointSprites->Draw(numPointSprites);

	thicknessShader->disable();

	pRenderer->SetDepthMask(true);
	pRenderer->SetBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	pRenderer->SetBlending(false);
}

void CScreenSpaceFluidRendering::RenderPointSprites(unsigned int numPointSprites, glm::mat4 &proj, glm::mat4 &view, float zfar, float znear, CPointSpritesShader* shader, int wH)
{
	if (shader != NULL){
		shader->enable();
		shader->uniform1f(shader->ulocPointScale, CSphericalPointSprites::GetPointScale(wH, 50.0f));
		shader->uniform1f(shader->ulocPointRadius, particleRadius);
		shader->uniform1f(shader->ulocNear, znear);
		shader->uniform1f(shader->ulocFar, zfar);
		shader->uniformMatrix4(shader->ulocViewMat, &view[0][0]);
		shader->uniformMatrix4(shader->ulocProjMat, &proj[0][0]);
	}
	pPointSprites->Draw(numPointSprites);
	if (shader != NULL){
		shader->disable();
	}
}

void CScreenSpaceFluidRendering::RenderFullscreenQuad()
{
	pRenderer->DrawTexturedQuad(0.0f,0.0f,1.0f,1.0f);
}

void CScreenSpaceFluidRendering::BlurDepthPass(const glm::mat4 &mvp, CTexture2D* depthTexture, float dirX, float dirY)
{
	pRenderer->SetDepthTest(false);
	pRenderer->SetDepthMask(false);

	// Clear buffer
	pRenderer->Clear(ClearFlags::Color);

	// Enable depth texture
	pRenderer->EnableTexture(0, depthTexture);

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
	pRenderer->DisableTexture(0, depthTexture);

	pRenderer->SetDepthTest(true);
	pRenderer->SetDepthMask(true);
}

void CScreenSpaceFluidRendering::WaterPass(const glm::mat4 &mvp, CCamera &cam, CTexture2D* depthTexture, CTexture2D* thicknessTexture, FluidColor *color, const int showType)
{
	// Bind 3 textures (Depth, Thickness, Scene)
	pRenderer->SetBlending(true);
	pRenderer->EnableTexture(0, depthTexture); // Depth Texture0
	pRenderer->EnableTexture(1, thicknessTexture); // Thickness Texture1
	pRenderer->EnableTexture(2, pSceneTexture); // Scene Texture2
	pRenderer->EnableTexture(3, pSkyboxCubemap); // Skybox Texture3 (Cubemap)

	// Process normal and shading shader on a fullscreen quad
	CWaterShader* shader;
	if (showType == 0)
		shader = !color->isClear ? colorWaterShader : clearWaterShader;
	else
		shader = debugWaterShader;
	shader->enable();
	shader->uniform1i(shader->ulocDepthTex, 0);
	shader->uniform1i(shader->ulocThicknessTex, 1);
	shader->uniform1i(shader->ulocSceneTex, 2);
	shader->uniform1i(shader->ulocSkyboxCubemap, 3);
	shader->uniform1f(shader->ulocXFactor, 1.0f/((float)iFBOWidth));
	shader->uniform1f(shader->ulocYFactor, 1.0f/((float)iFBOHeight));
	shader->uniform1f(shader->ulocZFar, cam.GetFarClip());
	shader->uniform1f(shader->ulocZNear, cam.GetNearClip());
	shader->uniform1f(shader->ulocMinDepth, MIN_DEPTH);
	shader->uniform4f(shader->ulocColorFalloff, (GLfloat*)&color->falloff[0]);
	shader->uniform1f(shader->ulocFalloffScale, color->falloffScale);

	shader->uniform4f(shader->ulocFluidColor, (GLfloat*)&color->color[0]);
	shader->uniform1i(shader->ulocShowType, showType);

	shader->uniformMatrix4(shader->ulocMVPMat, &mvp[0][0]);
	RenderFullscreenQuad();
	shader->disable();

	// Unbind 4 textures
	pRenderer->DisableTexture(3, pSkyboxCubemap);
	pRenderer->DisableTexture(2, pSceneTexture);
	pRenderer->DisableTexture(1, thicknessTexture);
	pRenderer->DisableTexture(0, depthTexture);
	pRenderer->SetBlending(false);
}

void CScreenSpaceFluidRendering::RenderSSF(CCamera &cam, unsigned int numPointSprites, SSFDrawingOptions &dstate, int wW, int wH)
{
	assert(cFrameBuffer);
	assert(pRenderer);

	// Resize FBO if needed
	if ((wW != iWindowWidth) ||
		(wH != iWindowHeight) ||
		(fFBOFactor != fNewFBOFactor))
	{
		iWindowWidth = wW;
		iWindowHeight = wH;
		fFBOFactor = fNewFBOFactor;
		iFBOWidth = CalcFBOSize(wW, fFBOFactor);
		iFBOHeight = CalcFBOSize(wH, fFBOFactor);
		cFrameBuffer->resize(iFBOWidth, iFBOHeight);
		cFrameBufferDepth->resize(iFBOWidth, iFBOHeight);
	}

	// Retrieve texture pointers
	CTexture2D* depthTexture = cFrameBufferDepth->getTexture(TEXTURE_ID_SSF_DEPTH);
	CTexture2D* colorTexture = cFrameBufferDepth->getTexture(TEXTURE_ID_SSF_COLOR);
	CTexture2D* thicknessTexture = cFrameBuffer->getTexture(TEXTURE_ID_SSF_THICKNESS);
	CTexture2D* depthSmoothATexture = cFrameBuffer->getTexture(TEXTURE_ID_SSF_DEPTH_SMOOTH_A);
	CTexture2D* depthSmoothBTexture = cFrameBuffer->getTexture(TEXTURE_ID_SSF_DEPTH_SMOOTH_B);
	CTexture2D* waterTexture = cFrameBuffer->getTexture(TEXTURE_ID_SSF_WATER);
	
	// Save latest draw buffer
	GLint latestDrawBuffer = cFrameBuffer->getDrawBuffer();

	// Get required matrices
	glm::mat4 mvp = cam.GetModelViewProjection();
	glm::mat4 mview = cam.GetModelview();
	glm::mat4 mproj = cam.GetProjection();

	// Calculate ortho matrix
	glm::mat4 orthoProj = glm::ortho(0.0f,1.0f,1.0f,0.0f);
	glm::mat4 orthoMVP = glm::mat4(1.0f) * orthoProj;

	// Change to 3D mvp
	pRenderer->LoadMatrix(mvp);

	// Get depth range
	float nf[2];
	glGetFloatv(GL_DEPTH_RANGE,nf);
	float near_depth = nf[0];
	float far_depth = nf[1];

	// Set view and scissor
	pRenderer->SetViewport(0,0,iFBOWidth,iFBOHeight);
	pRenderer->SetScissor(0,0,iFBOWidth,iFBOHeight);

	// From here, everything is white
	pRenderer->SetColor(1.0f,1.0f,1.0f,1.0f);

	// Pass 1: Render point sprites to depth and color
	cFrameBufferDepth->enable();
	cFrameBufferDepth->setDrawBuffer(GL_COLOR_ATTACHMENT0);
	pRenderer->ClearColor(-10000.0f,0.0f,0.0f,0.0f);
	pRenderer->Clear(ClearFlags::Color | ClearFlags::Depth);
	DepthPass(numPointSprites, mproj, mview, far_depth, near_depth, iFBOHeight);
	cFrameBufferDepth->disable();

	// Enable FBO
	cFrameBuffer->enable();

	// Pass 2: Render point sprites to thickness
	// -------------------------------------
	cFrameBuffer->setDrawBuffer(GL_COLOR_ATTACHMENT0); // Draw to color attachment 0: Thickness
	ThicknessPass(numPointSprites, mproj, mview, far_depth, near_depth, iFBOHeight);

	// Change to 2D mvp
	pRenderer->LoadMatrix(orthoMVP);

	if (dstate.blurEnabled) {
		// Pass 3: Blur depth A
		// -------------------------------------
		cFrameBuffer->setDrawBuffer(GL_COLOR_ATTACHMENT1); // Draw to color attachment 1: Smooth depth A
		BlurDepthPass(orthoMVP, colorTexture, dstate.blurScale, 0.0f);

		// Pass 4: Blur depth B
		// -------------------------------------
		cFrameBuffer->setDrawBuffer(GL_COLOR_ATTACHMENT2); // Draw to color attachment 2: Smooth depth B
		BlurDepthPass(orthoMVP, depthSmoothATexture, 0.0f, dstate.blurScale);
	} else {
		depthSmoothBTexture = colorTexture;
	}

	// Disable FBO
	cFrameBuffer->disable();

	// Restore latest draw buffer
	cFrameBuffer->setDrawBuffer(latestDrawBuffer);

	// Set view and scissor
	pRenderer->SetViewport(0,0,iWindowWidth,iWindowHeight);
	pRenderer->SetScissor(0,0,iWindowWidth,iWindowHeight);

	// Pass 5: Water rendering
	WaterPass(orthoMVP, cam, depthSmoothBTexture, thicknessTexture, dstate.fluidColor, dstate.debugType);
}

void CScreenSpaceFluidRendering::Render(CCamera &cam, unsigned int numPointSprites, SSFDrawingOptions &dstate, int wW, int wH)
{
	assert(pPointSpritesShader);
	assert(pPointSprites);

	glm::mat4 mview = cam.GetModelview();
	glm::mat4 mproj = cam.GetProjection();
	glm::mat4 mvp = cam.GetModelViewProjection();

	bool waterIsColored = !dstate.fluidColor->isClear;

	// Render SSF or not
	switch (dstate.renderMode)
	{
	case SSFRenderMode_PointSprites:
		{
			pRenderer->LoadMatrix(mvp);
			if (waterIsColored)
				pRenderer->SetColor(&dstate.fluidColor->color[0]);
			else
				pRenderer->SetColor(1,1,1,1);
			RenderPointSprites(numPointSprites, mproj, mview, cam.GetFarClip(), cam.GetNearClip(), pPointSpritesShader, wH);
			pRenderer->SetColor(1,1,1,1);
			break;
		}
	case SSFRenderMode_Points:
		{
			pRenderer->LoadMatrix(mvp);
			if (waterIsColored)
				pRenderer->SetColor(&dstate.fluidColor->color[0]);
			else
				pRenderer->SetColor(1,1,1,1);
			RenderPointSprites(numPointSprites, mproj, mview, cam.GetFarClip(), cam.GetNearClip(), NULL, wH);
			pRenderer->SetColor(1,1,1,1);
			break;
		}
	case SSFRenderMode_Fluid:
		{
			RenderSSF(cam, numPointSprites, dstate, wW, wH);
			break;
		}
	}
}