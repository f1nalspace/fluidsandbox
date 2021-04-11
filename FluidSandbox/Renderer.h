#pragma once

#include <assert.h>

#include <GL/glew.h>
#include <GL/freeglut.h>
#include <glm/glm.hpp>
#include "VBO.h"
#include "Texture.h"

#define GL_CHECKERROR() assert(glGetError() == GL_NO_ERROR)

static const char *glErrorToString(GLuint code) {
	switch(code) {
		case GL_INVALID_ENUM: return "GL_INVALID_ENUM";

		case GL_INVALID_VALUE: return "GL_INVALID_VALUE";

		case GL_INVALID_OPERATION: return "GL_INVALID_OPERATION";

		case GL_STACK_OVERFLOW: return "GL_STACK_OVERFLOW";

		case GL_STACK_UNDERFLOW: return "GL_STACK_UNDERFLOW";

		case GL_OUT_OF_MEMORY: return "GL_OUT_OF_MEMORY";

		case GL_TABLE_TOO_LARGE: return "GL_TABLE_TOO_LARGE";

		default: return "GL_NO_ERROR";
	}
}

namespace ClearFlags
{
	enum {
		Color = 1,
		Depth = 2
	};
};

const short MAX_TEXTURES = 16;
struct TextureState {
	bool active;
	CTexture* texture;
};

class CRenderer
{
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
	void Clear(unsigned int flags);
	void ClearColor(float r, float g, float b, float a);
	
	void SetViewport(int left, int top, int width, int height);
	void SetScissor(int left, int top, int width, int height);
	void LoadMatrix(const glm::mat4 &m);

	void SetColor(float r, float g, float b, float a);
	void SetColor(float* color);
	void SetDepthTest(bool enabled);
	void SetDepthMask(bool enabled);
	void SetCullFace(bool enabled);
	void SetBlending(bool enabled);
	void SetBlendFunc(GLenum sfactor, GLenum dfactor);
	void SetWireframe(bool enabled);
	void EnableTexture(int index, CTexture* texture);
	void DisableTexture(int index, CTexture* texture);

	void DrawTexturedQuad(float posX, float posY, float scaleW, float scaleH);
	void DrawTexturedRect(float left, float top, float right, float bottom);
	void DrawVBO(CVBO* vbo, const GLenum mode);

	void Flip();

	inline const char *CheckError() {
		GLenum err = glGetError();
		const char *result = glErrorToString(err);
		assert(err == GL_NO_ERROR);
		return(result);
	}
};

