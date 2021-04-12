#include "AllShaders.h"

void CLightingShader::updateUniformLocations() {
	ulocColor = getUniformLocation("color");
}

void CSkyboxShader::updateUniformLocations() {
	ulocMVP = getUniformLocation("mvp");
	ulocCubemap = getUniformLocation("cubemap");
}

void CPointSpritesShader::updateUniformLocations() {
	ulocPointScale = getUniformLocation("pointScale");
	ulocPointRadius = getUniformLocation("pointRadius");
	ulocNear = getUniformLocation("near");
	ulocFar = getUniformLocation("far");
	ulocViewMat = getUniformLocation("viewMat");
	ulocProjMat = getUniformLocation("projMat");
}
void CDepthBlurShader::updateUniformLocations() {
	ulocDepthTex = getUniformLocation("depthTex");
	ulocScale = getUniformLocation("scale");
	ulocRadius = getUniformLocation("radius");
	ulocMinDepth = getUniformLocation("minDepth");
	ulocMVPMat = getUniformLocation("mvpMat");
}

void CWaterShader::updateUniformLocations() {
	ulocDepthTex = getUniformLocation("depthTex");
	ulocThicknessTex = getUniformLocation("thicknessTex");
	ulocSceneTex = getUniformLocation("sceneTex");
	ulocSkyboxCubemap = getUniformLocation("skyboxCubemap");
	ulocXFactor = getUniformLocation("xFactor");
	ulocYFactor = getUniformLocation("yFactor");
	ulocZNear = getUniformLocation("zNear");
	ulocZFar = getUniformLocation("zFar");
	ulocMinDepth = getUniformLocation("minDepth");
	ulocColorFalloff = getUniformLocation("colorFalloff");
	ulocFluidColor = getUniformLocation("fluidColor");
	ulocShowType = getUniformLocation("showType");
	ulocMVPMat = getUniformLocation("mvpMat");
	ulocFalloffScale = getUniformLocation("falloffScale");
}
