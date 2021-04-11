#include "ScreenSpaceFluidRendering.h"

#include "TextureIDs.h"

//#define BETTERBLUR

const char* SSFShaderFiles[SSFShaderCount] = {"Depth", "Thickness", "DepthBlur", "ClearWater", "ColorWater", "Debug"};

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
		for (int i = 0; i < SSFShaderCount; i++)
		{
			std::string temp = "shaders\\";
			temp += SSFShaderFiles[i];
			aShaders[i] = new CGLSL();
			Utils::attachShaderFromFile(aShaders[i], GL_VERTEX_SHADER, (temp + ".vertex").c_str(), "    ");
			Utils::attachShaderFromFile(aShaders[i], GL_FRAGMENT_SHADER, (temp + ".fragment").c_str(), "    ");
		}
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
	CGLSL* shader = aShaders[SSFShaderIndex_DepthPass];
	shader->enable();
	shader->uniform1f(shader->getUniformLocation("pointScale"), CSphericalPointSprites::GetPointScale(wH, 50.0f));
	shader->uniform1f(shader->getUniformLocation("pointRadius"), particleRadius);
	shader->uniform1f(shader->getUniformLocation("near"), znear);
	shader->uniform1f(shader->getUniformLocation("far"), zfar);
	shader->uniformMatrix4(shader->getUniformLocation("viewMat"), &view[0][0]);
	shader->uniformMatrix4(shader->getUniformLocation("projMat"), &proj[0][0]);
	pPointSprites->Draw(numPointSprites);
	shader->disable();
}

void CScreenSpaceFluidRendering::ThicknessPass(unsigned int numPointSprites, const glm::mat4 &proj, const glm::mat4 &view, float zfar, float znear, int wH)
{
	CGLSL* shader = aShaders[SSFShaderIndex_ThicknessPass];

	pRenderer->ClearColor(0,0,0,0);
	pRenderer->Clear(ClearFlags::Color);
	pRenderer->SetBlendFunc(GL_ONE, GL_ONE);
	pRenderer->SetBlending(true);
	pRenderer->SetDepthMask(false);

	shader->enable();
	shader->uniform1f(shader->getUniformLocation("pointScale"), CSphericalPointSprites::GetPointScale(wH, 50.0f));
	shader->uniform1f(shader->getUniformLocation("pointRadius"), particleRadius * 2.0f);
	shader->uniform1f(shader->getUniformLocation("near"), znear);
	shader->uniform1f(shader->getUniformLocation("far"), zfar);
	shader->uniformMatrix4(shader->getUniformLocation("viewMat"), &view[0][0]);
	shader->uniformMatrix4(shader->getUniformLocation("projMat"), &proj[0][0]);

	pPointSprites->Draw(numPointSprites);

	shader->disable();

	pRenderer->SetDepthMask(true);
	pRenderer->SetBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	pRenderer->SetBlending(false);
}

void CScreenSpaceFluidRendering::RenderPointSprites(unsigned int numPointSprites, glm::mat4 &proj, glm::mat4 &view, float zfar, float znear, CGLSL* shader, int wH)
{
	if (shader != NULL){
		shader->enable();
		shader->uniform1f(shader->getUniformLocation("pointScale"), CSphericalPointSprites::GetPointScale(wH, 50.0f));
		shader->uniform1f(shader->getUniformLocation("pointRadius"), particleRadius);
		shader->uniform1f(shader->getUniformLocation("near"), znear);
		shader->uniform1f(shader->getUniformLocation("far"), zfar);
		shader->uniformMatrix4(shader->getUniformLocation("viewMat"), &view[0][0]);
		shader->uniformMatrix4(shader->getUniformLocation("projMat"), &proj[0][0]);
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
	CGLSL* shader = aShaders[SSFShaderIndex_DepthBlurFast];
	shader->enable();
	shader->uniform1i(shader->getUniformLocation("depthTex"), 0);
	shader->uniform2f(shader->getUniformLocation("scale"), dirX, dirY);
	shader->uniform1f(shader->getUniformLocation("radius"), 10.0f);
	shader->uniform1f(shader->getUniformLocation("minDepth"), MIN_DEPTH);
	shader->uniformMatrix4(shader->getUniformLocation("mvpMat"), &mvp[0][0]);
	RenderFullscreenQuad();
	shader->disable();

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
	CGLSL* shader;
	if (showType == 0)
		shader = aShaders[!color->isClear ? SSFShaderIndex_ColorWater : SSFShaderIndex_ClearWater];
	else
		shader = aShaders[SSFShaderIndex_Debug];
	shader->enable();
	shader->uniform1i(shader->getUniformLocation("depthTex"), 0);
	shader->uniform1i(shader->getUniformLocation("thicknessTex"), 1);
	shader->uniform1i(shader->getUniformLocation("sceneTex"), 2);
	shader->uniform1i(shader->getUniformLocation("skyboxCubemap"), 3);
	shader->uniform1f(shader->getUniformLocation("xFactor"), 1.0f/((float)iFBOWidth));
	shader->uniform1f(shader->getUniformLocation("yFactor"), 1.0f/((float)iFBOHeight));
	shader->uniform1f(shader->getUniformLocation("zFar"), cam.GetFarClip());
	shader->uniform1f(shader->getUniformLocation("zNear"), cam.GetNearClip());
	shader->uniform1f(shader->getUniformLocation("minDepth"), MIN_DEPTH);
	shader->uniform4f(shader->getUniformLocation("colorFalloff"), (GLfloat*)&color->falloff[0]);
	shader->uniform4f(shader->getUniformLocation("fluidColor"), (GLfloat*)&color->color[0]);
	shader->uniform1i(shader->getUniformLocation("showType"), showType);
	shader->uniformMatrix4(shader->getUniformLocation("mvpMat"), &mvp[0][0]);
	shader->uniform1f(shader->getUniformLocation("falloffScale"), color->falloffScale);
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