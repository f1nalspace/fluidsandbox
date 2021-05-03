/*
======================================================================================================================
	Fluid Sandbox - Renderer.h

	Copyright (C) Torsten Spaete 2011-2021. All rights reserved.
	MPL v2 licensed. See LICENSE.txt for more details.
======================================================================================================================
*/

#pragma once

#include <assert.h>

#include <cstdint>

#include <glad/glad.h>

#include <glm/glm.hpp>

#include "VBO.h"
#include "Texture.h"
#include "TextureFont.h"
#include "GeometryVBO.h"

struct FontVertex {
	glm::vec4 color;
	glm::vec2 pos;
	glm::vec2 uv;
};
constexpr static uint32_t FontVertexStride = sizeof(FontVertex);

#define GL_CHECKERROR() assert(glGetError() == GL_NO_ERROR)

static const char *glErrorToString(GLuint code) {
	switch(code) {
		case GL_INVALID_ENUM: return "GL_INVALID_ENUM";

		case GL_INVALID_VALUE: return "GL_INVALID_VALUE";

		case GL_INVALID_OPERATION: return "GL_INVALID_OPERATION";

		case GL_STACK_OVERFLOW: return "GL_STACK_OVERFLOW";

		case GL_STACK_UNDERFLOW: return "GL_STACK_UNDERFLOW";

		case GL_OUT_OF_MEMORY: return "GL_OUT_OF_MEMORY";

		default: return "GL_NO_ERROR";
	}
}

enum class ClearFlags: int {
	None = 0,
	Color = 1 << 0,
	Depth = 1 << 1,
};

inline ClearFlags operator | (ClearFlags a, ClearFlags b) {

	return static_cast<ClearFlags>(static_cast<int>(a) | static_cast<int>(b));
}
inline ClearFlags &operator |= (ClearFlags &a, ClearFlags b) {

	return a = a | b;
}
inline ClearFlags operator & (ClearFlags a, ClearFlags b) {

	return static_cast<ClearFlags>(static_cast<int>(a) & static_cast<int>(b));
}
inline ClearFlags &operator &= (ClearFlags &a, ClearFlags b) {

	return a = a & b;
}
inline ClearFlags operator ~ (ClearFlags a) {

	return static_cast<ClearFlags>(~static_cast<int>(a));
}
inline ClearFlags operator ^ (ClearFlags a, ClearFlags b) {

	return static_cast<ClearFlags>(static_cast<int>(a) ^ static_cast<int>(b));
}
inline ClearFlags &operator ^= (ClearFlags &a, ClearFlags b) {

	return a = a ^ b;
}

const short MAX_TEXTURES = 16;
struct TextureState {
	bool active;
	CTexture *texture;
};

class CRenderer {
private:
	bool depthTestEnabled;
	bool depthMaskEnabled;
	bool cullFaceEnabled;
	bool blendingEnabled;
	bool wireframeEnabled;
	GLenum blendFunc[2];
	TextureState textureStates[MAX_TEXTURES];
public:
	CRenderer(void);
	~CRenderer(void);
	void Clear(const ClearFlags flags);
	void ClearColor(const float r, const float g, const float b, const float a);

	void SetViewport(const int left, const int top, const int width, const int height);
	void SetScissor(const int left, const int top, const int width, const int height);

	void SetDepthTest(const bool enabled);
	void SetDepthMask(const bool enabled);
	void SetCullFace(const bool enabled);
	void SetBlending(const bool enabled);
	void SetBlendFunc(const GLenum sfactor, const GLenum dfactor);
	void SetWireframe(const bool enabled);
	void EnableTexture(const int index, CTexture *texture);
	void DisableTexture(const int index, CTexture *texture);

	void DrawPrimitive(GeometryVBO *vbo, const bool asLines);
	void DrawVBO(CVBO *vbo, const GLenum mode, const GLuint count, const GLsizeiptr offset);
	glm::vec2 GetStringSize(const FontAtlas *atlas, const char *text, const size_t textLen, const float charHeight, int &glyphCount);
	void DrawString(const FontAtlas *atlas, const float posX, const float posY, const float charHeight, const char *text, const size_t textLen, const glm::vec4 &color, VBOWritter &writer);
	void DrawString(const FontAtlas *atlas, const float posX, const float posY, const float charHeight, const char *text, const glm::vec4 &color, VBOWritter &writer);

	void Flip();

	inline const char *CheckError() {
		GLenum err = glGetError();
		const char *result = glErrorToString(err);
		assert(err == GL_NO_ERROR);
		return(result);
	}
};

