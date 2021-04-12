#pragma once

#include <map>
#include <assert.h>
#include <malloc.h>

#include <GL/glew.h>

#include "Texture2D.h"

typedef uint32_t FBOTextureID;

class CFBO
{
private:
	static constexpr uint32_t MaxTextureCount = 8;
	std::map<FBOTextureID, CTexture2D *> textureMap;
	CTexture2D *textures[MaxTextureCount];
	GLuint bufferId;
	uint32_t textureCount;
	uint32_t maxColorAttachments;
	int width;
	int height;
public:
	CFBO(int width, int height);
	~CFBO(void);
	uint32_t addTextureTarget(GLint internalFormat, GLenum format, GLenum type, GLenum fbotype, const FBOTextureID id, GLuint texfilter);
	uint32_t addRenderTarget(GLint internalFormat, GLenum format, GLenum type, GLenum fbotype, const FBOTextureID id, GLuint texfilter);
	CTexture2D* getTexture(const FBOTextureID id);
	void enable();
	void disable();
	void resize(int width, int height);
	void update();
	void changeDepthTexture(const FBOTextureID id);
	void removeDepthTexture();
	int getWidth() { return width; }
	int getHeight() { return height; }
	GLint getDrawBuffer();
	void setDrawBuffer(GLenum buffer);
	void setDrawBuffers(GLenum* buffers, int count);
	static uint32_t getMaxColorAttachments();
};

