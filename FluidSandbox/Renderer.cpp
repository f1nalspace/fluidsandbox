#include "Renderer.h"

CRenderer::CRenderer(void)
{
	glClearDepth(1.0f);
	glDepthFunc(GL_LESS);

	depthTestEnabled = true;
	glEnable(GL_DEPTH_TEST);

	depthMaskEnabled = true;
	glDepthMask(GL_TRUE);

	cullFaceEnabled = true;
	glFrontFace(GL_CCW);
	glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);

	glDisable(GL_TEXTURE_3D);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_TEXTURE_1D);
	glDisable(GL_TEXTURE_CUBE_MAP);
	for (int i = 0; i < MAX_TEXTURES; i++){
		textureStates[i].active = false;
		textureStates[i].texture = NULL;
	}

	glShadeModel(GL_SMOOTH);
	glClearColor(0, 0, 0, 1);

	blendFunc[0] = GL_SRC_ALPHA;
	blendFunc[1] = GL_ONE_MINUS_SRC_ALPHA;
	glBlendFunc(blendFunc[0], blendFunc[1]);

	blendingEnabled = false;
	glDisable(GL_BLEND);

	wireframeEnabled = false;
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

CRenderer::~CRenderer(void)
{
}

void CRenderer::Clear(const ClearFlags flags)
{
	GLbitfield f = 0;
	if ((flags & ClearFlags::Color) == ClearFlags::Color) 
		f |= GL_COLOR_BUFFER_BIT;
	if ((flags & ClearFlags::Depth) == ClearFlags::Depth) 
		f |= GL_DEPTH_BUFFER_BIT;
	glClear(f);
}

void CRenderer::ClearColor(const float r, const float g, const float b, const float a)
{
	glClearColor(r, g, b, a);
}

void CRenderer::SetViewport(const int left, const int top, const int width, const int height)
{
	glViewport(left, top, width, height);
}

void CRenderer::SetScissor(const int left, const int top, const int width, const int height)
{
	glScissor(left, top, width, height);
}

void CRenderer::LoadMatrix(const glm::mat4 &m)
{
	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf(&m[0][0]);
}

void CRenderer::SetColor(const float r, const float g, const float b, const float a)
{
	glColor4f(r, g, b, a);
}

void CRenderer::SetColor(const float* color)
{
	glColor4fv(color);
}

void CRenderer::SetDepthTest(const bool enabled)
{
	assert(depthTestEnabled != enabled);
	depthTestEnabled = enabled;
	if (depthTestEnabled)
		glEnable(GL_DEPTH_TEST);
	else
		glDisable(GL_DEPTH_TEST);
}

void CRenderer::SetDepthMask(const bool enabled)
{
	assert(depthMaskEnabled != enabled);
	depthMaskEnabled = enabled;
	if (depthMaskEnabled)
		glDepthMask(GL_TRUE);
	else
		glDepthMask(GL_FALSE);
}

void CRenderer::SetCullFace(const bool enabled)
{
	assert(cullFaceEnabled != enabled);
	cullFaceEnabled = enabled;
	if (cullFaceEnabled)
		glEnable(GL_CULL_FACE);
	else
		glDisable(GL_CULL_FACE);
}

void CRenderer::SetBlending(const bool enabled)
{
	assert(blendingEnabled != enabled);
	blendingEnabled = enabled;
	if (blendingEnabled)
		glEnable(GL_BLEND);
	else
		glDisable(GL_BLEND);
}

void CRenderer::SetBlendFunc(const GLenum sfactor, const GLenum dfactor)
{
	assert(blendFunc[0] != sfactor || blendFunc[1] != dfactor);
	blendFunc[0] = sfactor;
	blendFunc[1] = dfactor;
	glBlendFunc(blendFunc[0], blendFunc[1]);
}

void CRenderer::SetWireframe(const bool enabled)
{
	assert(wireframeEnabled != enabled);
	wireframeEnabled = enabled;
	if (wireframeEnabled)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	else
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void CRenderer::DrawTexturedQuad(const float posX, const float posY, const float scaleW, const float scaleH)
{
	glBegin(GL_QUADS);
	glTexCoord2f(0.0f, 1.0f); glVertex2f(posX + 0.0f, posY + 0.0f);
	glTexCoord2f(0.0f, 0.0f); glVertex2f(posX + 0.0f, posY + 1.0f * scaleW);
	glTexCoord2f(1.0f, 0.0f); glVertex2f(posX + 1.0f * scaleW, posY + 1.0f * scaleW);
	glTexCoord2f(1.0f, 1.0f); glVertex2f(posX + 1.0f * scaleW, posY + 0.0f);
	glEnd();
}

void CRenderer::DrawTexturedRect(const float left, const float top, const float right, const float bottom)
{
	glBegin(GL_QUADS);
	glTexCoord2f(0.0f, 0.0f); glVertex2f(left, top);
	glTexCoord2f(0.0f, 1.0f); glVertex2f(left, bottom);
	glTexCoord2f(1.0f, 1.0f); glVertex2f(right, bottom);
	glTexCoord2f(1.0f, 0.0f); glVertex2f(right, top);
	glEnd();
}

void CRenderer::DrawVBO(CVBO* vbo, const GLenum mode)
{
	vbo->drawElements(mode);
}

void CRenderer::Flip()
{
	glFinish();
	glutSwapBuffers();
}

void CRenderer::EnableTexture(const int index, CTexture* texture)
{
	assert(texture != NULL);
	assert(!textureStates[index].active);
	assert(textureStates[index].texture == NULL);
	glActiveTexture(GL_TEXTURE0 + index);
	glEnable(texture->getTarget());
	texture->bind();
	textureStates[index].active = true;
	textureStates[index].texture = texture;
}

void CRenderer::DisableTexture(const int index, CTexture* texture)
{
	assert(textureStates[index].active);
	assert(textureStates[index].texture == texture);
	glActiveTexture(GL_TEXTURE0 + index);
	texture->unbind();
	glDisable(texture->getTarget());
	textureStates[index].active = false;
	textureStates[index].texture = NULL;
}
