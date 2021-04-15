#include "Renderer.h"

constexpr int SpacesForTabstop = 2;

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
		textureStates[i].texture = nullptr;
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

glm::vec2 CRenderer::GetTextSize(const CTextureFont *fontTex, const char *text, const size_t textLen, const float charHeight) {
	glm::vec2 result = glm::vec2(0);
	if(text != nullptr && textLen > 0 && fontTex != nullptr) {
		const FontAtlas &atlas = fontTex->GetAtlas();
		const FontInfo &info = atlas.info;
		float totalWidth = 0.0f;
		const char *p = text;
		while(*p) {
			size_t n = (p - text) + 1;
			if(n > textLen) break;

			// Tab stop
			if(*p == '\t') {
				totalWidth += info.spaceAdvance * charHeight * SpacesForTabstop;
				++p;
				continue;
			}

			uint32_t codePoint = (unsigned char)*p;
			if(codePoint >= info.minChar && codePoint <= info.maxChar) {
				uint32_t charIndex = codePoint - info.minChar;
				const FontGlyph *glyph = atlas.glyphs + charIndex;
				glm::vec2 verts[] = {
					glyph->offset[0] * charHeight,
					glyph->offset[1] * charHeight,
					glyph->offset[2] * charHeight,
					glyph->offset[3] * charHeight,
				};
				glm::vec2 min = glyph->offset[0];
				glm::vec2 max = glyph->offset[0];
				for(int i = 1; i < 4; ++i) {
					min = glm::min(min, verts[i]);
					max = glm::max(max, verts[i]);
				}
				totalWidth += glyph->advance * charHeight;
			}

			++p;
		}
		result.x = totalWidth;
		result.y = charHeight;
	}
	return(result);
}

void CRenderer::DrawText(const int texIndex, CTextureFont *fontTex, const float posX, const float posY, const char *text, const size_t textLen, const float charHeight) {
	if(fontTex == nullptr) return;

	const FontAtlas &atlas = fontTex->GetAtlas();
	const FontInfo &info = atlas.info;

	EnableTexture(texIndex, fontTex);

	glColor4f(1, 1, 1, 1);
	glBegin(GL_QUADS);
	const char *p = text;
	glm::vec2 d = glm::vec2(0, 0);
	glm::vec2 pos = glm::vec2(posX, posY) + d;
	float scale = charHeight;
	while(*p) {

		// Tab stop
		if(*p == '\t') {
			pos += glm::vec2(info.spaceAdvance * scale * SpacesForTabstop, 0);
			++p;
			continue;
		}

		uint32_t codePoint = (unsigned char)*p;
		if(codePoint >= info.minChar && codePoint <= info.maxChar) {
			uint32_t charIndex = codePoint - info.minChar;
			const FontGlyph *glyph = atlas.glyphs + charIndex;

			glm::vec2 v0 = pos + glyph->offset[0] * scale;
			glm::vec2 v1 = pos + glyph->offset[1] * scale;
			glm::vec2 v2 = pos + glyph->offset[2] * scale;
			glm::vec2 v3 = pos + glyph->offset[3] * scale;

			glTexCoord2fv(&glyph->uv[0][0]); glVertex2fv(&v0[0]);
			glTexCoord2fv(&glyph->uv[1][0]); glVertex2fv(&v1[0]);
			glTexCoord2fv(&glyph->uv[2][0]); glVertex2fv(&v2[0]);
			glTexCoord2fv(&glyph->uv[3][0]); glVertex2fv(&v3[0]);

			pos += glm::vec2(glyph->advance * scale, 0);
		}
		++p;
	}
	glEnd();
	glColor4f(1, 1, 1, 1);

	DisableTexture(texIndex, fontTex);
}

void CRenderer::Flip()
{
	glFinish();
	glutSwapBuffers();
}

void CRenderer::EnableTexture(const int index, CTexture* texture)
{
	assert(texture != nullptr);
	assert(!textureStates[index].active);
	assert(textureStates[index].texture == nullptr);
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
	textureStates[index].texture = nullptr;
}
