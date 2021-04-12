#pragma once

#include <assert.h>
#include <malloc.h>
#include <stdint.h>

#include <GL/glew.h>

#include "Texture2D.h"

class CFBO
{
private:
	static constexpr uint32_t MaxTextureCount = 16;
	CTexture2D *textures[MaxTextureCount];
	GLuint bufferId;
	uint32_t textureCount;
	uint32_t maxColorAttachments;
	int width;
	int height;
protected:
	virtual void updateIndices() {}
public:
	CFBO(int width, int height);
	~CFBO(void);
	CTexture2D *addTextureTarget(GLint internalFormat, GLenum format, GLenum type, GLenum fbotype, GLuint texfilter);
	CTexture2D *addRenderTarget(GLint internalFormat, GLenum format, GLenum type, GLenum fbotype, GLuint texfilter);
	void enable();
	void disable();
	void resize(int width, int height);
	void update();
	int getWidth() { return width; }
	int getHeight() { return height; }
	GLint getDrawBuffer();
	void setDrawBuffer(GLenum buffer);
	void setDrawBuffers(GLenum* buffers, int count);
	static uint32_t getMaxColorAttachments();
};

