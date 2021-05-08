/*
======================================================================================================================
	Fluid Sandbox - Renderer.cpp

	Copyright (C) Torsten Spaete 2011-2021. All rights reserved.
	MPL v2 licensed. See LICENSE.txt for more details.
======================================================================================================================
*/

#include "Renderer.h"

#define _USE_MATH_DEFINES 1
#include <math.h>

#include <final_platform_layer.h>

#include "Primitives.h"

constexpr int SpacesForTabstop = 2;

CRenderer::CRenderer(void) {
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
	for(int i = 0; i < MAX_TEXTURES; i++) {
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

CRenderer::~CRenderer(void) {
}

void CRenderer::Clear(const ClearFlags flags) {
	GLbitfield f = 0;
	if((flags & ClearFlags::Color) == ClearFlags::Color)
		f |= GL_COLOR_BUFFER_BIT;
	if((flags & ClearFlags::Depth) == ClearFlags::Depth)
		f |= GL_DEPTH_BUFFER_BIT;
	glClear(f);
}

void CRenderer::ClearColor(const float r, const float g, const float b, const float a) {
	glClearColor(r, g, b, a);
}

void CRenderer::SetViewport(const int left, const int top, const int width, const int height) {
	glViewport(left, top, width, height);
}

void CRenderer::SetScissor(const int left, const int top, const int width, const int height) {
	glScissor(left, top, width, height);
}

void CRenderer::SetDepthTest(const bool enabled) {
	assert(depthTestEnabled != enabled);
	depthTestEnabled = enabled;
	if(depthTestEnabled)
		glEnable(GL_DEPTH_TEST);
	else
		glDisable(GL_DEPTH_TEST);
}

void CRenderer::SetDepthMask(const bool enabled) {
	assert(depthMaskEnabled != enabled);
	depthMaskEnabled = enabled;
	if(depthMaskEnabled)
		glDepthMask(GL_TRUE);
	else
		glDepthMask(GL_FALSE);
}

void CRenderer::SetCullFace(const bool enabled) {
	assert(cullFaceEnabled != enabled);
	cullFaceEnabled = enabled;
	if(cullFaceEnabled)
		glEnable(GL_CULL_FACE);
	else
		glDisable(GL_CULL_FACE);
}

void CRenderer::SetBlending(const bool enabled) {
	assert(blendingEnabled != enabled);
	blendingEnabled = enabled;
	if(blendingEnabled)
		glEnable(GL_BLEND);
	else
		glDisable(GL_BLEND);
}

void CRenderer::SetBlendFunc(const GLenum sfactor, const GLenum dfactor) {
	assert(blendFunc[0] != sfactor || blendFunc[1] != dfactor);
	blendFunc[0] = sfactor;
	blendFunc[1] = dfactor;
	glBlendFunc(blendFunc[0], blendFunc[1]);
}

void CRenderer::SetWireframe(const bool enabled) {
	assert(wireframeEnabled != enabled);
	wireframeEnabled = enabled;
	if(wireframeEnabled)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	else
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void CRenderer::DrawPrimitive(GeometryVBO *vbo, const bool asLines) {
	// NOTE(final): Expect that a shader is already bound

	// Vertex (vec3, vec3, vec2)
	vbo->Bind();

	if(!asLines || vbo->lineIndexCount == 0) {
		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_NORMAL_ARRAY);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glVertexPointer(3, GL_FLOAT, sizeof(Primitives::Vertex), (void *)(offsetof(Primitives::Vertex, pos)));
		glNormalPointer(GL_FLOAT, sizeof(Primitives::Vertex), (void *)(offsetof(Primitives::Vertex, normal)));
		glTexCoordPointer(2, GL_FLOAT, sizeof(Primitives::Vertex), (void *)(offsetof(Primitives::Vertex, texcoord)));
		DrawVBO(vbo, GL_TRIANGLES, vbo->triangleIndexCount, 0);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		glDisableClientState(GL_NORMAL_ARRAY);
		glDisableClientState(GL_VERTEX_ARRAY);
	} else {
		glEnableClientState(GL_VERTEX_ARRAY);
		glVertexPointer(3, GL_FLOAT, sizeof(Primitives::Vertex), (void *)(offsetof(Primitives::Vertex, pos)));
		DrawVBO(vbo, GL_LINES, vbo->lineIndexCount, sizeof(GLuint) * vbo->triangleIndexCount);
		glDisableClientState(GL_VERTEX_ARRAY);
	}

	vbo->Unbind();
}

void CRenderer::DrawVBO(CVBO *vbo, const GLenum mode, const GLuint count, const GLsizeiptr offset) {
	vbo->DrawElements(mode, count, offset);
}

glm::vec2 CRenderer::GetStringSize(const FontAtlas *atlas, const char *text, const size_t textLen, const float charHeight, int &glyphCount) {
	glyphCount = 0;
	glm::vec2 result = glm::vec2(0);
	if(text != nullptr && textLen > 0 && atlas != nullptr) {
		const FontInfo &info = atlas->info;
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
				const FontGlyph *glyph = atlas->glyphs + charIndex;
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
				++glyphCount;
			}

			++p;
		}
		result.x = totalWidth;
		result.y = charHeight;
	}
	return(result);
}

void CRenderer::DrawString(const FontAtlas *atlas, const float posX, const float posY, const float charHeight, const char *text, const size_t textLen, const glm::vec4 &color, VBOWritter &writer) {
	if(atlas == nullptr) return;

	const FontInfo &info = atlas->info;

	int glyphCount = 0;
	glm::vec2 size = GetStringSize(atlas, text, textLen, charHeight, glyphCount);
	glm::vec2 alignOffset = glm::vec2(0, size.y * 0.5f);

	uint32_t requiredVertexCount = glyphCount * 4;
	uint32_t requiredIndexCount = glyphCount * 6;

	assert((writer.vertexOffset + requiredVertexCount) <= writer.maxVertexCount);
	assert((writer.indexOffset + requiredIndexCount) <= writer.maxIndexCount);

	FontVertex *verts = (FontVertex *)writer.verts + writer.vertexOffset;
	GLuint *indices = writer.indices + writer.indexOffset;
	uint32_t vertexIndex = writer.vertexOffset;

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
			const FontGlyph *glyph = atlas->glyphs + charIndex;

			// Vertex format is: vec4 vec2 vec2

			// TR, TL, BL, BR ([0, W] [0, 0] [0, H] [W, H])
			verts[0].pos = pos + glyph->offset[0] * scale + alignOffset;
			verts[1].pos = pos + glyph->offset[1] * scale + alignOffset;
			verts[2].pos = pos + glyph->offset[2] * scale + alignOffset;
			verts[3].pos = pos + glyph->offset[3] * scale + alignOffset;

			verts[0].uv = glyph->uv[0];
			verts[1].uv = glyph->uv[1];
			verts[2].uv = glyph->uv[2];
			verts[3].uv = glyph->uv[3];

			verts[0].color = color;
			verts[1].color = color;
			verts[2].color = color;
			verts[3].color = color;

			indices[0] = vertexIndex + 0;
			indices[1] = vertexIndex + 1;
			indices[2] = vertexIndex + 2;
			indices[3] = vertexIndex + 2;
			indices[4] = vertexIndex + 3;
			indices[5] = vertexIndex + 0;

			verts += 4;
			vertexIndex += 4;
			indices += 6;

			writer.vertexOffset += 4;
			writer.indexOffset += 6;

			pos += glm::vec2(glyph->advance * scale, 0);
		}
		++p;
	}
}

void CRenderer::DrawString(const FontAtlas *atlas, const float posX, const float posY, const float charHeight, const char *text, const glm::vec4 &color, VBOWritter &writer) {
	size_t textLen = strlen(text);
	DrawString(atlas, posX, posY, charHeight, text, textLen, color, writer);
}

void CRenderer::Flip() {
	glFinish();
	fplVideoFlip();
}

void CRenderer::EnableTexture(const int index, CTexture *texture) {
	assert(texture != nullptr);
	assert(!textureStates[index].active);
	assert(textureStates[index].texture == nullptr);
	glActiveTexture(GL_TEXTURE0 + index);
	glEnable(texture->getTarget());
	texture->bind();
	textureStates[index].active = true;
	textureStates[index].texture = texture;
}

void CRenderer::DisableTexture(const int index, CTexture *texture) {
	assert(textureStates[index].active);
	assert(textureStates[index].texture == texture);
	glActiveTexture(GL_TEXTURE0 + index);
	texture->unbind();
	glDisable(texture->getTarget());
	textureStates[index].active = false;
	textureStates[index].texture = nullptr;
}
