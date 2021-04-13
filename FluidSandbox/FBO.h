#pragma once

#include <assert.h>
#include <malloc.h>
#include <cstdint>

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
	CFBO(const int width, const int height);
	~CFBO(void);
	CTexture2D *addTextureTarget(const GLint internalFormat, const GLenum format, const GLenum type, const GLenum fbotype, const GLuint texfilter);
	CTexture2D *addRenderTarget(const GLint internalFormat, const GLenum format, const GLenum type, const GLenum fbotype, const GLuint texfilter);
	void enable();
	void disable();
	void resize(const int width, const int height);
	void update();
	int getWidth() { return width; }
	int getHeight() { return height; }
	GLint getDrawBuffer();
	void setDrawBuffer(const GLenum buffer);
	void setDrawBuffers(const GLenum* buffers, const int count);
	static uint32_t getMaxColorAttachments();
};

